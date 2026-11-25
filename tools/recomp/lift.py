#!/usr/bin/env python3
"""Static recompiler: MMU.exe (x86-32, MSVC 2008) -> portable C.

Every guest function becomes `void f_<va>(Cpu* c)` operating on a flat 32-bit
guest address space (image mapped at its original 0x400000 base; the PE has no
relocations). Semantics are instruction-exact:

* integer registers are cached in C locals and synced around calls;
* EFLAGS bits are materialised only where a later instruction reads them
  (backward liveness over each function's CFG);
* the x87 stack is emulated with IEEE doubles. MSVC's CRT sets precision
  control to 53 bits (_PC_53), so double arithmetic is bit-exact for the game;
  rounding control is honoured for FIST/FISTP;
* direct calls are C calls, jumps to other function entries are tail calls,
  `call/jmp` through registers or memory go through rt_call_indirect();
* imports (IAT slots) call host shims `imp_<name>` (recomp/runtime).

Usage:
  python3 tools/recomp/lift.py <MMU.exe> <out_dir> [--entries extra.txt]
      [--only 0xVA,...] [--files N]

The output is derived from the user's own MMU.exe and must not be committed.
"""

import argparse
import collections
import os
import re
import struct
import sys

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import (X86_OP_IMM, X86_OP_MEM, X86_OP_REG, X86_REG_FS,
                          X86_REG_INVALID)

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# ---------------------------------------------------------------- PE image


class PE:
    def __init__(self, path, rebase=None):
        d = self.data = bytearray(open(path, 'rb').read())
        self.path = path
        pe = struct.unpack_from('<I', d, 0x3c)[0]
        opt = pe + 24
        self.base = struct.unpack_from('<I', d, opt + 28)[0]
        self.entry = self.base + struct.unpack_from('<I', d, opt + 16)[0]
        n = struct.unpack_from('<H', d, pe + 6)[0]
        optsz = struct.unpack_from('<H', d, pe + 20)[0]
        self.sections = []
        for i in range(n):
            o = pe + 24 + optsz + i * 40
            name = d[o:o + 8].rstrip(b'\0').decode('latin1')
            vs, va, rs, ra = struct.unpack_from('<IIII', d, o + 8)
            self.sections.append((name, self.base + va, vs, ra, rs))
        self.text = [s for s in self.sections if s[0] == '.text'][0]
        self.opt = opt
        self.relocs = []
        rel_rva, rel_size = struct.unpack_from('<II', d, opt + 96 + 5 * 8)
        if rel_rva:
            o, end = self.base + rel_rva, self.base + rel_rva + rel_size
            while o < end:
                page, blk = struct.unpack('<II', self.read(o, 8))
                if blk < 8:
                    break
                for k in range((blk - 8) // 2):
                    e = struct.unpack('<H', self.read(o + 8 + 2 * k, 2))[0]
                    if e >> 12 == 3:
                        self.relocs.append(self.base + page + (e & 0xfff))
                o += blk
        if rebase is not None and rebase != self.base:
            delta = (rebase - self.base) & 0xffffffff
            for va in self.relocs:
                off = self.file_off(va)
                v = struct.unpack_from('<I', d, off)[0]
                struct.pack_into('<I', d, off, (v + delta) & 0xffffffff)
            self.relocs = [r - self.base + rebase for r in self.relocs]
            shift = rebase - self.base
            self.sections = [(n, va + shift, vs, ra, rs) for n, va, vs, ra, rs in self.sections]
            self.entry += shift
            self.base = rebase
            self.text = [s for s in self.sections if s[0] == '.text'][0]
        self.exports = {}
        exp_rva, exp_size = struct.unpack_from('<II', d, opt + 96)
        if exp_rva:
            e = self.base + exp_rva
            (_, _, _, _, _, ordbase, nfun, nname, afun, aname, aord) = struct.unpack('<IIHHIIIIIII', self.read(e, 40))
            funcs = [struct.unpack('<I', self.read(self.base + afun + 4 * k, 4))[0] for k in range(nfun)]
            for k in range(nname):
                nrva = struct.unpack('<I', self.read(self.base + aname + 4 * k, 4))[0]
                ordi = struct.unpack('<H', self.read(self.base + aord + 2 * k, 2))[0]
                rva = funcs[ordi]
                name = self.cstr(self.base + nrva)
                fwd = self.cstr(self.base + rva) if exp_rva <= rva < exp_rva + exp_size else None
                self.exports[name] = ('fwd', fwd) if fwd else ('va', self.base + rva)
            for k, rva in enumerate(funcs):
                self.exports.setdefault('#%d' % (ordbase + k), ('va', self.base + rva))
        self.iat = {}
        imp = struct.unpack_from('<I', d, opt + 96 + 8)[0] + self.base
        o = imp
        while True:
            oft, _, _, name, ft = struct.unpack('<IIIII', self.read(o, 20))
            if name == 0:
                break
            dll = self.cstr(name + self.base)
            t, f = (oft or ft) + self.base, ft + self.base
            while True:
                e = struct.unpack('<I', self.read(t, 4))[0]
                if e == 0:
                    break
                sym = ('#%d' % (e & 0xffff)) if e & 0x80000000 else self.cstr(e + self.base + 2)
                self.iat[f] = (dll, sym)
                t += 4
                f += 4
            o += 20

    def file_off(self, va):
        s = self.section(va)
        return s[3] + va - s[1]

    def is_code(self, va):
        return self.in_text(va)

    def section(self, va):
        for s in self.sections:
            if s[1] <= va < s[1] + max(s[2], s[4]):
                return s
        return None

    def read(self, va, n):
        s = self.section(va)
        if not s:
            return None
        off = va - s[1]
        out = self.data[s[3] + off:s[3] + min(off + n, s[4])]
        return out + b'\0' * (n - len(out))

    def u32(self, va):
        b = self.read(va, 4)
        return struct.unpack('<I', b)[0] if b else None

    def cstr(self, va):
        out = bytearray()
        while True:
            b = self.read(va, 1)
            if not b or b == b'\0':
                return out.decode('latin1')
            out += b
            va += 1

    def in_text(self, va):
        return self.text[1] <= va < self.text[1] + self.text[2]


def sanitize(sym):
    return re.sub(r'[^A-Za-z0-9_]', '_', sym)


def import_c_name(dll, sym):
    return 'imp_%s__%s' % (sanitize(dll.split('.')[0].upper()), sanitize(sym))

# ---------------------------------------------------------------- registers

R32 = ['eax', 'ecx', 'edx', 'ebx', 'esp', 'ebp', 'esi', 'edi']
R16 = {'ax': 'eax', 'cx': 'ecx', 'dx': 'edx', 'bx': 'ebx', 'sp': 'esp', 'bp': 'ebp',
       'si': 'esi', 'di': 'edi'}
R8L = {'al': 'eax', 'cl': 'ecx', 'dl': 'edx', 'bl': 'ebx'}
R8H = {'ah': 'eax', 'ch': 'ecx', 'dh': 'edx', 'bh': 'ebx'}
FLAGS = ('cf', 'pf', 'zf', 'sf', 'of')
ALLF = frozenset(FLAGS)
MASK = {1: '0xffu', 2: '0xffffu', 4: '0xffffffffu', 8: '0xffffffffffffffffull'}
SIGN = {1: '0x80u', 2: '0x8000u', 4: '0x80000000u'}
UT = {1: 'uint8_t', 2: 'uint16_t', 4: 'uint32_t', 8: 'uint64_t'}
ST = {1: 'int8_t', 2: 'int16_t', 4: 'int32_t', 8: 'int64_t'}

COND = {
    'o': 'f_of', 'no': '!f_of', 'b': 'f_cf', 'ae': '!f_cf', 'e': 'f_zf', 'ne': '!f_zf',
    'be': '(f_cf|f_zf)', 'a': '!(f_cf|f_zf)', 's': 'f_sf', 'ns': '!f_sf', 'p': 'f_pf',
    'np': '!f_pf', 'l': '(f_sf!=f_of)', 'ge': '(f_sf==f_of)', 'le': '(f_zf|(f_sf!=f_of))',
    'g': '(!f_zf&&(f_sf==f_of))',
}
COND_USES = {
    'o': {'of'}, 'no': {'of'}, 'b': {'cf'}, 'ae': {'cf'}, 'e': {'zf'}, 'ne': {'zf'},
    'be': {'cf', 'zf'}, 'a': {'cf', 'zf'}, 's': {'sf'}, 'ns': {'sf'}, 'p': {'pf'},
    'np': {'pf'}, 'l': {'sf', 'of'}, 'ge': {'sf', 'of'}, 'le': {'zf', 'sf', 'of'},
    'g': {'zf', 'sf', 'of'},
}
JCC = {'j' + k: k for k in COND}
JCC.update({'jc': 'b', 'jnc': 'ae', 'jz': 'e', 'jnz': 'ne', 'jnae': 'b', 'jnb': 'ae',
            'jna': 'be', 'jnbe': 'a', 'jpe': 'p', 'jpo': 'np', 'jnge': 'l', 'jnl': 'ge',
            'jng': 'le', 'jnle': 'g'})
SETCC = {'set' + k: k for k in COND}


class LiftError(Exception):
    pass

# ---------------------------------------------------------------- lifter


class Lifter:
    def __init__(self, pe):
        self.pe = pe
        self.md = Cs(CS_ARCH_X86, CS_MODE_32)
        self.md.detail = True
        self.cache = {}
        self.entries = set()
        self.unsupported = collections.Counter()

    def insn(self, va):
        i = self.cache.get(va)
        if i is None:
            code = self.pe.read(va, 16)
            if code is None:
                return None
            i = next(self.md.disasm(code, va), None)
            self.cache[va] = i
        return i

    # ---- operand helpers
    def rn(self, i, r):
        return i.reg_name(r)

    def addr(self, i, op):
        m = op.mem
        parts = []
        if m.base != X86_REG_INVALID:
            parts.append(self.reg_read32(self.rn(i, m.base)))
        if m.index != X86_REG_INVALID:
            idx = self.reg_read32(self.rn(i, m.index))
            parts.append(idx if m.scale == 1 else '%s*%du' % (idx, m.scale))
        if m.disp or not parts:
            parts.append('0x%xu' % (m.disp & 0xffffffff))
        e = '(uint32_t)(' + '+'.join(parts) + ')'
        if m.segment == X86_REG_FS:
            e = '(uint32_t)(RT_FS_BASE+%s)' % e
        return e

    def reg_read32(self, name):
        if name in R32:
            return name
        raise LiftError('reg ' + name)

    def rd(self, i, op, size=None):
        size = size or op.size
        if op.type == X86_OP_REG:
            n = self.rn(i, op.reg)
            if n in R32:
                return n
            if n in R16:
                return '(uint16_t)%s' % R16[n]
            if n in R8L:
                return '(uint8_t)%s' % R8L[n]
            if n in R8H:
                return '(uint8_t)(%s>>8)' % R8H[n]
            raise LiftError('reg ' + n)
        if op.type == X86_OP_IMM:
            return '0x%xu' % (op.imm & ((1 << (8 * size)) - 1))
        if op.type == X86_OP_MEM:
            return 'R%d(%s)' % (size * 8, self.addr(i, op))
        raise LiftError('operand')

    def wr(self, i, op, val):
        if op.type == X86_OP_REG:
            n = self.rn(i, op.reg)
            if n in R32:
                return '%s=(uint32_t)(%s);' % (n, val)
            if n in R16:
                r = R16[n]
                return '%s=(%s&0xffff0000u)|(uint16_t)(%s);' % (r, r, val)
            if n in R8L:
                r = R8L[n]
                return '%s=(%s&0xffffff00u)|(uint8_t)(%s);' % (r, r, val)
            if n in R8H:
                r = R8H[n]
                return '%s=(%s&0xffff00ffu)|((uint32_t)(uint8_t)(%s)<<8);' % (r, r, val)
            raise LiftError('reg ' + n)
        if op.type == X86_OP_MEM:
            return 'W%d(%s,%s);' % (op.size * 8, self.addr(i, op), val)
        raise LiftError('write operand')

    # ---- flags
    def flags_logic(self, r, size, live):
        out = []
        if 'cf' in live:
            out.append('f_cf=0;')
        if 'of' in live:
            out.append('f_of=0;')
        out += self.flags_szp(r, size, live)
        return out

    def flags_szp(self, r, size, live):
        out = []
        if 'zf' in live:
            out.append('f_zf=((%s)&%s)==0;' % (r, MASK[size]))
        if 'sf' in live:
            out.append('f_sf=((%s)&%s)!=0;' % (r, SIGN[size]))
        if 'pf' in live:
            out.append('f_pf=PARITY8(%s);' % r)
        return out

    # ---- function discovery
    def jump_table(self, i, fstart, recent=()):
        """jmp dword ptr [reg*4 + table] -> list of in-function targets.

        The entry count comes from the `cmp idx, N; ja default` guard MSVC
        emits just before the indexed jump (N+1 entries)."""
        op = i.operands[0]
        if op.type != X86_OP_MEM or op.mem.scale != 4 or op.mem.base != X86_REG_INVALID:
            return None
        table = op.mem.disp & 0xffffffff
        count = None
        for p in reversed(recent):
            if p.mnemonic == 'cmp' and len(p.operands) == 2 and p.operands[1].type == X86_OP_IMM:
                count = (p.operands[1].imm & 0xffffffff) + 1
                break
        if count is None or count > 1024:
            return None
        out = []
        for k in range(count):
            t = self.pe.u32(table + 4 * k)
            if t is None or not self.pe.in_text(t) or abs(t - i.address) > 0x20000:
                break
            if k and (table + 4 * k) in self.entries_data_guard:
                break
            out.append(t)
        return out or None

    def collect(self, entry):
        """Recursive traversal. Returns (insns{va:insn}, labels, tables{va:[targets]})."""
        insns, tables = {}, {}
        work = [entry]
        while work:
            va = work.pop()
            recent = []
            while va not in insns:
                i = self.insn(va)
                if i is None or not self.pe.in_text(va):
                    raise LiftError('bad code at 0x%x' % va)
                insns[va] = i
                recent = (recent + [i])[-6:]
                m = i.mnemonic
                nxt = va + i.size
                if m in JCC or m in ('jecxz', 'loop', 'loope', 'loopne'):
                    t = i.operands[0].imm & 0xffffffff
                    if t not in self.entries or t == entry:
                        work.append(t)
                elif m == 'jmp':
                    op = i.operands[0]
                    if op.type == X86_OP_IMM:
                        t = op.imm & 0xffffffff
                        if t not in self.entries or t == entry:
                            work.append(t)
                    else:
                        tt = self.jump_table(i, entry, recent[:-1])
                        if tt:
                            tables[va] = tt
                            work.extend(t for t in tt if t not in self.entries or t == entry)
                    break
                elif m in ('ret', 'retn', 'repz ret', 'int3', 'hlt', 'ud2'):
                    break
                elif m == 'call':
                    op = i.operands[0]
                    if op.type == X86_OP_IMM:
                        t = op.imm & 0xffffffff
                        self.new_entries.add(t)
                        if t in self.noreturn:
                            break
                va = nxt
        return insns, tables

    # ---- liveness
    def flag_effect(self, i):
        """(uses, defs) flag sets for one instruction."""
        m = i.mnemonic
        if m.startswith('lock '):
            m = m[5:]
        if m in ('xadd', 'cmpxchg'):
            return set(), set(ALLF)
        if m in ('stc', 'clc'):
            return set(), {'cf'}
        if m == 'cmc':
            return {'cf'}, {'cf'}
        if m in ('lahf',):
            return set(ALLF), set()
        if m in ('sahf',):
            return set(), set(ALLF)
        if m.startswith('cmov'):
            cc = m[4:]
            cc = {'z': 'e', 'nz': 'ne', 'c': 'b', 'nc': 'ae'}.get(cc, cc)
            return COND_USES.get(cc, set(ALLF)), set()
        if m in ('bt', 'bts', 'btr', 'btc'):
            return set(), {'cf'}
        if re.match(r'^(repe |repne |repz |repnz |rep )?(scas|cmps)[bwd]$', m):
            return set(), set(ALLF)
        if m in JCC:
            return COND_USES[JCC[m]], set()
        if m in SETCC:
            return COND_USES[SETCC[m]], set()
        if m in ('add', 'sub', 'cmp', 'neg', 'and', 'or', 'xor', 'test'):
            return set(), set(ALLF)
        if m in ('adc', 'sbb'):
            return {'cf'}, set(ALLF)
        if m in ('inc', 'dec'):
            return set(), {'pf', 'zf', 'sf', 'of'}
        if m in ('shl', 'sal', 'shr', 'sar', 'shld', 'shrd', 'rcr', 'rcl', 'rol', 'ror'):
            cnt = i.operands[-1]
            u = {'cf'} if m in ('rcr', 'rcl') else set()
            if cnt.type == X86_OP_IMM and (cnt.imm & 31):
                return u, set(ALLF) if m not in ('rol', 'ror', 'rcr', 'rcl') else {'cf', 'of'}
            return set(ALLF), set(ALLF)
        if m in ('imul', 'mul'):
            return set(), set(ALLF)
        if m in ('div', 'idiv'):
            return set(), set(ALLF)
        if m in ('call',):
            return set(), set(ALLF)
        if m == 'pushfd':
            return set(ALLF), set()
        if m == 'popfd':
            return set(), set(ALLF)
        if m == 'cpuid':
            return set(), set()
        if m in ('comisd', 'ucomisd', 'comiss', 'ucomiss', 'fcomi', 'fcomip', 'fucomi', 'fucomip',
                 'fucompi', 'fcompi'):
            return set(), set(ALLF)
        if m.startswith('fcmov'):
            return set(ALLF), set()
        if m in ('loope', 'loopne'):
            return {'zf'}, set()
        return set(), set()

    def liveness(self, insns, succ):
        order = sorted(insns)
        live_in = {va: set() for va in order}
        changed = True
        while changed:
            changed = False
            for va in reversed(order):
                out = set()
                for s in succ[va]:
                    out |= live_in.get(s, ALLF if s is None else set())
                u, d = self.flag_effect(insns[va])
                new = u | (out - d)
                if new != live_in[va]:
                    live_in[va] = new
                    changed = True
        live_out = {}
        for va in order:
            out = set()
            for s in succ[va]:
                out |= live_in.get(s, ALLF if s is None else set())
            live_out[va] = out
        return live_out

    # ---- codegen
    def lift_function(self, entry):
        insns, tables = self.collect(entry)
        succ = {}
        labels = set()
        for va, i in insns.items():
            m = i.mnemonic
            nxt = va + i.size
            if m in JCC or m in ('jecxz', 'loop', 'loope', 'loopne'):
                t = i.operands[0].imm & 0xffffffff
                succ[va] = [nxt] + ([t] if t in insns else [])
                if t in insns:
                    labels.add(t)
            elif m == 'jmp':
                op = i.operands[0]
                if op.type == X86_OP_IMM and (op.imm & 0xffffffff) in insns and \
                        ((op.imm & 0xffffffff) not in self.entries or (op.imm & 0xffffffff) == entry):
                    t = op.imm & 0xffffffff
                    succ[va] = [t]
                    labels.add(t)
                elif va in tables:
                    succ[va] = [t for t in tables[va] if t in insns] + [None]
                    labels.update(t for t in tables[va] if t in insns)
                else:
                    succ[va] = []
            elif m in ('ret', 'retn', 'repz ret', 'int3', 'hlt', 'ud2'):
                succ[va] = []
            elif m == 'call' and i.operands[0].type == X86_OP_IMM and \
                    (i.operands[0].imm & 0xffffffff) in self.noreturn:
                succ[va] = []
            else:
                succ[va] = [nxt]
        # Code addresses used as data (callback/thunk tables built at run
        # time, e.g. `mov [ebp-x], offset thunk`) become entries too.
        for i in insns.values():
            if i.mnemonic in JCC or i.mnemonic in ('call', 'jmp', 'jecxz', 'loop', 'loope', 'loopne'):
                continue
            for op in i.operands:
                if op.type == X86_OP_IMM:
                    v = op.imm & 0xffffffff
                    if v not in self.entries and self.pe.in_text(v) and plausible_code(self, v):
                        self.new_entries.add(v)
        live = self.liveness(insns, succ)
        order = sorted(insns)
        for k, va in enumerate(order):
            nxt = va + insns[va].size
            if nxt in succ[va] and (k + 1 >= len(order) or order[k + 1] != nxt) and nxt in insns:
                labels.add(nxt)
        body = []
        if order[0] != entry:
            # Code below the entry address (shared tails reached by jumps) is
            # emitted first in address order; start at the real entry.
            labels.add(entry)
            body.append('goto L_%x;' % entry)
        for k, va in enumerate(order):
            i = insns[va]
            if va in labels:
                body.append('L_%x:;' % va)
            try:
                code = self.emit(i, live[va], entry, insns, tables)
            except LiftError as e:
                self.unsupported['%s (%s)' % (i.mnemonic, e)] += 1
                code = ['RT_UNSUPPORTED(0x%xu, "%s %s");' % (va, i.mnemonic, i.op_str.replace('"', ''))]
            if va in HOOKS and self.pe.mod(va) is self.pe.modules[0]:
                code = ['SYNC_OUT; %s(c); SYNC_IN;' % HOOKS[va]] + code
            if va in SKIP_HOOKS and va == entry:  # also DLL functions (relocated VAs)
                name, n = SKIP_HOOKS[va]
                code = ['SYNC_OUT; if (%s(c)) { SYNC_IN; RT_RET_CHECK(0x%xu); esp+=%du; SYNC_OUT; return; } SYNC_IN;'
                        % (name, entry, 4 + n)] + code
            body.append('/*%x: %s %s*/ ' % (va, i.mnemonic, i.op_str) + ' '.join(code))
            # explicit fall-through when the next emitted insn is not adjacent
            if succ[va] and (va + i.size) in succ[va] and i.mnemonic not in ('jmp',):
                if k + 1 >= len(order) or order[k + 1] != va + i.size:
                    if (va + i.size) in insns:
                        body.append('goto L_%x;' % (va + i.size))
                    else:
                        body.append('SYNC_OUT; f_%x(c); return;' % (va + i.size))
                        self.new_entries.add(va + i.size)
        out = ['void f_%x(Cpu* c) {' % entry,
               '  uint32_t eax,ecx,edx,ebx,esp,ebp,esi,edi; int f_cf=0,f_pf=0,f_zf=0,f_sf=0,f_of=0;',
               '  (void)f_cf;(void)f_pf;(void)f_zf;(void)f_sf;(void)f_of;',
               '  SYNC_IN; RT_ENTER(0x%xu); uint32_t esp0_=esp; (void)esp0_;' % entry]
        out += ['  ' + b for b in body]
        out.append('}')
        return '\n'.join(out)

    def target_call(self, t):
        """C statement(s) performing a call to guest VA t (return addr pushed)."""
        if t in self.import_thunks:
            return '%s(c);' % self.import_thunks[t]
        self.new_entries.add(t)
        return 'f_%x(c);' % t

    def emit(self, i, live, entry, insns, tables):
        m = i.mnemonic
        if m.startswith('lock '):
            m = m[5:]  # single guest thread: atomicity is implicit
        ops = i.operands
        va = i.address
        nxt = va + i.size
        o = []
        if m in ('nop', 'wait', 'fwait', 'pause', 'lfence', 'mfence', 'sfence'):
            return []
        if m in ('mov',):
            return [self.wr(i, ops[0], self.rd(i, ops[1], ops[0].size))]
        if m in ('movzx', 'movsx'):
            src = self.rd(i, ops[1])
            if m == 'movsx':
                src = '(uint32_t)(int32_t)(%s)(%s)' % (ST[ops[1].size], src)
            return [self.wr(i, ops[0], src)]
        if m == 'lea':
            return [self.wr(i, ops[0], self.addr(i, ops[1]).replace('RT_FS_BASE+', ''))]
        if m == 'push':
            v = self.rd(i, ops[0], 4)
            if ops[0].type == X86_OP_IMM:
                v = '0x%xu' % (ops[0].imm & 0xffffffff)
            return ['{uint32_t t_=%s; esp-=4; W32(esp,t_);}' % v]
        if m == 'pop':
            if ops[0].type == X86_OP_REG and self.rn(i, ops[0].reg) == 'esp':
                return ['esp=R32(esp);']
            return ['{uint32_t t_=R32(esp); esp+=4; %s}' % self.wr(i, ops[0], 't_')]
        if m == 'xchg':
            a, b = ops
            return ['{uint32_t t_=%s; %s %s}' % (self.rd(i, a), self.wr(i, a, self.rd(i, b)),
                                               self.wr(i, b, 't_'))]
        if m == 'leave':
            return ['esp=ebp; ebp=R32(esp); esp+=4;']
        if m == 'cdq':
            return ['edx=(uint32_t)((int32_t)eax>>31);']
        if m == 'cwde':
            return ['eax=(uint32_t)(int32_t)(int16_t)eax;']
        if m in ('add', 'sub', 'cmp', 'adc', 'sbb'):
            s = ops[0].size
            a, b = self.rd(i, ops[0]), self.rd(i, ops[1], s)
            cin = 'f_cf' if m in ('adc', 'sbb') else '0'
            o.append('{uint64_t a_=%s, b_=%s, ci_=%s;' % (a, b, cin))
            if m in ('add', 'adc'):
                o.append('uint64_t r_=a_+b_+ci_;')
                if 'cf' in live:
                    o.append('f_cf=(r_>>%d)&1;' % (s * 8))
                if 'of' in live:
                    o.append('f_of=(((a_^r_)&(b_^r_))&%s)!=0;' % SIGN[s])
            else:
                o.append('uint64_t r_=a_-b_-ci_;')
                if 'cf' in live:
                    o.append('f_cf=(a_<b_+ci_);')
                if 'of' in live:
                    o.append('f_of=(((a_^b_)&(a_^r_))&%s)!=0;' % SIGN[s])
            o += self.flags_szp('r_', s, live)
            if m != 'cmp':
                o.append(self.wr(i, ops[0], '(%s)r_' % UT[s]))
            o.append('}')
            return o
        if m in ('and', 'or', 'xor', 'test'):
            s = ops[0].size
            # xor r,r / sub r,r idioms are fine generically
            opc = {'and': '&', 'test': '&', 'or': '|', 'xor': '^'}[m]
            o.append('{uint32_t r_=(%s)(%s %s %s);' % (UT[s], self.rd(i, ops[0]), opc,
                                                      self.rd(i, ops[1], s)))
            o += self.flags_logic('r_', s, live)
            if m != 'test':
                o.append(self.wr(i, ops[0], 'r_'))
            o.append('}')
            return o
        if m in ('inc', 'dec'):
            s = ops[0].size
            sign = '+' if m == 'inc' else '-'
            o.append('{uint32_t a_=%s; uint32_t r_=(%s)(a_%s1u);' % (self.rd(i, ops[0]), UT[s], sign))
            if 'of' in live:
                o.append('f_of=(r_==%s);' % SIGN[s] if m == 'inc' else 'f_of=(a_==%s);' % SIGN[s])
            o += self.flags_szp('r_', s, live)
            o.append(self.wr(i, ops[0], 'r_') + '}')
            return o
        if m == 'neg':
            s = ops[0].size
            o.append('{uint32_t a_=%s; uint32_t r_=(%s)(0u-a_);' % (self.rd(i, ops[0]), UT[s]))
            if 'cf' in live:
                o.append('f_cf=(a_!=0);')
            if 'of' in live:
                o.append('f_of=(a_==%s);' % SIGN[s])
            o += self.flags_szp('r_', s, live)
            o.append(self.wr(i, ops[0], 'r_') + '}')
            return o
        if m == 'not':
            return [self.wr(i, ops[0], '~(%s)' % self.rd(i, ops[0]))]
        if m in ('shl', 'sal', 'shr', 'sar'):
            s = ops[0].size
            bits = s * 8
            cnt = self.rd(i, ops[1], 1) if len(ops) > 1 else '1u'
            o.append('{uint32_t n_=(%s)&31u; if(n_){ uint32_t a_=%s; uint32_t r_;' % (cnt, self.rd(i, ops[0])))
            if m in ('shl', 'sal'):
                o.append('r_=(%s)((uint64_t)a_<<n_); uint32_t cf_=(uint32_t)(((uint64_t)a_<<n_)>>%d)&1u;' % (UT[s], bits))
                ofx = '((r_&%s)!=0)^cf_' % SIGN[s]
            elif m == 'shr':
                o.append('r_=(%s)(a_>>n_); uint32_t cf_=(n_<=%d)?((a_>>(n_-1))&1u):0u;' % (UT[s], bits))
                ofx = '((a_&%s)!=0)' % SIGN[s]
            else:
                o.append('int32_t sa_=(int32_t)(%s)a_; uint32_t k_=n_>%d?%d:n_; r_=(%s)(sa_>>k_); '
                         'uint32_t cf_=(uint32_t)(sa_>>(k_-1))&1u;' % (ST[s], bits - 1, bits - 1, UT[s]))
                ofx = '0'
            if 'cf' in live:
                o.append('f_cf=cf_;')
            else:
                o.append('(void)cf_;')
            if 'of' in live:
                o.append('f_of=%s;' % ofx)
            o += self.flags_szp('r_', s, live)
            o.append(self.wr(i, ops[0], 'r_') + '}}')
            return o
        if m in ('shld', 'shrd'):
            d, srcop, cnt = ops
            c_ = self.rd(i, cnt, 1)
            o.append('{uint32_t n_=(%s)&31u; if(n_){ uint32_t a_=%s, b_=%s, r_;' % (c_, self.rd(i, d), self.rd(i, srcop)))
            if m == 'shld':
                o.append('r_=(a_<<n_)|(b_>>(32-n_)); f_cf=(a_>>(32-n_))&1u;')
            else:
                o.append('r_=(a_>>n_)|(b_<<(32-n_)); f_cf=(a_>>(n_-1))&1u;')
            o.append('f_of=0;')
            o += self.flags_szp('r_', 4, live)
            o.append(self.wr(i, d, 'r_') + '}}')
            return o
        if m in ('rcr', 'rcl', 'rol', 'ror'):
            s = ops[0].size
            bits = s * 8
            cnt = self.rd(i, ops[1], 1) if len(ops) > 1 else '1u'
            if m in ('rol', 'ror'):
                o.append('{uint32_t n_=((%s)&31u)%%%d; uint32_t a_=%s; if(n_){' % (cnt, bits, self.rd(i, ops[0])))
                if m == 'rol':
                    o.append('uint32_t r_=(%s)((a_<<n_)|(a_>>(%d-n_))); f_cf=r_&1u;' % (UT[s], bits))
                else:
                    o.append('uint32_t r_=(%s)((a_>>n_)|(a_<<(%d-n_))); f_cf=(r_>>%d)&1u;' % (UT[s], bits, bits - 1))
                o.append(self.wr(i, ops[0], 'r_') + '}}')
                return o
            o.append('{uint32_t n_=(%s)&31u; uint32_t a_=%s; while(n_--){' % (cnt, self.rd(i, ops[0])))
            if m == 'rcr':
                o.append('uint32_t lo_=a_&1u; a_=(%s)((a_>>1)|((uint32_t)f_cf<<%d)); f_cf=lo_;}' % (UT[s], bits - 1))
            else:
                o.append('uint32_t hi_=(a_>>%d)&1u; a_=(%s)((a_<<1)|(uint32_t)f_cf); f_cf=hi_;}' % (bits - 1, UT[s]))
            o.append(self.wr(i, ops[0], 'a_') + '}')
            return o
        if m in ('mul', 'imul') and len(ops) == 1:
            s = ops[0].size
            src = self.rd(i, ops[0])
            if s == 4:
                if m == 'mul':
                    o.append('{uint64_t r_=(uint64_t)eax*(uint64_t)%s; eax=(uint32_t)r_; edx=(uint32_t)(r_>>32); f_cf=f_of=(edx!=0);}' % src)
                else:
                    o.append('{int64_t r_=(int64_t)(int32_t)eax*(int64_t)(int32_t)%s; eax=(uint32_t)r_; edx=(uint32_t)((uint64_t)r_>>32); f_cf=f_of=(r_!=(int64_t)(int32_t)eax);}' % src)
            elif s == 1:
                if m == 'mul':
                    o.append('{uint32_t r_=(uint8_t)eax*(uint32_t)%s; eax=(eax&0xffff0000u)|(uint16_t)r_; f_cf=f_of=(r_>>8)!=0;}' % src)
                else:
                    o.append('{int32_t r_=(int32_t)(int8_t)eax*(int32_t)(int8_t)%s; eax=(eax&0xffff0000u)|(uint16_t)r_; f_cf=f_of=(r_!=(int8_t)r_);}' % src)
            else:
                raise LiftError('mul16')
            return o
        if m == 'imul':
            s = ops[0].size
            if len(ops) == 2:
                a, b = self.rd(i, ops[0]), self.rd(i, ops[1], s)
            else:
                a, b = self.rd(i, ops[1]), self.rd(i, ops[2], s)
                if ops[2].type == X86_OP_IMM:
                    b = '0x%xu' % (ops[2].imm & 0xffffffff)
            o.append('{int64_t r_=(int64_t)(%s)%s*(int64_t)(%s)%s; f_cf=f_of=(r_!=(int64_t)(%s)r_);' % (
                ST[s], a, ST[s], b, ST[s]))
            o.append(self.wr(i, ops[0], '(uint32_t)r_') + '}')
            return o
        if m in ('div', 'idiv'):
            s = ops[0].size
            src = self.rd(i, ops[0])
            if s == 4:
                if m == 'div':
                    o.append('{uint64_t n_=((uint64_t)edx<<32)|eax; uint32_t d_=%s; if(!d_) RT_FAULT(0x%xu,"div0"); eax=(uint32_t)(n_/d_); edx=(uint32_t)(n_%%d_);}' % (src, va))
                else:
                    o.append('{int64_t n_=(int64_t)(((uint64_t)edx<<32)|eax); int32_t d_=(int32_t)%s; if(!d_) RT_FAULT(0x%xu,"div0"); eax=(uint32_t)(int32_t)(n_/d_); edx=(uint32_t)(int32_t)(n_%%d_);}' % (src, va))
            elif s == 1:
                if m == 'div':
                    o.append('{uint32_t n_=(uint16_t)eax; uint32_t d_=%s; if(!d_) RT_FAULT(0x%xu,"div0"); eax=(eax&0xffff0000u)|((n_%%d_)<<8)|(uint8_t)(n_/d_);}' % (src, va))
                else:
                    o.append('{int32_t n_=(int16_t)eax; int32_t d_=(int8_t)%s; if(!d_) RT_FAULT(0x%xu,"div0"); eax=(eax&0xffff0000u)|((uint32_t)(uint8_t)(n_%%d_)<<8)|(uint8_t)(n_/d_);}' % (src, va))
            else:
                raise LiftError('div16')
            return o
        if m in SETCC:
            return [self.wr(i, ops[0], '(%s)?1u:0u' % COND[SETCC[m]])]
        if m in JCC:
            t = ops[0].imm & 0xffffffff
            cond = COND[JCC[m]]
            if t in insns and (t not in self.entries or t == entry):
                return ['if(%s) goto L_%x;' % (cond, t)]
            self.new_entries.add(t)
            return ['if(%s){ SYNC_OUT; f_%x(c); return; }' % (cond, t)]
        if m == 'jmp':
            op = ops[0]
            if op.type == X86_OP_IMM:
                t = op.imm & 0xffffffff
                if t in insns and (t not in self.entries or t == entry):
                    return ['goto L_%x;' % t]
                if t in self.import_thunks:
                    return ['SYNC_OUT; %s(c); return;' % self.import_thunks[t]]
                self.new_entries.add(t)
                return ['SYNC_OUT; f_%x(c); return;' % t]
            # indirect jump
            slot = self.iat_slot(op)
            if slot is not None and slot in self.bound:
                self.new_entries.add(self.bound[slot])
                return ['SYNC_OUT; f_%x(c); return;' % self.bound[slot]]
            if slot is not None and slot in self.shims:
                return ['SYNC_OUT; %s(c); return;' % self.shims[slot][2]]
            tgt = self.rd(i, op, 4)
            o.append('{uint32_t t_=%s; switch(t_){' % tgt)
            for t in sorted(set(tables.get(va, []))):
                if t in insns and (t not in self.entries or t == entry):
                    o.append('case 0x%xu: goto L_%x;' % (t, t))
            o.append('default: SYNC_OUT; rt_jump_indirect(c,t_); return; }}')
            return o
        if m == 'call':
            op = ops[0]
            push = 'esp-=4; W32(esp,0x%xu);' % nxt
            if op.type == X86_OP_IMM:
                t = op.imm & 0xffffffff
                if t in self.noreturn and t not in self.import_thunks:
                    self.new_entries.add(t)
                    return [push, 'SYNC_OUT;', 'f_%x(c);' % t, 'return;']
                return [push, 'SYNC_OUT;', self.target_call(t), 'SYNC_IN;']
            slot = self.iat_slot(op)
            if slot is not None and slot in self.bound:
                self.new_entries.add(self.bound[slot])
                return [push, 'SYNC_OUT;', 'f_%x(c);' % self.bound[slot], 'SYNC_IN;']
            if slot is not None and slot in self.shims:
                return [push, 'SYNC_OUT;', '%s(c);' % self.shims[slot][2], 'SYNC_IN;']
            tgt = self.rd(i, op, 4)
            return ['{uint32_t t_=%s;' % tgt, push, 'SYNC_OUT; rt_call_indirect(c,t_); SYNC_IN;}']
        if m in ('ret', 'retn', 'repz ret'):
            n = ops[0].imm if ops else 0
            return ['RT_RET_CHECK(0x%xu); esp+=%du; SYNC_OUT; return;' % (entry, 4 + n)]
        if m in ('int3', 'hlt', 'ud2'):
            return ['SYNC_OUT; RT_FAULT(0x%xu,"%s"); return;' % (va, m)]
        if m == 'cpuid':
            return ['SYNC_OUT; rt_cpuid(c); SYNC_IN;']
        if m == 'pushfd':
            return ['{uint32_t fl_=0x202u|(f_cf?1u:0)|(f_pf?4u:0)|(f_zf?0x40u:0)|(f_sf?0x80u:0)|(f_of?0x800u:0)|c->eflags_extra; esp-=4; W32(esp,fl_);}']
        if m == 'popfd':
            return ['{uint32_t fl_=R32(esp); esp+=4; f_cf=fl_&1; f_pf=(fl_>>2)&1; f_zf=(fl_>>6)&1; f_sf=(fl_>>7)&1; f_of=(fl_>>11)&1; c->eflags_extra=fl_&0x200000u;}']
        sm = re.match(r'^(rep |repe |repne |repz |repnz )?(movs|stos|lods|scas|cmps)([bwd])$', m)
        if sm:
            return self.string_op(sm.group(1), sm.group(2), {'b': 1, 'w': 2, 'd': 4}[sm.group(3)], live)
        if m in ('cld', 'std'):
            return ['c->df=%d;' % (1 if m == 'std' else 0)]
        if m == 'cmc':
            return ['f_cf=!f_cf;']
        if m == 'stc':
            return ['f_cf=1;']
        if m == 'clc':
            return ['f_cf=0;']
        if m == 'xadd':
            s_ = ops[0].size
            o.append('{uint64_t a_=%s, b_=%s; uint64_t r_=a_+b_; f_cf=(r_>>%d)&1; f_of=(((a_^r_)&(b_^r_))&%s)!=0;' % (
                self.rd(i, ops[0]), self.rd(i, ops[1]), s_ * 8, SIGN[s_]))
            o += self.flags_szp('r_', s_, ALLF)
            o.append(self.wr(i, ops[1], '(%s)a_' % UT[s_]) + self.wr(i, ops[0], '(%s)r_' % UT[s_]) + '}')
            return o
        if m == 'cmpxchg':
            s_ = ops[0].size
            acc = {1: '(uint8_t)eax', 2: '(uint16_t)eax', 4: 'eax'}[s_]
            o.append('{uint64_t a_=%s, b_=%s; uint64_t r_=a_-b_; f_cf=a_<b_; f_of=(((a_^b_)&(a_^r_))&%s)!=0;' % (acc, self.rd(i, ops[0]), SIGN[s_]))
            o += self.flags_szp('r_', s_, ALLF)
            o.append('if(f_zf){%s} else {%s}}' % (self.wr(i, ops[0], self.rd(i, ops[1])),
                     {1: 'eax=(eax&0xffffff00u)|(uint8_t)b_;', 2: 'eax=(eax&0xffff0000u)|(uint16_t)b_;', 4: 'eax=(uint32_t)b_;'}[s_]))
            return o
        if m == 'lahf':
            return ['eax=(eax&0xffff00ffu)|((uint32_t)(0x02u|(f_cf?1u:0)|(f_pf?4u:0)|(f_zf?0x40u:0)|(f_sf?0x80u:0))<<8);']
        if m == 'sahf':
            return ['{uint32_t h_=(eax>>8)&0xffu; f_cf=h_&1; f_pf=(h_>>2)&1; f_zf=(h_>>6)&1; f_sf=(h_>>7)&1;}']
        if m.startswith('cmov'):
            cc = m[4:]
            cc = {'z': 'e', 'nz': 'ne', 'c': 'b', 'nc': 'ae'}.get(cc, cc)
            return ['if(%s){%s}' % (COND[cc], self.wr(i, ops[0], self.rd(i, ops[1], ops[0].size)))]
        if m in ('bt', 'bts', 'btr', 'btc'):
            s_ = ops[0].size
            if ops[0].type == X86_OP_MEM and ops[1].type == X86_OP_REG:
                raise LiftError('bt mem,reg')
            o = ['{uint32_t b_=(%s)&%du; uint32_t v_=%s; f_cf=(v_>>b_)&1u;' % (self.rd(i, ops[1], 1 if ops[1].type == X86_OP_IMM else s_), s_ * 8 - 1, self.rd(i, ops[0]))]
            if m != 'bt':
                expr = {'bts': 'v_|(1u<<b_)', 'btr': 'v_&~(1u<<b_)', 'btc': 'v_^(1u<<b_)'}[m]
                o.append(self.wr(i, ops[0], expr))
            o.append('}')
            return o
        if m == 'jecxz':
            t = ops[0].imm & 0xffffffff
            if t in insns and (t not in self.entries or t == entry):
                return ['if(!ecx) goto L_%x;' % t]
            self.new_entries.add(t)
            return ['if(!ecx){ SYNC_OUT; f_%x(c); return; }' % t]
        if m in ('rep movsd', 'rep movsb', 'rep stosd', 'rep stosb'):
            unit = 4 if m.endswith('d') else 1
            if 'movs' in m:
                return ['rt_memmove_guest(edi,esi,ecx*%du); esi+=ecx*%du; edi+=ecx*%du; ecx=0;' % (unit, unit, unit)]
            return ['rt_stos(edi,eax,ecx,%d); edi+=ecx*%du; ecx=0;' % (unit, unit)]
        if m in ('movsd', 'movsb') and not ops:
            unit = 4 if m == 'movsd' else 1
            return ['W%d(edi,R%d(esi)); esi+=%d; edi+=%d;' % (unit * 8, unit * 8, unit, unit)]
        if any(o_.type == X86_OP_REG and re.match(r'^mm\d$', self.rn(i, o_.reg)) for o_ in ops):
            return self.emit_mmx(i)
        if any(o_.type == X86_OP_REG and self.rn(i, o_.reg).startswith('xmm') for o_ in ops) or \
                m in ('stmxcsr', 'ldmxcsr', 'cvttsd2si', 'cvtsd2si', 'cvttss2si', 'cvtss2si'):
            return self.emit_sse(i, live)
        if m in ('emms', 'femms') or m.startswith('prefetch'):
            return []
        if m == 'bswap':
            r = self.rn(i, ops[0].reg)
            return ['%s=((%s>>24)|((%s>>8)&0xff00u)|((%s<<8)&0xff0000u)|(%s<<24));' % (r, r, r, r, r)]
        if m == 'xlatb':
            return ['eax=(eax&0xffffff00u)|R8((uint32_t)(ebx+(uint8_t)eax));']
        if m in ('loop', 'loope', 'loopne') and ops[0].type == X86_OP_IMM:
            t = ops[0].imm & 0xffffffff
            cond = {'loop': 'ecx', 'loope': '(ecx&&f_zf)', 'loopne': '(ecx&&!f_zf)'}[m]
            if t in insns and (t not in self.entries or t == entry):
                return ['ecx--; if(%s) goto L_%x;' % (cond, t)]
            self.new_entries.add(t)
            return ['ecx--; if(%s){ SYNC_OUT; f_%x(c); return; }' % (cond, t)]
        if m.startswith('f'):
            return self.emit_x87(i, live)
        raise LiftError('unsupported')

    def string_op(self, prefix, kind, unit, live):
        """movs/stos/lods/scas/cmps with optional rep/repe/repne (DF honoured)."""
        bits = unit * 8
        step = '(c->df?(uint32_t)-%d:%du)' % (unit, unit)
        acc = {1: '(uint8_t)eax', 2: '(uint16_t)eax', 4: 'eax'}[unit]
        def wacc(v):
            return {1: 'eax=(eax&0xffffff00u)|(uint8_t)(%s);', 2: 'eax=(eax&0xffff0000u)|(uint16_t)(%s);',
                    4: 'eax=(uint32_t)(%s);'}[unit] % v
        body = {
            'movs': 'W%d(edi,R%d(esi)); esi+=%s; edi+=%s;' % (bits, bits, step, step),
            'stos': 'W%d(edi,%s); edi+=%s;' % (bits, acc, step),
            'lods': wacc('R%d(esi)' % bits) + ' esi+=%s;' % step,
            'scas': '{uint64_t a_=%s, b_=R%d(edi); uint64_t r_=a_-b_; f_cf=a_<b_; f_of=(((a_^b_)&(a_^r_))&%s)!=0; f_zf=((r_)&%s)==0; f_sf=((r_)&%s)!=0; f_pf=PARITY8(r_);} edi+=%s;' % (acc, bits, SIGN[unit], MASK[unit], SIGN[unit], step),
            'cmps': '{uint64_t a_=R%d(esi), b_=R%d(edi); uint64_t r_=a_-b_; f_cf=a_<b_; f_of=(((a_^b_)&(a_^r_))&%s)!=0; f_zf=((r_)&%s)==0; f_sf=((r_)&%s)!=0; f_pf=PARITY8(r_);} esi+=%s; edi+=%s;' % (bits, bits, SIGN[unit], MASK[unit], SIGN[unit], step, step),
        }[kind]
        if not prefix:
            return [body]
        prefix = prefix.strip()
        if kind in ('scas', 'cmps') and prefix != 'rep':
            stop = '!f_zf' if prefix in ('repe', 'repz') else 'f_zf'
            return ['while(ecx){ %s ecx--; if(%s) break; }' % (body, stop)]
        if kind in ('movs', 'stos') and prefix == 'rep':
            fast = {'movs': 'rt_memmove_guest(edi,esi,ecx*%du); esi+=ecx*%du; edi+=ecx*%du; ecx=0;' % (unit, unit, unit),
                    'stos': 'rt_stos(edi,eax,ecx,%d); edi+=ecx*%du; ecx=0;' % (unit, unit)}[kind]
            if unit == 2:
                fast = None
            if fast:
                return ['if(!c->df && (%s)){ %s } else while(ecx){ %s ecx--; }' % (
                    '1' if kind == 'stos' else '(edi<=esi||edi>=esi+ecx*%du)' % unit, fast, body)]
        return ['while(ecx){ %s ecx--; }' % body]

    def iat_slot(self, op):
        if op.type == X86_OP_MEM and op.mem.base == X86_REG_INVALID and \
                op.mem.index == X86_REG_INVALID and (op.mem.disp & 0xffffffff) in self.pe.iat:
            return op.mem.disp & 0xffffffff
        return None

    def emit_mmx(self, i):
        m = i.mnemonic
        ops = i.operands
        ismm = lambda op: op.type == X86_OP_REG and re.match(r'^mm\d$', self.rn(i, op.reg))
        R = lambda op: 'c->mm[%d]' % int(self.rn(i, op.reg)[2:])
        def src64(op):
            if ismm(op):
                return '%s.q' % R(op)
            if op.type == X86_OP_MEM:
                return 'R64(%s)' % self.addr(i, op)
            raise LiftError('mmx src')
        d = ops[0]
        s_ = ops[1] if len(ops) > 1 else None
        if m == 'movq':
            if ismm(d):
                return ['%s.q=%s;' % (R(d), src64(s_))]
            return ['W64(%s,%s.q);' % (self.addr(i, d), R(s_))]
        if m == 'movd':
            if ismm(d):
                return ['%s.q=(uint64_t)%s;' % (R(d), self.rd(i, s_, 4))]
            return [self.wr(i, d, '%s.d[0]' % R(s_))]
        x = R(d)
        if m in ('pand', 'por', 'pxor', 'pandn'):
            expr = {'pand': '%s.q&s_', 'por': '%s.q|s_', 'pxor': '%s.q^s_', 'pandn': '~%s.q&s_'}[m] % x
            return ['{uint64_t s_=%s; %s.q=%s;}' % (src64(s_), x, expr)]
        mm = re.match(r'^punpck(l|h)(bw|wd|dq)$', m)
        if mm:
            half, kind = mm.groups()
            n, f = {'bw': (4, 'b'), 'wd': (2, 'w'), 'dq': (1, 'd')}[kind]
            off = 0 if half == 'l' else n
            return ['{__typeof__(%s) s_, d_=%s; s_.q=%s; for(int k_=0;k_<%d;++k_){ %s.%s[2*k_]=d_.%s[%d+k_]; %s.%s[2*k_+1]=s_.%s[%d+k_]; }}' % (
                x, x, src64(s_), n, x, f, f, off, x, f, f, off)]
        if m == 'packuswb':
            return ['{__typeof__(%s) s_, d_=%s; s_.q=%s; for(int k_=0;k_<4;++k_){ int16_t a_=(int16_t)d_.w[k_], b_=(int16_t)s_.w[k_]; %s.b[k_]=(uint8_t)(a_<0?0:a_>255?255:a_); %s.b[4+k_]=(uint8_t)(b_<0?0:b_>255?255:b_); }}' % (x, x, src64(s_), x, x)]
        mm = re.match(r'^p(add|sub)(b|w|d|q)$', m)
        if mm:
            kind, ln = mm.groups()
            n, f = {'b': (8, 'b'), 'w': (4, 'w'), 'd': (2, 'd'), 'q': (1, 'q')}[ln]
            if ln == 'q':
                return ['%s.q%s=%s;' % (x, '+' if kind == 'add' else '-', src64(s_))]
            return ['{__typeof__(%s) s_; s_.q=%s; for(int k_=0;k_<%d;++k_) %s.%s[k_]%s=s_.%s[k_];}' % (
                x, src64(s_), n, x, f, '+' if kind == 'add' else '-', f)]
        if m in ('pmullw', 'pmulhw'):
            sh = '' if m == 'pmullw' else '>>16'
            return ['{__typeof__(%s) s_; s_.q=%s; for(int k_=0;k_<4;++k_) %s.w[k_]=(uint16_t)(((int32_t)(int16_t)%s.w[k_]*(int32_t)(int16_t)s_.w[k_])%s);}' % (
                x, src64(s_), x, x, sh)]
        mm = re.match(r'^ps(rl|ll|ra)(w|d|q)$', m)
        if mm:
            kind, ln = mm.groups()
            cnt = '(uint64_t)%d' % (ops[1].imm & 0xff) if ops[1].type == X86_OP_IMM else src64(s_)
            n, f, bits, st = {'w': (4, 'w', 16, 'int16_t'), 'd': (2, 'd', 32, 'int32_t'), 'q': (1, 'q', 64, 'int64_t')}[ln]
            op = '>>' if kind == 'rl' else '<<'
            if ln == 'q':
                if kind == 'ra':
                    return ['{uint64_t n_=%s; if(n_>63) n_=63; %s.q=(uint64_t)((int64_t)%s.q>>n_);}' % (cnt, x, x)]
                return ['{uint64_t n_=%s; %s.q=n_>=64?0:(%s.q%s(unsigned)n_);}' % (cnt, x, x, op)]
            if kind == 'ra':
                return ['{uint64_t n_=%s; if(n_>%d) n_=%d; for(int k_=0;k_<%d;++k_) %s.%s[k_]=(%s)(%s.%s[k_])>>n_;}' % (cnt, bits - 1, bits - 1, n, x, f, st, x, f)]
            return ['{uint64_t n_=%s; for(int k_=0;k_<%d;++k_) %s.%s[k_]=n_>=%d?0:(%s.%s[k_]%s(unsigned)n_);}' % (cnt, n, x, f, bits, x, f, op)]
        mm = re.match(r'^pcmpeq(b|w|d)$', m)
        if mm:
            n, f = {'b': (8, 'b'), 'w': (4, 'w'), 'd': (2, 'd')}[mm.group(1)]
            return ['{__typeof__(%s) s_; s_.q=%s; for(int k_=0;k_<%d;++k_) %s.%s[k_]=(%s.%s[k_]==s_.%s[k_])?(__typeof__(%s.%s[0]))~0u:0;}' % (
                x, src64(s_), n, x, f, x, f, f, x, f)]
        raise LiftError('mmx')

    def xr(self, i, op):
        if op.type != X86_OP_REG or not self.rn(i, op.reg).startswith('xmm'):
            return None
        return 'c->xmm[%d]' % int(self.rn(i, op.reg)[3:])

    def emit_sse(self, i, live):
        m = i.mnemonic
        ops = i.operands
        isx = lambda op: op.type == X86_OP_REG and self.rn(i, op.reg).startswith('xmm')
        A = lambda op: self.addr(i, op)
        def src_xmm(op, size=16):
            """C expression of type Xmm for a source operand."""
            if isx(op):
                return self.xr(i, op)
            if op.type == X86_OP_MEM:
                return 'rt_ldx(%s,%d)' % (A(op), size)
            raise LiftError('sse src')
        if m in ('stmxcsr',):
            return ['W32(%s,c->mxcsr);' % A(ops[0])]
        if m in ('ldmxcsr',):
            return ['c->mxcsr=R32(%s);' % A(ops[0])]
        d = ops[0]
        s_ = ops[1] if len(ops) > 1 else None
        # ---- moves
        if m in ('movdqa', 'movdqu', 'movaps', 'movups', 'movapd', 'movupd'):
            if isx(d):
                return ['%s=%s;' % (self.xr(i, d), src_xmm(s_))]
            return ['rt_stx(%s,&%s,16);' % (A(d), self.xr(i, s_))]
        if m in ('movsd', 'movss'):
            n = 8 if m == 'movsd' else 4
            lane = 'q[0]' if n == 8 else 'd[0]'
            if isx(d) and isx(s_):
                return ['%s.%s=%s.%s;' % (self.xr(i, d), lane, self.xr(i, s_), lane)]
            if isx(d):
                return ['{Xmm t_=rt_ldx(%s,%d); %s=t_;}' % (A(s_), n, self.xr(i, d))]
            return ['rt_stx(%s,&%s,%d);' % (A(d), self.xr(i, s_), n)]
        if m in ('movlpd', 'movlps'):
            if isx(d):
                return ['%s.q[0]=R64(%s);' % (self.xr(i, d), A(s_))]
            return ['W64(%s,%s.q[0]);' % (A(d), self.xr(i, s_))]
        if m in ('movhpd', 'movhps'):
            if isx(d):
                return ['%s.q[1]=R64(%s);' % (self.xr(i, d), A(s_))]
            return ['W64(%s,%s.q[1]);' % (A(d), self.xr(i, s_))]
        if m == 'movq':
            if isx(d) and isx(s_):
                return ['%s.q[0]=%s.q[0]; %s.q[1]=0;' % (self.xr(i, d), self.xr(i, s_), self.xr(i, d))]
            if isx(d):
                return ['{Xmm t_=rt_ldx(%s,8); %s=t_;}' % (A(s_), self.xr(i, d))]
            return ['W64(%s,%s.q[0]);' % (A(d), self.xr(i, s_))]
        if m == 'movd':
            if isx(d):
                return ['{uint32_t v_=%s; memset(&%s,0,16); %s.d[0]=v_;}' % (self.rd(i, s_, 4), self.xr(i, d), self.xr(i, d))]
            return [self.wr(i, d, '%s.d[0]' % self.xr(i, s_))]
        # ---- scalar / packed float arithmetic
        fops = {'add': '+', 'sub': '-', 'mul': '*', 'div': '/'}
        mm = re.match(r'^(add|sub|mul|div|sqrt|min|max)(sd|ss|pd|ps)$', m)
        if mm:
            kind, form = mm.groups()
            lanes = {'sd': ['dd[0]'], 'ss': ['f[0]'], 'pd': ['dd[0]', 'dd[1]'],
                     'ps': ['f[0]', 'f[1]', 'f[2]', 'f[3]']}[form]
            size = {'sd': 8, 'ss': 4, 'pd': 16, 'ps': 16}[form]
            fl = form[1] == 's'
            out = ['{Xmm s_=%s;' % src_xmm(s_, size)]
            for ln in lanes:
                dl = '%s.%s' % (self.xr(i, d), ln)
                sl = 's_.%s' % ln
                if kind in fops:
                    out.append('%s=%s%s%s;' % (dl, dl, fops[kind], sl))
                elif kind == 'sqrt':
                    out.append('%s=%s(%s);' % (dl, 'sqrtf' if fl else 'sqrt', sl))
                else:
                    cast = '(float)' if fl else ''
                    out.append('%s=%srt_%ssd(%s,%s);' % (dl, cast, kind, dl, sl))
            out.append('}')
            return out
        # ---- bitwise
        mm = re.match(r'^(and|andn|or|xor)(pd|ps)$|^p(and|andn|or|xor)$', m)
        if mm:
            kind = mm.group(1) or mm.group(3)
            x = self.xr(i, d)
            expr = {'and': '%s.q[%d]&s_.q[%d]', 'andn': '~%s.q[%d]&s_.q[%d]', 'or': '%s.q[%d]|s_.q[%d]',
                    'xor': '%s.q[%d]^s_.q[%d]'}[kind]
            return ['{Xmm s_=%s; %s.q[0]=%s; %s.q[1]=%s;}' % (src_xmm(s_), x, expr % (x, 0, 0), x, expr % (x, 1, 1))]
        # ---- shuffles / unpacks
        x = self.xr(i, d)
        if m == 'unpcklpd':
            return ['{Xmm s_=%s; %s.q[1]=s_.q[0];}' % (src_xmm(s_), x)]
        if m == 'unpckhpd':
            return ['{Xmm s_=%s; %s.q[0]=%s.q[1]; %s.q[1]=s_.q[1];}' % (src_xmm(s_), x, x, x)]
        if m == 'shufpd':
            imm = ops[2].imm
            return ['{Xmm s_=%s, d_=%s; %s.q[0]=d_.q[%d]; %s.q[1]=s_.q[%d];}' % (src_xmm(s_), x, x, imm & 1, x, (imm >> 1) & 1)]
        if m == 'pshufd':
            imm = ops[2].imm
            return ['{Xmm s_=%s; %s}' % (src_xmm(s_), ' '.join('%s.d[%d]=s_.d[%d];' % (x, k, (imm >> (2 * k)) & 3) for k in range(4)))]
        if m == 'pextrw':
            return [self.wr(i, d, '%s.w[%d]' % (self.xr(i, s_), ops[2].imm & 7))]
        if m == 'pinsrw':
            v = self.rd(i, s_, 2) if s_.type == X86_OP_MEM else '(uint16_t)%s' % self.rd(i, s_, 4)
            return ['%s.w[%d]=(uint16_t)(%s);' % (x, ops[2].imm & 7, v)]
        if m == 'pmovmskb':
            return [self.wr(i, d, 'rt_pmovmskb(&%s)' % self.xr(i, s_))]
        if m == 'movmskpd':
            return [self.wr(i, d, '(uint32_t)((%s.q[0]>>63)|((%s.q[1]>>63)<<1))' % (self.xr(i, s_), self.xr(i, s_)))]
        # ---- integer SIMD
        mm = re.match(r'^p(add|sub)(b|w|d|q)$', m)
        if mm:
            kind, ln = mm.groups()
            n, field = {'b': (16, 'b'), 'w': (8, 'w'), 'd': (4, 'd'), 'q': (2, 'q')}[ln]
            op = '+' if kind == 'add' else '-'
            return ['{Xmm s_=%s; for(int k_=0;k_<%d;++k_) %s.%s[k_]=%s.%s[k_]%ss_.%s[k_];}' % (src_xmm(s_), n, x, field, x, field, op, field)]
        if m == 'pcmpeqd':
            return ['{Xmm s_=%s; for(int k_=0;k_<4;++k_) %s.d[k_]=(%s.d[k_]==s_.d[k_])?~0u:0u;}' % (src_xmm(s_), x, x)]
        if m == 'pmaxsw':
            return ['{Xmm s_=%s; for(int k_=0;k_<8;++k_) if((int16_t)s_.w[k_]>(int16_t)%s.w[k_]) %s.w[k_]=s_.w[k_];}' % (src_xmm(s_), x, x)]
        mm = re.match(r'^ps(rl|ll|ra)(w|d|q|dq)$', m)
        if mm:
            kind, ln = mm.groups()
            cnt = '(uint32_t)%d' % (ops[1].imm & 0xff) if ops[1].type == X86_OP_IMM else '(uint32_t)(%s).q[0]' % src_xmm(s_)
            if ln == 'dq':
                return ['rt_pshift_bytes(&%s,%s,%d);' % (x, cnt, 1 if kind == 'rl' else 0)]
            n, field, bits, st = {'w': (8, 'w', 16, 'int16_t'), 'd': (4, 'd', 32, 'int32_t'), 'q': (2, 'q', 64, 'int64_t')}[ln]
            if kind == 'ra':
                return ['{uint32_t n_=%s; if(n_>%d) n_=%d; for(int k_=0;k_<%d;++k_) %s.%s[k_]=(%s)(%s.%s[k_])>>n_;}' % (cnt, bits - 1, bits - 1, n, x, field, st, x, field)]
            op = '>>' if kind == 'rl' else '<<'
            return ['{uint32_t n_=%s; for(int k_=0;k_<%d;++k_) %s.%s[k_]=n_>=%d?0:(%s.%s[k_]%sn_);}' % (cnt, n, x, field, bits, x, field, op)]
        # ---- conversions
        if m == 'cvtsi2sd':
            return ['%s.dd[0]=(double)(int32_t)%s;' % (x, self.rd(i, s_, 4))]
        if m == 'cvtsi2ss':
            return ['%s.f[0]=(float)(int32_t)%s;' % (x, self.rd(i, s_, 4))]
        if m in ('cvttsd2si', 'cvtsd2si'):
            v = '%s.dd[0]' % self.xr(i, s_) if isx(s_) else 'RF64(%s)' % A(s_)
            fn = 'rt_cvtt_i32(%s)' % v if m == 'cvttsd2si' else 'rt_cvt_i32(c,%s)' % v
            return [self.wr(i, d, fn)]
        if m in ('cvttss2si', 'cvtss2si'):
            v = '(double)%s.f[0]' % self.xr(i, s_) if isx(s_) else 'RF32(%s)' % A(s_)
            fn = 'rt_cvtt_i32(%s)' % v if m == 'cvttss2si' else 'rt_cvt_i32(c,%s)' % v
            return [self.wr(i, d, fn)]
        if m == 'cvtsd2ss':
            return ['{Xmm s_=%s; %s.f[0]=(float)s_.dd[0];}' % (src_xmm(s_, 8), x)]
        if m == 'cvtss2sd':
            return ['{Xmm s_=%s; %s.dd[0]=(double)s_.f[0];}' % (src_xmm(s_, 4), x)]
        if m == 'cvtpd2ps':
            return ['{Xmm s_=%s; %s.f[0]=(float)s_.dd[0]; %s.f[1]=(float)s_.dd[1]; %s.q[1]=0;}' % (src_xmm(s_), x, x, x)]
        if m == 'cvtps2pd':
            return ['{Xmm s_=%s; %s.dd[1]=(double)s_.f[1]; %s.dd[0]=(double)s_.f[0];}' % (src_xmm(s_, 8), x, x)]
        if m == 'cvtdq2pd':
            return ['{Xmm s_=%s; %s.dd[1]=(double)(int32_t)s_.d[1]; %s.dd[0]=(double)(int32_t)s_.d[0];}' % (src_xmm(s_, 8), x, x)]
        # ---- compares
        if m in ('comisd', 'ucomisd', 'comiss', 'ucomiss'):
            v = '%s.dd[0]' if m.endswith('sd') else '(double)%s.f[0]'
            a = v % x
            b = v % 's_'
            return ['{Xmm s_=%s; double a_=%s, b_=%s; if(isnan(a_)||isnan(b_)){f_zf=f_pf=f_cf=1;} else {f_zf=(a_==b_); f_pf=0; f_cf=(a_<b_);} f_of=f_sf=0;}' % (src_xmm(s_, 8 if m.endswith('sd') else 4), a, b)]
        mm = re.match(r'^cmp(eq|lt|le|unord|neq|nlt|nle|ord)(sd|pd)$', m)
        if mm:
            pred = ['eq', 'lt', 'le', 'unord', 'neq', 'nlt', 'nle', 'ord'].index(mm.group(1))
            lanes = [0] if mm.group(2) == 'sd' else [0, 1]
            return ['{Xmm s_=%s; %s}' % (src_xmm(s_, 8 if len(lanes) == 1 else 16), ' '.join(
                '%s.q[%d]=rt_cmp_mask(%s.dd[%d],s_.dd[%d],%d);' % (x, k, x, k, k, pred) for k in lanes))]
        if m == 'rcpss':
            return ['{Xmm s_=%s; %s.f[0]=1.0f/s_.f[0];}' % (src_xmm(s_, 4), x)]
        raise LiftError('sse')

    def xmm_mov(self, i):
        d, s = i.operands
        def val(op):
            if op.type == X86_OP_REG:
                return 'c->xmm[%d][0]' % int(self.rn(i, op.reg)[3:])
            return 'RF64(%s)' % self.addr(i, op)
        if d.type == X86_OP_REG:
            return 'c->xmm[%d][0]=%s;' % (int(self.rn(i, d.reg)[3:]), val(s))
        return 'WF64(%s,%s);' % (self.addr(i, d), val(s))

    # ---- x87
    def fmem(self, i, op, kind='f'):
        a = self.addr(i, op)
        if kind == 'f':
            return {4: 'RF32(%s)', 8: 'RF64(%s)', 10: 'RF80(%s)'}[op.size] % a
        return {2: '(double)(int16_t)R16(%s)', 4: '(double)(int32_t)R32(%s)',
                8: '(double)(int64_t)R64(%s)'}[op.size] % a

    def streg(self, i, op):
        n = self.rn(i, op.reg)
        m = re.match(r'st\((\d)\)', n) or re.match(r'st(\d)', n)
        if not m:
            raise LiftError('st ' + n)
        return int(m.group(1))

    def emit_x87(self, i, live):
        m = i.mnemonic
        ops = i.operands
        if m in ('fld', 'fild'):
            if ops[0].type == X86_OP_MEM:
                return ['FPUSH(%s);' % self.fmem(i, ops[0], 'f' if m == 'fld' else 'i')]
            return ['{double t_=ST(%d); FPUSH(t_);}' % self.streg(i, ops[0])]
        if m == 'fldz':
            return ['FPUSH(0.0);']
        if m == 'fldpi':
            return ['FPUSH(3.14159265358979323846);']
        if m == 'fldl2e':
            return ['FPUSH(1.4426950408889634074);']
        if m == 'fldln2':
            return ['FPUSH(0.69314718055994530942);']
        if m == 'fldl2t':
            return ['FPUSH(3.32192809488736234787);']
        if m == 'fldlg2':
            return ['FPUSH(0.30102999566398119521);']
        if m in ('fnclex', 'fwait', 'wait'):
            return ['c->c0=c->c0;'] if m != 'fnclex' else ['c->fsw_exc=0;']
        if m == 'fninit':
            return ['c->fcw=0x037f; c->top=0; c->c0=c->c1=c->c2=c->c3=0; c->fsw_exc=0;']
        if m == 'frndint':
            return ['ST(0)=rt_fround(c,ST(0));']
        if m == 'fsqrt':
            return ['ST(0)=sqrt(ST(0));']
        if m == 'fscale':
            return ['ST(0)=ldexp(ST(0),(int)trunc(ST(1)));']
        if m in ('fprem', 'fprem1'):
            fn = 'fmod' if m == 'fprem' else 'remainder'
            return ['{double q_=trunc(ST(0)/ST(1)); ST(0)=%s(ST(0),ST(1)); c->c2=0; c->c0=((int64_t)q_>>2)&1; c->c3=((int64_t)q_>>1)&1; c->c1=(int64_t)q_&1;}' % fn]
        if m == 'fpatan':
            return ['ST(1)=atan2(ST(1),ST(0)); FPOP();']
        if m == 'fptan':
            return ['ST(0)=tan(ST(0)); FPUSH(1.0); c->c2=0;']
        if m == 'f2xm1':
            return ['ST(0)=exp2(ST(0))-1.0;']
        if m == 'fyl2x':
            return ['ST(1)=ST(1)*log2(ST(0)); FPOP();']
        if m in ('fsin', 'fcos'):
            return ['ST(0)=%s(ST(0)); c->c2=0;' % m[1:]]
        if m == 'fsincos':
            return ['{double v_=ST(0); ST(0)=sin(v_); FPUSH(cos(v_)); c->c2=0;}']
        if m == 'ftst':
            return ['rt_fcom(c,ST(0),0.0);']
        if m == 'fxam':
            return ['rt_fxam(c);']
        if m in ('fnstenv', 'fstenv'):
            return ['rt_fnstenv(c,%s);' % self.addr(i, ops[0])]
        if m == 'fldenv':
            return ['rt_fldenv(c,%s);' % self.addr(i, ops[0])]
        if m in ('ffree',):
            return []
        if m == 'fdecstp':
            return ['c->top=(c->top-1u)&7u;']
        if m == 'fincstp':
            return ['c->top=(c->top+1u)&7u;']
        if m == 'fld1':
            return ['FPUSH(1.0);']
        if m in ('fst', 'fstp'):
            pop = ' FPOP();' if m == 'fstp' else ''
            if ops[0].type == X86_OP_MEM:
                fn = {4: 'WF32', 8: 'WF64', 10: 'WF80'}[ops[0].size]
                return ['%s(%s,ST(0));%s' % (fn, self.addr(i, ops[0]), pop)]
            return ['ST(%d)=ST(0);%s' % (self.streg(i, ops[0]), pop)]
        if m in ('fist', 'fistp'):
            pop = ' FPOP();' if m == 'fistp' else ''
            s = ops[0].size
            return ['W%d(%s,(uint%d_t)rt_fist%d(c,ST(0)));%s' % (s * 8, self.addr(i, ops[0]), s * 8, s * 8, pop)]
        if m == 'fchs':
            return ['ST(0)=-ST(0);']
        if m == 'fabs':
            return ['ST(0)=fabs(ST(0));']
        if m == 'fxch':
            # Capstone may list `fxch st(i)` as (st(0), st(i)): use the last operand.
            n = self.streg(i, ops[-1]) if ops else 1
            return ['{double t_=ST(0); ST(0)=ST(%d); ST(%d)=t_;}' % (n, n)]
        if m == 'fnstsw':
            v = 'rt_fsw(c)'
            if ops[0].type == X86_OP_REG:
                return ['eax=(eax&0xffff0000u)|%s;' % v]
            return ['W16(%s,%s);' % (self.addr(i, ops[0]), v)]
        if m == 'fnstcw':
            return ['W16(%s,c->fcw);' % self.addr(i, ops[0])]
        if m == 'fldcw':
            return ['c->fcw=R16(%s);' % self.addr(i, ops[0])]
        b0, b1 = i.bytes[0], (i.bytes[1] if len(i.bytes) > 1 else 0)
        if b0 in (0xd8, 0xdc, 0xde) and b1 >= 0xc0 and ((b1 >> 3) & 7) not in (2, 3):
            # Register forms, decoded from the opcode (Intel SDM vol. 2):
            # D8: ST0 = ST0 op ST(r); DC/DE: ST(r) = ST(r) op ST0 with the
            # sub/subr and div/divr encodings swapped; DE pops.
            opf, r = (b1 >> 3) & 7, b1 & 7
            if b0 == 0xd8:
                d, s_ = 'ST(0)', 'ST(%d)' % r
                expr = {0: '%s+%s', 1: '%s*%s', 4: '%s-%s', 5: '%s2-%s1', 6: '%s/%s', 7: '%s2/%s1'}[opf]
            else:
                d, s_ = 'ST(%d)' % r, 'ST(0)'
                expr = {0: '%s+%s', 1: '%s*%s', 4: '%s2-%s1', 5: '%s-%s', 6: '%s2/%s1', 7: '%s/%s'}[opf]
            if '2' in expr:
                code = expr.replace('%s2', '(s_)').replace('%s1', '(d_)')
            else:
                code = expr % ('(d_)', '(s_)')
            out = ['{double d_=%s, s_=%s; %s=%s;}' % (d, s_, d, code)]
            if b0 == 0xde:
                out.append('FPOP();')
            return out
        arith = {'fadd': '+', 'fsub': '-', 'fmul': '*', 'fdiv': '/', 'fsubr': 'r-', 'fdivr': 'r/'}
        base = m[:-1] if m.endswith('p') and m[:-1] in arith else m
        if base in arith:
            op = arith[base]
            pop = m.endswith('p')
            def comb(dst, src):
                if op.startswith('r'):
                    return '%s=(%s)%s(%s);' % (dst, src, op[1], dst)
                return '%s=(%s)%s(%s);' % (dst, dst, op, src)
            if len(ops) == 1 and ops[0].type == X86_OP_MEM:
                return [comb('ST(0)', self.fmem(i, ops[0]))]
            if len(ops) == 0:
                return [comb('ST(1)', 'ST(0)'), 'FPOP();']
            if len(ops) == 1:
                return [comb('ST(%d)' % self.streg(i, ops[0]), 'ST(0)')] + (['FPOP();'] if pop else [])
            d, s = self.streg(i, ops[0]), self.streg(i, ops[1])
            return ['{double s_=ST(%d); %s}' % (s, comb('ST(%d)' % d, 's_'))] + (['FPOP();'] if pop else [])
        iarith = {'fiadd': '+', 'fisub': '-', 'fimul': '*', 'fidiv': '/', 'fisubr': 'r-', 'fidivr': 'r/'}
        if m in iarith:
            op = iarith[m]
            v = self.fmem(i, ops[0], 'i')
            if op.startswith('r'):
                return ['ST(0)=(%s)%s(ST(0));' % (v, op[1])]
            return ['ST(0)=(ST(0))%s(%s);' % (op, v)]
        if m in ('fcomi', 'fcomip', 'fucomi', 'fucomip', 'fucompi', 'fcompi'):
            n = self.streg(i, ops[1]) if len(ops) > 1 else self.streg(i, ops[0])
            pop = 'p' in m[4:] or m.endswith('pi')
            return ['{double a_=ST(0), b_=ST(%d); if(isnan(a_)||isnan(b_)){f_zf=f_pf=f_cf=1;} else {f_zf=(a_==b_); f_pf=0; f_cf=(a_<b_);} f_of=f_sf=0;}' % n] + (['FPOP();'] if pop else [])
        if m.startswith('fcmov'):
            cc = {'b': 'f_cf', 'e': 'f_zf', 'be': '(f_cf|f_zf)', 'u': 'f_pf', 'nb': '!f_cf', 'ne': '!f_zf',
                  'nbe': '!(f_cf|f_zf)', 'nu': '!f_pf'}[m[5:]]
            n = self.streg(i, ops[1]) if len(ops) > 1 else self.streg(i, ops[0])
            return ['if(%s) ST(0)=ST(%d);' % (cc, n)]
        if m in ('fnsave', 'fsave'):
            return ['rt_fnsave(c,%s);' % self.addr(i, ops[0])]
        if m == 'frstor':
            return ['rt_frstor(c,%s);' % self.addr(i, ops[0])]
        if m in ('fcom', 'fcomp', 'fcompp', 'fucom', 'fucomp', 'fucompp'):
            if ops and ops[0].type == X86_OP_MEM:
                src = self.fmem(i, ops[0])
            elif ops:
                src = 'ST(%d)' % self.streg(i, ops[0])
            else:
                src = 'ST(1)'
            pops = 2 if m.endswith('pp') else (1 if m.endswith('p') else 0)
            return ['rt_fcom(c,ST(0),%s);' % src] + ['FPOP();'] * pops
        raise LiftError('x87')


BAD_MNEMONICS = {'arpl', 'bound', 'daa', 'das', 'aaa', 'aas', 'aam', 'aad', 'pushal', 'popal',
                 'pushaw', 'popaw', 'insb', 'insd', 'insw', 'outsb', 'outsd', 'outsw', 'in', 'out',
                 'int1', 'into', 'les', 'lds', 'hlt', 'iretd', 'retf', 'lcall', 'ljmp', 'salc',
                 'loopne', 'loope', 'cli', 'sti', 'wait'}


def plausible_code(L, va, limit=24):
    """Reject data that relocations point at inside .text."""
    for _ in range(limit):
        i = L.insn(va)
        if i is None or i.mnemonic in BAD_MNEMONICS or i.mnemonic.startswith('(bad'):
            return False
        string_op = re.search(r'(movs|stos|lods|scas|cmps)[bwd]?$', i.mnemonic)
        if not string_op and (any(r in i.op_str for r in (' es:', ' ds:', ' ss:', ' cs:')) or
                              re.search(r'\b(es|ds|ss|cs|gs)\b', i.op_str)):
            return False
        if i.mnemonic in ('ret', 'jmp'):
            return True
        va += i.size
    return True


class Image:
    """Several PE modules sharing one guest address space."""

    def __init__(self, modules):
        self.modules = modules
        self.iat = {}
        for m in modules:
            self.iat.update(m.iat)
        self.entry = modules[0].entry
        self.sections = [s for m in modules for s in m.sections]

    def mod(self, va):
        for m in self.modules:
            if m.section(va):
                return m
        return None

    def read(self, va, n):
        m = self.mod(va)
        return m.read(va, n) if m else None

    def u32(self, va):
        m = self.mod(va)
        return m.u32(va) if m else None

    def in_text(self, va):
        return any(m.in_text(va) for m in self.modules)


# Lifted DLLs and the guest base each is rebased to (above the guest heap; the
# whole layout fits in 0x59000000 bytes so consoles can allocate it outright). Order = DllMain order (dependencies first).
DLLS = [('msvcr90.dll', 0x58000000), ('msvcp90.dll', 0x58200000),
        ('SDL.dll', 0x58400000), ('SDL_gfx.dll', 0x58500000), ('SDL_image.dll', 0x58540000),
        ('libogg-0.dll', 0x58600000), ('libvorbis-0.dll', 0x58640000),
        ('libvorbisfile-3.dll', 0x58700000), ('SDL_mixer.dll', 0x58740000),
        # Shipped Xiph DLLs with a static CRT; something loads them at run time.
        ('ogg.dll', 0x58800000), ('vorbis.dll', 0x58820000), ('vorbisfile.dll', 0x58960000)]
# Lifted exports that are routed through a host wrapper (which logs and then
# calls the lifted code); used to debug the boundary. See imports_debug.cpp.
INTERPOSED = {('sdl_mixer.dll', 'Mix_LoadMUS_RW'), ('sdl_mixer.dll', 'Mix_PlayMusic'),
              ('sdl_mixer.dll', 'Mix_VolumeMusic')}
# DLLs that are not shipped but resolve to a lifted equivalent.
DLL_ALIASES = {'msvcrt.dll': 'msvcr90.dll'}

# Port changes to the game itself, kept minimal and listed here so they are easy
# to audit (docs/recompilation.md "Port changes"). Code bytes are only used by
# the lifter, so a patch changes the generated C, not the data the runtime maps.
#   PATCHES: MMU.exe VA -> (original bytes, new bytes)
#   HOOKS:   MMU.exe VA -> C function `void name(Cpu* c)` (recomp/src/patches.cpp)
#            called with synced registers right before that instruction.
PATCHES = {
    # Options menu, DISPLAY SIZE: allow value 6 (WIDE) after 1X-4X/FULL.
    0x4fc444: (b'\x6a\x05', b'\x6a\x06'),
}
# SKIP_HOOKS: function entry VA -> (C function `int name(Cpu* c)`, ret imm). A
# nonzero result returns from the function at once (`ret imm`, result in eax).
# DLL functions use their VAs as relocated by DLLS (SDL.dll at 0x58400000).
SKIP_HOOKS = {
    0x61c510: ('rt_hook_buster_press', 8),     # weapon press-fire: always charged buster shots
    0x5a9040: ('rt_hook_fast_knockback_push', 0),  # HurtKnockback update: straight to Hurt
    0x58414030: ('rt_hook_blit_pixel_alpha', 0),  # SDL.dll BlitNtoNPixelAlpha: native (performance)
}
HOOKS = {
    0x4fb244: 'rt_hook_display_label',  # label switch: 6 -> WIDE text in the FULL slot
    0x4fcf30: 'rt_hook_joymap',         # joystick mapping screen ran (- + ZR skip chord)
    0x4fc280: 'rt_hook_options_update', # OPTIONS update: no MAP KEYBOARD INPUTS on Switch
    0x4405c0: 'rt_hook_state_change',   # switch state by name (dev trace, input scripts)
    # Port menu (recomp/src/port_menu.cpp): cheats and live data.
    0x5b02f0: 'rt_hook_player',         # MegaMan message handler: remember the player
    0x5b0130: 'rt_hook_damage',         # MegaMan apply damage: infinite health
    0x420eb0: 'rt_hook_set_energy',     # set weapon energy: infinite weapon energy
    0x5a9ef3: 'rt_hook_jump_velocity',  # after GameParam(jump velocity)
    0x5af76c: 'rt_hook_jump_velocity',
    0x5af037: 'rt_hook_jump_cut',       # after GameParam(jump release cut)
    0x5af591: 'rt_hook_moon_jump',      # Fall state, after gravity + clamp, before the move
    0x5acbf0: 'rt_hook_fast_knockback', # Hurt update
    0x5a8df0: 'rt_hook_fast_knockback', # HurtSlide update
    0x61d280: 'rt_hook_buster_release', # weapon release-fire: no second shot with always-charged
    # Menus: A confirms, B cancels (after default bindings and options.xopts).
    0x4fc22c: 'rt_hook_menu_buttons',
    0x5dd1bc: 'rt_hook_menu_buttons',
    0x440f93: 'rt_hook_menu_buttons',
    0x4fd106: 'rt_hook_menu_buttons',   # after MAP CONTROLLER INPUTS rebinds a button
    0x4fd43f: 'rt_hook_menu_buttons',   # after MAP KEYBOARD INPUTS rebinds a key
}
# SDL exports that touch the OS (window, GL, input, timing, audio device,
# threads) are host shims (recomp/src/imports_sdl.cpp); every other export,
# including all surface/blit/RWops code, runs the original lifted SDL.dll.
SDL_SHIMMED = {
    'SDL_Init', 'SDL_InitSubSystem', 'SDL_QuitSubSystem', 'SDL_Quit', 'SDL_WasInit',
    'SDL_SetVideoMode', 'SDL_GetVideoSurface', 'SDL_GetVideoInfo', 'SDL_VideoDriverName',
    'SDL_ListModes', 'SDL_VideoModeOK', 'SDL_WM_SetCaption', 'SDL_WM_SetIcon',
    'SDL_WM_GrabInput', 'SDL_WM_IconifyWindow', 'SDL_WM_ToggleFullScreen', 'SDL_ShowCursor',
    'SDL_PollEvent', 'SDL_PumpEvents', 'SDL_WaitEvent', 'SDL_PeepEvents', 'SDL_PushEvent',
    'SDL_EventState', 'SDL_GetKeyState', 'SDL_GetModState', 'SDL_EnableUNICODE',
    'SDL_EnableKeyRepeat', 'SDL_GetMouseState', 'SDL_GetAppState',
    'SDL_GetTicks', 'SDL_Delay',
    'SDL_GL_SetAttribute', 'SDL_GL_GetAttribute', 'SDL_GL_SwapBuffers', 'SDL_GL_GetProcAddress',
    'SDL_GL_LoadLibrary', 'SDL_UpdateRect', 'SDL_UpdateRects', 'SDL_Flip',
    'SDL_NumJoysticks', 'SDL_JoystickOpen', 'SDL_JoystickClose', 'SDL_JoystickUpdate',
    'SDL_JoystickEventState', 'SDL_JoystickName', 'SDL_JoystickNumAxes',
    'SDL_JoystickNumButtons', 'SDL_JoystickNumHats', 'SDL_JoystickGetAxis',
    'SDL_JoystickGetButton', 'SDL_JoystickGetHat', 'SDL_JoystickOpened', 'SDL_JoystickIndex',
    'SDL_OpenAudio', 'SDL_CloseAudio', 'SDL_PauseAudio', 'SDL_LockAudio', 'SDL_UnlockAudio',
    'SDL_GetAudioStatus', 'SDL_AudioDriverName',
    'SDL_CreateThread', 'SDL_WaitThread', 'SDL_KillThread', 'SDL_ThreadID',
    'SDL_CreateMutex', 'SDL_DestroyMutex', 'SDL_mutexP', 'SDL_mutexV',
    'SDL_CreateSemaphore', 'SDL_DestroySemaphore', 'SDL_SemPost', 'SDL_SemWait',
    'SDL_SemTryWait', 'SDL_SemWaitTimeout', 'SDL_SemValue',
    'SDL_CreateCond', 'SDL_DestroyCond', 'SDL_CondSignal', 'SDL_CondBroadcast', 'SDL_CondWait',
    'SDL_CondWaitTimeout', 'SDL_AddTimer', 'SDL_RemoveTimer', 'SDL_SetTimer',
    'SDL_SetModuleHandle', 'SDL_RegisterApp', 'SDL_UnregisterApp', 'SDL_getenv', 'SDL_putenv',
    'SDL_CDNumDrives',
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('exe')
    ap.add_argument('out')
    ap.add_argument('--entries', default=os.path.join(ROOT, 'tools', 'recomp', 'extra_entries.txt'))
    ap.add_argument('--only', default='')
    ap.add_argument('--files', type=int, default=48)
    args = ap.parse_args()
    gamedir = os.path.dirname(os.path.abspath(args.exe))
    exe = PE(args.exe)
    for va, (old, new) in PATCHES.items():
        o = exe.file_off(va)
        if bytes(exe.data[o:o + len(old)]) != old:
            sys.exit('lift.py: patch at %#x does not match this MMU.exe' % va)
        exe.data[o:o + len(new)] = new
    dlls = [PE(os.path.join(gamedir, name), rebase=base) for name, base in DLLS]
    img = Image([exe] + dlls)
    by_name = {os.path.basename(m.path).lower(): m for m in dlls}

    L = Lifter(img)
    # Bind every IAT slot: lifted DLL export (code or data VA) or host shim.
    bound = {}      # iat va -> guest VA of the export
    shims = {}      # iat va -> (dll, sym, c name)
    def resolve(dll, sym, depth=0):
        dll = DLL_ALIASES.get(dll.lower(), dll)
        if dll.lower() == 'sdl.dll' and sym in SDL_SHIMMED:
            return None, ('SDL.dll', sym)
        if (dll.lower(), sym) in INTERPOSED:
            return None, (dll, sym)
        m = by_name.get(dll.lower())
        if m is None or depth > 4:
            return None, (dll, sym)
        e = m.exports.get(sym)
        if e is None:
            return None, (dll, sym)
        if e[0] == 'fwd':
            tdll, tsym = e[1].split('.', 1)
            return resolve(tdll + '.dll', tsym, depth + 1)
        return e[1], None
    for slot, (dll, sym) in img.iat.items():
        va, shim = resolve(dll, sym)
        if va is not None:
            bound[slot] = va
        else:
            shims[slot] = (shim[0], shim[1], import_c_name(*shim))

    seeds = set()
    for line in open(os.path.join(ROOT, 'docs', 'function_inventory.txt')):
        if line.startswith('0x'):
            seeds.add(int(line.split()[0], 16))
    if os.path.exists(args.entries):
        for line in open(args.entries):
            line = line.split('#')[0].strip()
            if line:
                seeds.add(int(line.split()[0], 16))
    seeds.add(exe.entry)
    # MMU.exe has no relocations: dwords in its data sections that point at
    # plausible code are callbacks/initialisers (_initterm tables, vtables).
    for name, va, vs, ra, rs in exe.sections:
        if name in ('.rdata', '.data'):
            for k in range(0, min(vs, rs) - 3, 4):
                v = exe.u32(va + k)
                if v is not None and exe.in_text(v) and v not in seeds and plausible_code(L, v):
                    seeds.add(v)
    for m in dlls:
        seeds.add(m.entry)
        for kind, va in m.exports.values():
            if kind == 'va' and m.in_text(va) and plausible_code(L, va):
                seeds.add(va)
        # Relocated dwords that point into .text: vtables, callbacks, tables.
        for r in m.relocs:
            v = m.u32(r)
            if v is not None and m.in_text(v) and plausible_code(L, v):
                seeds.add(v)
    seeds.update(v for v in bound.values() if img.in_text(v) and plausible_code(L, v))
    L.bound = bound
    L.shims = shims
    L.import_thunks = {}
    for s_ in list(seeds):
        i = L.insn(s_)
        if i is not None and i.mnemonic == 'jmp' and i.operands[0].type == X86_OP_MEM and \
                i.operands[0].mem.base == X86_REG_INVALID and \
                (i.operands[0].mem.disp & 0xffffffff) in img.iat:
            slot = i.operands[0].mem.disp & 0xffffffff
            if slot in shims:
                L.import_thunks[s_] = shims[slot][2]
    L.noreturn = set()
    L.entries = set(seeds)
    L.entries_data_guard = set()

    only = [int(x, 16) for x in args.only.split(',') if x]
    done = {}
    L.new_entries = set()
    queue = sorted(only or (seeds - set(L.import_thunks)), reverse=True)
    while queue:
        e = queue.pop()
        if e in done or e in L.import_thunks:
            continue
        L.entries.add(e)
        try:
            done[e] = L.lift_function(e)
        except LiftError as ex:
            done[e] = 'void f_%x(Cpu* c) { RT_FAULT(0x%xu, "lift failed: %s"); }' % (e, e, str(ex).replace('"', ''))
        if not only:
            for n in L.new_entries:
                if n not in done and n not in L.import_thunks:
                    L.entries.add(n)
                    queue.append(n)
        L.new_entries = set()

    emitted = set(re.findall(r'SYNC_OUT; (?:if \()?(rt_hook_\w+)\(c\)', '\n'.join(done.values())))
    missing = sorted((set(HOOKS.values()) | set(n for n, _ in SKIP_HOOKS.values())) - emitted)
    if missing:
        sys.exit('lift.py: hooks never emitted (address not an instruction?): %s' % ', '.join(missing))
    hooked = set(int(m, 16) for m in re.findall(r'/\*([0-9a-f]+): [^*]*\*/ SYNC_OUT; rt_hook_', '\n'.join(done.values())))
    for va in sorted(set(HOOKS) - hooked):
        print('lift.py: warning: hook at %#x (%s) not emitted' % (va, HOOKS[va]))
    os.makedirs(args.out, exist_ok=True)
    funcs = sorted(done)
    shim_names = sorted({v[2] for v in shims.values()})
    with open(os.path.join(args.out, 'recomp_decls.h'), 'w') as h:
        h.write('// Generated by tools/recomp/lift.py from MMU.exe. Do not commit.\n#pragma once\n')
        h.write('#include "recomp/cpu.h"\n')
        for f in funcs:
            h.write('void f_%x(Cpu* c);\n' % f)
        for imp in shim_names:
            h.write('void %s(Cpu* c);\n' % imp)
        for hook in sorted(set(HOOKS.values())):
            h.write('void %s(Cpu* c);\n' % hook)
        for name, _ in sorted(set(SKIP_HOOKS.values())):
            h.write('int %s(Cpu* c);\n' % name)
    per = max(1, (len(funcs) + args.files - 1) // args.files)
    nfiles = 0
    for k in range(0, len(funcs), per):
        nfiles += 1
        with open(os.path.join(args.out, 'recomp_%03d.c' % (k // per)), 'w') as f:
            f.write('// Generated by tools/recomp/lift.py from MMU.exe. Do not commit.\n')
            f.write('#include "recomp_decls.h"\n#include "recomp/cpu_inline.h"\n\n')
            for fn in funcs[k:k + per]:
                f.write(done[fn] + '\n\n')
    for k in range(nfiles, args.files):  # keep the CMake file list stable
        open(os.path.join(args.out, 'recomp_%03d.c' % k), 'w').write('// empty\n')
    with open(os.path.join(args.out, 'recomp_table.c'), 'w') as f:
        f.write('// Generated by tools/recomp/lift.py. Do not commit.\n#include "recomp_decls.h"\n')
        f.write('const RtFunc rt_functions[] = {\n')
        for fn in funcs:
            f.write('  {0x%xu, f_%x},\n' % (fn, fn))
        f.write('};\nconst unsigned rt_function_count = %d;\n' % len(funcs))
        f.write('const RtImport rt_imports[] = {\n')
        for va, (dll, sym, cname) in sorted(shims.items()):
            f.write('  {0x%xu, "%s", "%s", %s},\n' % (va, dll, sym.replace('"', ''), cname))
        f.write('};\nconst unsigned rt_import_count = %d;\n' % len(shims))
        f.write('const RtBinding rt_bindings[] = {\n')
        for va, tgt in sorted(bound.items()):
            f.write('  {0x%xu, 0x%xu},\n' % (va, tgt))
        f.write('};\nconst unsigned rt_binding_count = %d;\n' % len(bound))
        f.write('const RtModule rt_modules[] = {\n')
        for m in [exe] + dlls:
            orig = struct.unpack_from('<I', m.data, m.opt + 28)[0]
            f.write('  {"%s", 0x%xu, 0x%xu, 0x%xu, %d},\n' % (
                os.path.basename(m.path), orig, m.base, m.entry, 0 if m is exe else 1))
        f.write('};\nconst unsigned rt_module_count = %d;\n' % (1 + len(dlls)))
        f.write('const uint32_t rt_entry_point = 0x%xu;\n' % exe.entry)
    with open(os.path.join(args.out, 'recomp_import_stubs.c'), 'w') as f:
        f.write('// Generated by tools/recomp/lift.py. Do not commit.\n#include "recomp_decls.h"\n')
        seen = set()
        for va, (dll, sym, cname) in sorted(shims.items()):
            if cname in seen:
                continue
            seen.add(cname)
            f.write('RT_WEAK void %s(Cpu* c) { (void)c; rt_missing_import("%s", "%s"); }\n' % (
                cname, dll, sym.replace('"', '')))
    print('functions=%d shims=%d bound=%d files=%d' % (len(funcs), len(shim_names), len(bound), nfiles))
    if L.unsupported:
        print('unsupported:', dict(L.unsupported.most_common(40)))


if __name__ == '__main__':
    main()
