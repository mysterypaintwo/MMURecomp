#!/usr/bin/env python3
"""Summarize enemy state callbacks straight from the PE with capstone.

For each function address, print referenced strings, setState() calls, field
accesses, and direct calls. This is a lightweight stand-in for a Ghidra
decompiler when reconstructing small state callbacks.

Usage: python3 tools/state_callback_report.py MMU.exe 0x6149d0 [0x... ...]
"""

import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

from analyze_rtti import PE

SET_STATE = 0x4405C0
GET_STATE = 0x401580
SET_ANIM = 0x43D340
ABS = 0x585FF0
DT = 0x43E510
NAMES = {SET_STATE: "setState", GET_STATE: "getState", SET_ANIM: "setAnim",
         ABS: "abs", DT: "dt"}


def cstring(pe, va, limit=80):
    off = pe.rva_to_off(va - pe.image_base)
    if off is None:
        return None
    raw = pe.data[off:off + limit].split(b"\0", 1)[0]
    if not raw:
        return None
    text = raw.decode("latin1")
    return text if all(c.isprintable() for c in text) else None


def report(pe, va):
    off = pe.rva_to_off(va - pe.image_base)
    if off is None:
        print(f"### 0x{va:x}: not in image")
        return
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    print(f"\n### 0x{va:x}")
    fields = set()
    strings = []
    calls = []
    state_calls = []
    pushed = None
    for ins in md.disasm(pe.data[off:off + 1536], va):
        if ins.mnemonic == "int3":
            break
        for op in ins.operands:
            if op.type == X86_OP_IMM:
                imm = op.imm & 0xFFFFFFFF
                text = cstring(pe, imm)
                if text:
                    strings.append((ins.address, imm, text))
                    pushed = text
                if ins.mnemonic == "call":
                    calls.append((ins.address, imm))
            elif op.type == X86_OP_MEM:
                if op.mem.base is not None or op.mem.index is not None:
                    fields.add(op.mem.disp & 0xFFFFFFFF)
                elif pe.in_text(op.mem.disp & 0xFFFFFFFF):
                    calls.append((ins.address, op.mem.disp & 0xFFFFFFFF))
        if ins.mnemonic in ("ret", "retf"):
            break
        if ins.mnemonic == "call":
            target = None
            for op in ins.operands:
                if op.type == X86_OP_IMM:
                    target = op.imm & 0xFFFFFFFF
                elif op.type == X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0:
                    target = op.mem.disp & 0xFFFFFFFF
            if target in (SET_STATE, GET_STATE) and pushed:
                state_calls.append((ins.address, NAMES[target], pushed))
                pushed = None
    print("  strings:", ", ".join(f"0x{a:x}={s!r}" for _, a, s in strings) or "-")
    print("  state:  ", ", ".join(f"0x{a:x}->{n}({s!r})" for a, n, s in state_calls) or "-")
    print("  calls:  ", ", ".join(
        f"0x{a:x}->0x{t:x}{'(' + NAMES[t] + ')' if t in NAMES else ''}"
        for a, t in calls) or "-")
    print("  fields: ", " ".join(f"+0x{f:x}" for f in sorted(fields) if f < 0x1000) or "-")


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    pe = PE(open(sys.argv[1], "rb").read())
    for token in sys.argv[2:]:
        report(pe, int(token, 0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
