#!/usr/bin/env python3
"""Differential test: recompiled functions vs the original x86 (Unicorn).

For each candidate function (no imports or indirect calls reachable through
its direct-call closure), run random trials on both sides from an identical
state and compare EAX..EDI, ESP, the x87 stack top/ST0 and every byte of the
scratch, stack and .data regions. Unicorn runs with FPCW=0x27F, the Windows
default the CRT establishes (53-bit precision).

  python3 tools/recomp/difftest.py MMU.exe build-macos/recomp_test/libmmurecomp.dylib \
      [--funcs 0x5a7490,...] [--limit 400] [--trials 12] [--seed 1]
"""

import argparse
import ctypes
import random
import struct
import sys
import os

from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import *

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lift import PE, Lifter, Image, DLLS, plausible_code  # noqa: E402
import lift as liftmod  # noqa: E402
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG, X86_REG_INVALID  # noqa: E402

BINDINGS = {}
SCRATCH = 0x02000000
SCRATCH_SIZE = 0x10000
STACK_LO = 0x00f00000
STACK_TOP = 0x00fffff0
SENTINEL = 0x3fff0000
IMG_LO, IMG_HI = 0x400000, 0x6d0000


class CpuStruct(ctypes.Structure):
    _fields_ = [(n, ctypes.c_uint32) for n in ('eax', 'ecx', 'edx', 'ebx', 'esp', 'ebp', 'esi', 'edi')] + [
        ('st', ctypes.c_double * 8), ('top', ctypes.c_uint32), ('fcw', ctypes.c_uint16),
        ('c0', ctypes.c_uint8), ('c1', ctypes.c_uint8), ('c2', ctypes.c_uint8), ('c3', ctypes.c_uint8),
        ('xmm', (ctypes.c_double * 2) * 8), ('eflags_extra', ctypes.c_uint32)]


def f80_to_double(raw):
    mant, se = raw if isinstance(raw, tuple) else (raw & ((1 << 64) - 1), raw >> 64)
    exp = se & 0x7fff
    sign = -1.0 if se & 0x8000 else 1.0
    if exp == 0 and mant == 0:
        return sign * 0.0
    if exp == 0x7fff:
        return float('nan') if (mant << 1) & ((1 << 64) - 1) else sign * float('inf')
    import math
    return sign * math.ldexp(mant, exp - 16383 - 63)


def pure_candidates(pe, L, limit):
    """Functions whose reachable code never calls an import or goes indirect."""
    verdict = {}

    def ok(entry, depth=0):
        if entry in verdict:
            return verdict[entry]
        if depth > 12 or entry in L.import_thunks:
            return False
        verdict[entry] = False  # recursion guard
        try:
            insns, tables = L.collect(entry)
        except Exception:
            return False
        good = True
        for i in insns.values():
            m = i.mnemonic
            if m in ('call', 'jmp'):
                op = i.operands[0]
                if op.type == X86_OP_MEM and op.mem.base == X86_REG_INVALID and \
                        op.mem.index == X86_REG_INVALID and (op.mem.disp & 0xffffffff) in BINDINGS:
                    if not ok(BINDINGS[op.mem.disp & 0xffffffff], depth + 1):
                        good = False
                        break
                    continue
                if op.type != X86_OP_IMM:
                    if not (m == 'jmp' and i.address in tables):
                        good = False
                        break
                    continue
                t = op.imm & 0xffffffff
                if m == 'call' or t in L.entries and t != entry:
                    if not ok(t, depth + 1):
                        good = False
                        break
            if m in ('cpuid', 'int3', 'hlt', 'pushfd', 'popfd') or 'fs:' in i.op_str or \
                    m.startswith('rep'):
                good = False
                break
        verdict[entry] = good
        return good

    out = []
    for e in sorted(L.entries):
        if ok(e):
            out.append(e)
            if len(out) >= limit:
                break
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('exe')
    ap.add_argument('lib')
    ap.add_argument('--funcs', default='')
    ap.add_argument('--limit', type=int, default=400)
    ap.add_argument('--trials', type=int, default=12)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--module', default='', help='only test functions of this DLL (e.g. libogg-0.dll)')
    args = ap.parse_args()
    rnd = random.Random(args.seed)

    gamedir = os.path.dirname(os.path.abspath(args.exe))
    exe = PE(args.exe)
    dlls = [PE(os.path.join(gamedir, n), rebase=b) for n, b in DLLS]
    img = Image([exe] + dlls)
    pe = img
    L = Lifter(img)
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    L.entries = {int(l.split()[0], 16) for l in open(os.path.join(root, 'docs', 'function_inventory.txt'))
                 if l.startswith('0x')}
    for m in dlls:
        for kind, va in m.exports.values():
            if kind == 'va' and m.in_text(va) and plausible_code(L, va):
                L.entries.add(va)
    lib0 = ctypes.CDLL(args.lib)
    # IAT slots that resolve to shims (OS layer) are off limits for pure tests.
    shim_slots = set()
    bindings = {}
    rt_b = (ctypes.c_uint32 * 2 * 4096)
    nb = ctypes.c_uint.in_dll(lib0, 'rt_binding_count').value
    nb_arr = (ctypes.c_uint32 * (2 * nb)).in_dll(lib0, 'rt_bindings')
    for k in range(nb):
        bindings[nb_arr[2 * k]] = nb_arr[2 * k + 1]
    shim_slots = {va for va in img.iat if va not in bindings}
    global BINDINGS
    BINDINGS = bindings
    L.import_thunks = {}
    for s in list(L.entries):
        i = L.insn(s)
        if i is not None and i.mnemonic == 'jmp' and i.operands[0].type == X86_OP_MEM and \
                i.operands[0].mem.base == X86_REG_INVALID and (i.operands[0].mem.disp & 0xffffffff) in pe.iat:
            L.import_thunks[s] = 'imp'
    L.noreturn = set()
    L.new_entries = set()
    L.entries_data_guard = set()

    lib = lib0
    lib.rt_init.argtypes = [ctypes.c_char_p]
    lib.rt_lookup.restype = ctypes.c_void_p
    lib.rt_lookup.argtypes = [ctypes.c_uint32]
    assert lib.rt_init(gamedir.encode())
    mem_base = ctypes.c_void_p.in_dll(lib, 'rt_mem').value
    cpu = CpuStruct.in_dll(lib, 'rt_cpu')
    FN = ctypes.CFUNCTYPE(None, ctypes.POINTER(CpuStruct))

    image = ctypes.string_at(mem_base + IMG_LO, IMG_HI - IMG_LO)
    # Snapshot every loaded module (already relocated + IAT-bound by rt_init).
    mods = []
    for m in dlls:
        lo = m.base
        hi = max(s_[1] + max(s_[2], s_[4]) for s_ in m.sections)
        hi = (hi + 0xfff) & ~0xfff
        mods.append((lo, hi, ctypes.string_at(mem_base + lo, hi - lo)))

    if args.module:
        mm_ = [m for m in dlls if os.path.basename(m.path).lower() == args.module.lower()][0]
        L.entries = {e for e in L.entries if mm_.in_text(e)} | {v for k_, v in mm_.exports.values() if k_ == 'va' and mm_.in_text(v)}
    funcs = [int(x, 16) for x in args.funcs.split(',') if x] or pure_candidates(pe, L, args.limit)
    print('testing %d functions x %d trials' % (len(funcs), args.trials))

    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(0x00400000, IMG_HI - IMG_LO)
    uc.mem_map(STACK_LO, 0x00100000)
    uc.mem_map(SCRATCH, SCRATCH_SIZE)
    uc.mem_map(SENTINEL, 0x1000)
    for lo, hi, data in mods:
        uc.mem_map(lo, hi - lo)
        uc.mem_write(lo, data)
    heap_lo = 0x01000000
    uc.mem_map(heap_lo, 0x01000000)  # guest heap region (rt_init allocations)
    uc.mem_write(heap_lo, ctypes.string_at(mem_base + heap_lo, 0x01000000))

    fails = passed = skipped = 0
    for fva in funcs:
        fn_ptr = lib.rt_lookup(fva)
        if not fn_ptr:
            print('  0x%x: not in recompiled table' % fva)
            continue
        fn = FN(fn_ptr)
        bad = None
        for t in range(args.trials):
            scratch = bytes(rnd.getrandbits(8) for _ in range(SCRATCH_SIZE))
            # Plant some well-formed floats/pointers in the scratch area.
            sb = bytearray(scratch)
            for k in range(0, SCRATCH_SIZE, 16):
                struct.pack_into('<f', sb, k, rnd.uniform(-500, 500))
                struct.pack_into('<I', sb, k + 4, SCRATCH + rnd.randrange(0, SCRATCH_SIZE - 64) & ~3)
            scratch = bytes(sb)
            def pick():
                r = rnd.random()
                if r < 0.4:
                    return SCRATCH + (rnd.randrange(0, SCRATCH_SIZE - 256) & ~3)
                if r < 0.7:
                    return rnd.randrange(0, 64)
                if r < 0.85:
                    return struct.unpack('<I', struct.pack('<f', rnd.uniform(-300, 300)))[0]
                return rnd.getrandbits(32)
            regs = [pick() for _ in range(8)]
            stack_args = b''.join(struct.pack('<I', pick()) for _ in range(16))
            esp = STACK_TOP - 0x100
            stack = bytearray(0x200)
            stack[0x100:0x104] = struct.pack('<I', SENTINEL)
            stack[0x104:0x144] = stack_args
            ebp = esp + 0x80

            # --- unicorn
            uc.mem_write(IMG_LO, image)
            uc.mem_write(SCRATCH, scratch)
            uc.mem_write(STACK_TOP - 0x200, bytes(stack))
            for r, v in zip((UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_EBX,
                             UC_X86_REG_ESI, UC_X86_REG_EDI), (regs[0], regs[1], regs[2], regs[3], regs[6], regs[7])):
                uc.reg_write(r, v)
            uc.reg_write(UC_X86_REG_ESP, esp)
            uc.reg_write(UC_X86_REG_EBP, ebp)
            uc.reg_write(UC_X86_REG_FPCW, 0x27f)
            uc.reg_write(UC_X86_REG_FPSW, 0)
            uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
            try:
                uc.emu_start(fva, SENTINEL, timeout=200000, count=200000)
            except UcError:
                skipped += 1
                continue
            if uc.reg_read(UC_X86_REG_EIP) != SENTINEL:
                skipped += 1
                continue
            ref = {n: uc.reg_read(r) for n, r in (('eax', UC_X86_REG_EAX), ('ecx', UC_X86_REG_ECX),
                                                    ('edx', UC_X86_REG_EDX), ('ebx', UC_X86_REG_EBX),
                                                    ('esp', UC_X86_REG_ESP), ('ebp', UC_X86_REG_EBP),
                                                    ('esi', UC_X86_REG_ESI), ('edi', UC_X86_REG_EDI))}
            fsw = uc.reg_read(UC_X86_REG_FPSW)
            ref_top = (fsw >> 11) & 7
            ref_st0 = f80_to_double(uc.reg_read(UC_X86_REG_FP0 + (ref_top & 7))) if ref_top != 0 else None
            ref_scratch = bytes(uc.mem_read(SCRATCH, SCRATCH_SIZE))
            ref_stack = bytes(uc.mem_read(STACK_TOP - 0x200, 0x200))
            ref_data = bytes(uc.mem_read(0x6c1000, 0x7000))

            # --- recompiled
            ctypes.memmove(mem_base + IMG_LO, image, len(image))
            ctypes.memmove(mem_base + SCRATCH, scratch, SCRATCH_SIZE)
            ctypes.memmove(mem_base + STACK_TOP - 0x200, bytes(stack), 0x200)
            cpu.eax, cpu.ecx, cpu.edx, cpu.ebx = regs[0], regs[1], regs[2], regs[3]
            cpu.esi, cpu.edi, cpu.esp, cpu.ebp = regs[6], regs[7], esp, ebp
            cpu.top, cpu.fcw = 0, 0x27f
            cpu.c0 = cpu.c1 = cpu.c2 = cpu.c3 = 0
            fn(ctypes.byref(cpu))
            got = {n: getattr(cpu, n) for n in ref}
            diffs = [n for n in ref if ref[n] != got[n]]
            if ref_top != (cpu.top & 7):
                diffs.append('fpu top %d vs %d' % (ref_top, cpu.top & 7))
            elif ref_st0 is not None:
                # unicorn FP0 is ST0 (relative), recompiled ST0 = st[top]
                g = cpu.st[cpu.top & 7]
                if not (g == ref_st0 or (g != g and ref_st0 != ref_st0)):
                    diffs.append('st0 %r vs %r' % (ref_st0, g))
            if ctypes.string_at(mem_base + SCRATCH, SCRATCH_SIZE) != ref_scratch:
                diffs.append('scratch memory')
            if ctypes.string_at(mem_base + STACK_TOP - 0x200, 0x200) != ref_stack:
                diffs.append('stack memory')
            if ctypes.string_at(mem_base + 0x6c1000, 0x7000) != ref_data:
                diffs.append('.data')
            if diffs:
                bad = (t, diffs, {n: (hex(ref[n]), hex(got[n])) for n in ref if ref[n] != got[n]})
                break
            passed += 1
        if bad:
            fails += 1
            print('  FAIL 0x%x trial %d: %s %s' % (fva, bad[0], bad[1], bad[2]))
    print('trials passed=%d skipped(original faulted)=%d failing functions=%d' % (passed, skipped, fails))
    return 1 if fails else 0


if __name__ == '__main__':
    sys.exit(main())
