#!/usr/bin/env python3
"""Quick PE32 function inspector using capstone (no external binary needed).

Usage:
  python3 tools/disasm.py MMU.exe 0x5b02f0 [max-instructions]
  python3 tools/disasm.py MMU.exe --class MegaMan

With --class, prints a compact disassembly of every virtual function in that
class's vftable, which is handy for reconstructing state machines.
"""

import struct
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

from analyze_rtti import PE, find_all


def find_vftables(pe):
    """Return {class_name: (vftable_rva, [function VAs])}."""
    descriptors = {}
    for off in find_all(pe.data, b'.?AV'):
        end = pe.data.find(b'@@', off)
        if end < 0:
            continue
        name = pe.data[off:end + 2].decode('latin1')
        if 'google' in name or 'protobuf' in name:
            continue
        rva = pe.off_to_rva(off)
        if rva and rva >= 8:
            descriptors[pe.image_base + rva - 8] = name

    result = {}
    for td_va, name in descriptors.items():
        ptr = struct.pack('<I', td_va)
        for off in find_all(pe.data, ptr):
            rva = pe.off_to_rva(off)
            if rva is None or rva < 12:
                continue
            col_rva = rva - 12
            if (pe.u32(col_rva) not in (0, 1) or not pe.in_rdata(col_rva) or
                    not pe.in_rdata(pe.u32(col_rva + 16) - pe.image_base)):
                continue
            col_va = pe.image_base + col_rva
            for poff in find_all(pe.data, struct.pack('<I', col_va)):
                vrva = pe.off_to_rva(poff)
                if vrva is None:
                    continue
                vft = vrva + 4
                funcs = []
                for i in range(64):
                    va = pe.u32(vft + 4 * i)
                    if va is None or not pe.in_text(va):
                        break
                    funcs.append(va)
                if funcs:
                    result.setdefault(name, []).append((vft, funcs))
    return result


def disasm(pe, va, limit=80):
    off = pe.rva_to_off(va - pe.image_base)
    if off is None:
        return
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    code = pe.data[off:off + 2048]
    depth = 0
    shown = 0
    for ins in md.disasm(code, va):
        if shown >= limit:
            break
        prefix = '  ' * min(depth, 4)
        extra = ''
        for op in ins.operands:
            if op.type == X86_OP_IMM:
                target = op.imm
                if pe.in_text(target):
                    extra += f' -> 0x{target:x}'
            elif op.type == X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0:
                target = op.mem.disp & 0xffffffff
                if pe.in_text(target):
                    extra += f' -> 0x{target:x}'
        print(f'{prefix}0x{ins.address:x}: {ins.mnemonic:7s} {ins.op_str}{extra}')
        shown += 1
        if ins.mnemonic in ('call',):
            depth += 1
        elif ins.mnemonic == 'ret':
            if depth == 0:
                break
            depth -= 1


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    pe = PE(open(sys.argv[1], 'rb').read())
    if sys.argv[2] == '--class':
        needle = sys.argv[3].lower()
        tables = find_vftables(pe)
        for name, entries in tables.items():
            if needle not in name.lower():
                continue
            for vft, funcs in entries:
                print(f'\n=== {name}  vftable 0x{vft:x} ({len(funcs)} slots) ===')
                for i, va in enumerate(funcs):
                    print(f'\n--- slot {i}: 0x{va:x} ---')
                    disasm(pe, va, 30)
    else:
        va = int(sys.argv[2], 0)
        limit = int(sys.argv[3], 0) if len(sys.argv) > 3 else 80
        disasm(pe, va, limit)
    return 0


if __name__ == '__main__':
    sys.path.insert(0, __file__.rsplit('/', 1)[0])
    sys.exit(main())
