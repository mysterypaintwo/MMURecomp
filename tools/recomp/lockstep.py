#!/usr/bin/env python3
"""Lockstep divergence finder: original x86 (Unicorn) vs recompiled code.

1. Run the recompiled game with RECOMP_LOCKSTEP=<dir>: the debug wrapper of an
   interposed export (imports_debug.cpp) writes <dir>/snapshot.bin (guest
   memory, CPU, heap and TLS state right before the call) and
   <dir>/lifted_trace.bin (every recompiled function entry during the call).
2. python3 tools/recomp/lockstep.py <dir> <build>/recomp/gen
   replays the same call on the original machine code under Unicorn from the
   snapshot, records the function entries it reaches, and prints the first
   divergence with context.

Host imports reached by the original code are emulated here in Python (heap,
TLS, critical sections, ...). An unknown one stops the replay with its name.
"""

import os
import re
import struct
import sys

from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_BLOCK, UC_HOOK_CODE, UC_PROT_ALL, UC_HOOK_MEM_INVALID
from unicorn.x86_const import *

SENTINEL = 0x58ef0000
IMPORT_BASE = 0x58f00000


def load_modules(gen, gamedir):
    sys.path.insert(0, __import__('os').path.dirname(__import__('os').path.abspath(__file__)))
    from lift import PE
    text = open(gen + '/recomp_table.c').read()
    mods = {}
    for name, orig, base in re.findall(r'\{"([^"]+)", 0x([0-9a-f]+)u, 0x([0-9a-f]+)u', text):
        if name.lower().endswith('.dll'):
            pe = PE(gamedir + '/' + name, rebase=int(base, 16))
            mods[name.lower()] = (int(base, 16), pe.exports)
    return mods


def load_tables(gen):
    text = open(gen + '/recomp_table.c').read()
    funcs = set(int(m, 16) for m in re.findall(r'\{0x([0-9a-f]+)u, f_', text))
    imports = re.findall(r'\{0x[0-9a-f]+u, "([^"]*)", "([^"]*)", imp_', text)
    return funcs, imports


def main():
    d, gen = sys.argv[1], sys.argv[2]
    gamedir = sys.argv[3] if len(sys.argv) > 3 else os.path.expanduser('~/Downloads/MegaManUnlimited')
    funcs, imports = load_tables(gen)
    mods = load_modules(gen, gamedir)
    f = open(d + '/snapshot.bin', 'rb')
    assert f.read(8) == b'MMUSNAP1'
    cpu = f.read(320)  # sizeof(Cpu) on the host build
    regs = struct.unpack_from('<8I', cpu, 0)
    fcw = struct.unpack_from('<H', cpu, 100)[0]
    heap = struct.unpack('<15I', f.read(60))
    tls = list(struct.unpack('<129I', f.read(4 * 129)))
    last_error = tls.pop()
    target = struct.unpack('<I', f.read(4))[0]
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    mapped = []
    while True:
        lo, hi = struct.unpack('<II', f.read(8))
        if lo == hi == 0:
            break
        data = f.read(hi - lo)
        uc.mem_map(lo, hi - lo, UC_PROT_ALL)
        uc.mem_write(lo, data)
        mapped.append((lo, hi))
    bump = heap[0]
    heap_top = (bump + 0xfff) & ~0xfff
    uc.mem_map(heap_top, 0x08000000)  # room for new allocations during the call
    uc.mem_map(SENTINEL, 0x1000)

    # The recompiled build puts the TIB at FS base 0x1000.
    # FS -> TIB at 0x1000 through a minimal GDT (32-bit mode has no FS_BASE).
    gdt = 0x58ee0000
    uc.mem_map(gdt, 0x1000)
    def desc(base, limit, access, flags=0xc):
        return struct.pack('<Q', (limit & 0xffff) | ((base & 0xffffff) << 16) | (access << 40) |
                           (((limit >> 16) & 0xf) << 48) | (flags << 52) | (((base >> 24) & 0xff) << 56))
    uc.mem_write(gdt, b'\0' * 8 + desc(0, 0xfffff, 0x93) + desc(0x1000, 0xfff, 0x93) + desc(0, 0xfffff, 0x9b))
    uc.reg_write(UC_X86_REG_GDTR, (0, gdt, 4 * 8 - 1, 0))
    uc.reg_write(UC_X86_REG_FS, 2 << 3)
    for r in (UC_X86_REG_SS, UC_X86_REG_DS, UC_X86_REG_ES):
        uc.reg_write(r, 1 << 3)

    eax, ecx, edx, ebx, esp, ebp, esi, edi = regs
    rw = struct.unpack('<I', uc.mem_read(esp + 4, 4))[0]  # Mix_LoadMUS_RW(rw)
    esp -= 4
    uc.mem_write(esp, struct.pack('<I', rw))
    esp -= 4
    uc.mem_write(esp, struct.pack('<I', SENTINEL))
    for r, v in zip((UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_EBX, UC_X86_REG_EBP,
                     UC_X86_REG_ESI, UC_X86_REG_EDI), (eax, ecx, edx, ebx, ebp, esi, edi)):
        uc.reg_write(r, v)
    uc.reg_write(UC_X86_REG_ESP, esp)
    uc.reg_write(UC_X86_REG_FPCW, fcw)

    state = {'bump': heap_top, 'tls': tls, 'err': last_error, 'sizes': {}}

    def arg(n):
        return struct.unpack('<I', uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * n, 4))[0]

    def ret(value, nargs, stdcall=True):
        sp = uc.reg_read(UC_X86_REG_ESP)
        ra = struct.unpack('<I', uc.mem_read(sp, 4))[0]
        uc.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
        uc.reg_write(UC_X86_REG_ESP, sp + 4 + (4 * nargs if stdcall else 0))
        uc.reg_write(UC_X86_REG_EIP, ra)
        return True

    def alloc(n):
        p = (state['bump'] + 16 + 15) & ~15
        state['bump'] = p + n
        uc.mem_write(p, b'\0' * n)
        state['sizes'][p] = n
        return p

    def shim(name):
        if name == 'HeapAlloc':
            return ret(alloc(arg(2)), 3)
        if name == 'HeapFree':
            return ret(1, 3)
        if name == 'HeapReAlloc':
            old, n = arg(2), arg(3)
            p = alloc(n)
            on = state['sizes'].get(old, n)
            uc.mem_write(p, bytes(uc.mem_read(old, min(on, n))))
            return ret(p, 4)
        if name == 'HeapSize':
            return ret(state['sizes'].get(arg(2), 0), 3)
        if name == 'TlsGetValue':
            return ret(state['tls'][arg(0)] if arg(0) < 128 else 0, 1)
        if name == 'TlsSetValue':
            state['tls'][arg(0)] = arg(1)
            return ret(1, 2)
        if name == 'GetLastError':
            return ret(state['err'], 0)
        if name == 'SetLastError':
            state['err'] = arg(0)
            return ret(0, 1)
        if name in ('EnterCriticalSection', 'LeaveCriticalSection'):
            return ret(0, 1)
        if name in ('InterlockedIncrement', 'InterlockedDecrement'):
            v = (struct.unpack('<I', uc.mem_read(arg(0), 4))[0] + (1 if name.endswith('ment') and 'Inc' in name else -1)) & 0xffffffff
            uc.mem_write(arg(0), struct.pack('<I', v))
            return ret(v, 1)
        if name == 'LoadLibraryA':
            n = bytes(uc.mem_read(arg(0), 260)).split(b'\0')[0].decode().lower().replace('/', '\\').split('\\')[-1]
            return ret(mods.get(n, (0, None))[0], 1)
        if name == 'GetProcAddress':
            base, want = arg(0), bytes(uc.mem_read(arg(1), 260)).split(b'\0')[0].decode()
            for b, exports in mods.values():
                if b == base:
                    e = exports.get(want)
                    return ret(e[1] if e and e[0] == 'va' else 0, 2)
            return ret(0, 2)
        if name == 'FreeLibrary':
            return ret(1, 1)
        if name == 'GetCurrentThreadId':
            return ret(0x1004, 0)
        if name in ('SDL_mutexP', 'SDL_mutexV', 'SDL_LockAudio', 'SDL_UnlockAudio'):
            return ret(0, 0, stdcall=False)
        return None

    trace = []

    def on_block(uc_, address, size, user):
        if address in funcs:
            trace.append(address)

    def on_code(uc_, address, size, user):
        if IMPORT_BASE <= address < IMPORT_BASE + 16 * len(imports):
            dll, name = imports[(address - IMPORT_BASE) // 16]
            if shim(name) is None:
                print('stopped: original code called unemulated import %s!%s' % (dll, name))
                uc_.emu_stop()

    uc.mem_map(IMPORT_BASE + 0x10000, 0x1000) if False else None
    def on_bad(uc_, access, address, size, value, user):
        print('invalid access type=%d addr=%#x size=%d eip=%#x esp=%#x' % (
            access, address, size, uc_.reg_read(UC_X86_REG_EIP), uc_.reg_read(UC_X86_REG_ESP)))
        return False
    uc.hook_add(UC_HOOK_MEM_INVALID, on_bad)
    uc.hook_add(UC_HOOK_BLOCK, on_block)
    uc.hook_add(UC_HOOK_CODE, on_code, begin=IMPORT_BASE, end=IMPORT_BASE + 0x10000)
    try:
        uc.emu_start(target, SENTINEL, count=50_000_000)
    except UcError as e:
        print('unicorn stopped:', e, 'at', hex(uc.reg_read(UC_X86_REG_EIP)))
    print('original result eax=%#x after %d function entries' % (uc.reg_read(UC_X86_REG_EAX), len(trace)))

    lifted = list(struct.unpack('<%dI' % (len(open(d + '/lifted_trace.bin', 'rb').read()) // 4),
                                open(d + '/lifted_trace.bin', 'rb').read()))
    print('recompiled trace: %d function entries' % len(lifted))
    # Walk both sequences; tolerate the original reaching extra entries via
    # fall-through/jumps that the recompiled code took as gotos.
    i = j = 0
    while i < len(trace) and j < len(lifted):
        if trace[i] == lifted[j]:
            i += 1
            j += 1
            continue
        if lifted[j] in trace[i:i + 4]:
            i = trace.index(lifted[j], i)
            continue
        print('DIVERGENCE at original #%d / recompiled #%d' % (i, j))
        print('  original  :', ' '.join('%x' % v for v in trace[max(0, i - 8):i + 8]))
        print('  recompiled:', ' '.join('%x' % v for v in lifted[max(0, j - 8):j + 8]))
        return 1
    print('no divergence in the compared prefix (orig %d/%d, lifted %d/%d)' % (i, len(trace), j, len(lifted)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
