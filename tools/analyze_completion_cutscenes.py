#!/usr/bin/env python3
"""Recover the cutscene prelude of FUN_00511350 from MMU.exe.

FUN_00511350 checks a sequence of raw flag ids through FUN_00441d90 before it
dispatches the manager's loader id. A true result starts a hard-coded cutscene
and returns; the flag ids and paths are literal binary evidence. It also has
two manager-state special cases (0x100 and 0x200) that choose a cutscene before
the normal loader-id routing. It also records the raw calls made by the 0x100
branch: FUN_004308c0 stores the chosen global string in the manager's pending
slot, and FUN_004308a0 clears two fields on the object at global 0x6c55f8.

The global std::string objects used by those branches are reconstructed from
their static initializers (`push <literal>; mov ecx,<global>`), so no string is
guessed from its filename or from label text.

Outputs:
  docs/completion_cutscenes.txt
  core/include/mmu/core/CompletionCutsceneTable.h
"""

import os
import re
import struct
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

from analyze_rtti import PE

FUNCTION_VA = 0x511350
FLAG_CHECK_VA = 0x441D90
CLEAR_CALL_VA = 0x4308A0
CLEAR_GLOBAL_VA = 0x6C55F8
PENDING_CALL_VA = 0x4308C0
PENDING_STRING_OFFSET = 0x40
PENDING_FLAG_OFFSET = 0x60
PENDING_FLAG_VALUE = 1
# (offset, access size, literal value) verified in FUN_004308a0.
CLEAR_WRITES = ((0x178, 1, 0), (0x17C, 4, 0))

# Manager-state special cases are keyed by their raw *manager value. The 0x100
# case selects between two global strings with FUN_00403d40 (manager byte
# +0x59); the 0x200 case uses a literal; 0x40 calls FUN_00510c80 without a
# cutscene. The last field lists the manager dword writes verified in the
# branch disassembly: (byte offset, literal value).
SPECIAL_CASES = [
    (0x100, 1, 0x6C6964, ((0x08, 0x100),)),
    (0x100, 2, 0x6C6A98, ((0x08, 0x100),)),
    (0x200, 0, 0x660938, ((0x08, 0x004), (0x30, 0x001))),
    (0x040, 0, 0, ()),
]

CSTR_RE = re.compile(rb"[ -~]+\.xcut\x00")


def read_c_string(pe, va):
    off = pe.rva_to_off(va - pe.image_base)
    if off is None:
        return None
    end = pe.data.find(b"\x00", off, off + 512)
    if end < 0:
        return None
    value = pe.data[off:end]
    if not CSTR_RE.fullmatch(value + b"\x00"):
        return None
    return value.decode("latin1")


def discover_global_strings(pe):
    """Map global std::string object VAs to their initializer literals."""
    result = {}
    text = next(s for s in pe.sections if s[0] == ".text")
    _name, _rva, _vs, off, size = text
    begin = off
    end = off + size
    for pos in range(begin, end - 10):
        if pe.data[pos] != 0x68 or pe.data[pos + 5] != 0xB9:
            continue
        literal = struct.unpack_from("<I", pe.data, pos + 1)[0]
        global_va = struct.unpack_from("<I", pe.data, pos + 6)[0]
        value = read_c_string(pe, literal)
        if value is None or pe.rva_to_off(global_va - pe.image_base) is None:
            continue
        if global_va in result and result[global_va][0] != literal:
            raise RuntimeError("conflicting initializer for 0x%x" % global_va)
        result[global_va] = (literal, value)
    return result


def disasm_function(pe, va, count=900):
    off = pe.rva_to_off(va - pe.image_base)
    if off is None:
        raise RuntimeError("function 0x%x is not mapped" % va)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    return list(md.disasm(pe.data[off:off + 4096], va))[:count]


def immediate_of(ins):
    for op in ins.operands:
        if op.type == X86_OP_IMM:
            return op.imm
    return None


def resolve_path_va(pe, va, globals_):
    direct = read_c_string(pe, va)
    if direct is not None:
        return va, direct
    if va in globals_:
        literal, value = globals_[va]
        return literal, value
    return None


def collect_flag_rows(pe, insns, globals_):
    rows = []
    for i, ins in enumerate(insns):
        if ins.mnemonic != "call" or immediate_of(ins) != FLAG_CHECK_VA:
            continue
        flag_id = None
        for back in range(i - 1, max(-1, i - 8), -1):
            prev = insns[back]
            if prev.mnemonic == "mov" and prev.op_str.startswith("eax, "):
                value = immediate_of(prev)
                if value is not None:
                    flag_id = value
                    break
        if flag_id is None:
            raise RuntimeError("no flag id before call at 0x%x" % ins.address)

        found = None
        for forward in insns[i + 1:i + 60]:
            if forward.mnemonic == "call" and immediate_of(forward) == FLAG_CHECK_VA:
                break
            if forward.mnemonic != "push":
                continue
            va = immediate_of(forward)
            if va is None:
                continue
            resolved = resolve_path_va(pe, va, globals_)
            if resolved is not None:
                found = resolved
                break
        if found is None:
            # Two later checks (0x25/0x26) select menus rather than cutscenes.
            continue
        literal_va, path = found
        rows.append({
            "flagId": flag_id,
            "managerState": 0,
            "selector": 0,
            "stringVa": literal_va,
            "pendingStringVa": 0,
            "path": path,
            "sourceVa": ins.address,
            "writes": (),
        })
    return rows


def verify_manager_writes(insns, start, expected):
    """Verify literal dword writes to the manager object in a branch."""
    found = set()
    for ins in insns[start:start + 50]:
        if ins.mnemonic != "mov" or len(ins.operands) != 2:
            continue
        dst, src = ins.operands
        if dst.type != X86_OP_MEM or src.type != X86_OP_IMM:
            continue
        if dst.mem.base == 0 or dst.mem.index != 0:
            continue
        found.add((dst.mem.disp & 0xffffffff, src.imm))
    for write in expected:
        if write not in found:
            raise RuntimeError("manager write %s not found after 0x%x" %
                               (str(write), insns[start].address))
    return expected


def collect_special_rows(pe, insns, globals_, pending_calls):
    rows = []
    texts = [(ins.address, ins.mnemonic, ins.op_str) for ins in insns]
    for manager_state, selector, string_va, writes in SPECIAL_CASES:
        state_pattern = "0x%x" % manager_state
        cmp_indexes = [i for i, (_addr, mnemonic, ops) in enumerate(texts)
                       if mnemonic == "cmp" and state_pattern in ops]
        if not cmp_indexes:
            raise RuntimeError("no manager-state compare for 0x%x" % manager_state)
        # The first compare is the branch that owns the cutscene selection.
        start = cmp_indexes[0]
        pushed = set()
        for _addr, mnemonic, ops in texts[start:start + 50]:
            if mnemonic == "push":
                value = immediate_of_asm(ops)
                if value is not None:
                    pushed.add(value)
        if string_va == 0:
            literal_va, path = 0, ""
        else:
            resolved = resolve_path_va(pe, string_va, globals_)
            if resolved is None:
                raise RuntimeError("special string 0x%x is not an .xcut" %
                                   string_va)
            if string_va not in pushed:
                raise RuntimeError("special string 0x%x not used by state 0x%x" %
                                   (string_va, manager_state))
            if manager_state == 0x100 and string_va not in pending_calls:
                raise RuntimeError("string 0x%x has no pending-helper call" %
                                   string_va)
            literal_va, path = resolved
        verified_writes = verify_manager_writes(insns, start, writes)
        rows.append({
            "flagId": 0,
            "managerState": manager_state,
            "selector": selector,
            "stringVa": literal_va,
            "pendingStringVa": string_va if manager_state == 0x100 else 0,
            "path": path,
            "sourceVa": texts[start][0],
            "writes": verified_writes,
        })
    return rows


def immediate_of_asm(op_str):
    match = re.fullmatch(r"(?:0x)?[0-9a-fA-F]+", op_str.strip())
    if match is None:
        return None
    return int(op_str.strip(), 0)


def call_sites(insns, va):
    return [i for i, ins in enumerate(insns)
            if ins.mnemonic == "call" and immediate_of(ins) == va]


def verify_helper_evidence(pe, insns):
    """Verify the raw clear/pending helpers instead of naming their semantics."""
    clear_sites = call_sites(insns, CLEAR_CALL_VA)
    if len(clear_sites) != 1:
        raise RuntimeError("expected one clear-helper call, got %d" %
                           len(clear_sites))
    prev = insns[clear_sites[0] - 1]
    if prev.mnemonic != "mov" or \
            "dword ptr [0x%x]" % CLEAR_GLOBAL_VA not in prev.op_str:
        raise RuntimeError("clear helper does not use global 0x%x: %s %s" %
                           (CLEAR_GLOBAL_VA, prev.mnemonic, prev.op_str))

    clear_ops = [(ins.mnemonic, ins.op_str)
                 for ins in disasm_function(pe, CLEAR_CALL_VA, 32)]
    if not any(m == "mov" and "byte ptr [eax + 0x178], 0" == ops
               for m, ops in clear_ops):
        raise RuntimeError("clear helper byte write at +0x178 not found")
    if not any(m == "fstp" and "dword ptr [ecx + 0x17c]" == ops
               for m, ops in clear_ops):
        raise RuntimeError("clear helper dword write at +0x17c not found")

    pending_ops = [(ins.mnemonic, ins.op_str)
                   for ins in disasm_function(pe, PENDING_CALL_VA, 32)]
    if not any(m == "add" and ops == "ecx, 0x40" for m, ops in pending_ops):
        raise RuntimeError("pending helper string offset +0x40 not found")
    if not any(m == "mov" and ops == "byte ptr [ecx + 0x60], dl"
               for m, ops in pending_ops):
        raise RuntimeError("pending helper flag write +0x60 not found")
    return insns[clear_sites[0]].address


def pending_call_strings(insns):
    """Map each global string pushed to FUN_004308c0 to its call site."""
    result = {}
    for index in call_sites(insns, PENDING_CALL_VA):
        string_va = None
        for prev in reversed(insns[max(0, index - 6):index]):
            if prev.mnemonic != "push":
                continue
            value = immediate_of(prev)
            if value in (0x6C6964, 0x6C6A98):
                string_va = value
                break
        if string_va is None:
            raise RuntimeError("no global string before pending call at 0x%x" %
                               insns[index].address)
        result[string_va] = insns[index].address
    return result


def write_report(rows, path):
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_completion_cutscenes.py\n")
        out.write("Source: FUN_00511350 flag checks (0x441d90) and manager-state\n")
        out.write("branches, plus the static .xcut string initializers in MMU.exe.\n")
        out.write("entries=%d\n" % len(rows))
        out.write("pending helper=0x%06x string offset=+0x%02x flag offset=+0x%02x=0x%x\n" % (
            PENDING_CALL_VA, PENDING_STRING_OFFSET, PENDING_FLAG_OFFSET,
            PENDING_FLAG_VALUE))
        clear_writes = ",".join("+0x%x:%u=0x%x" % (offset, size, value)
                                for offset, size, value in CLEAR_WRITES)
        out.write("clear helper=0x%06x global=[0x%06x] writes=%s\n\n" % (
            CLEAR_CALL_VA, CLEAR_GLOBAL_VA, clear_writes))
        for row in rows:
            if row["flagId"]:
                key = "flag=0x%02x" % row["flagId"]
            else:
                key = "manager=0x%x selector=%d" % (row["managerState"],
                                                   row["selector"])
            writes = ",".join("+0x%02x=0x%x" % (off, value)
                              for off, value in row["writes"])
            pending = ""
            if row["pendingStringVa"]:
                pending = " pending=0x%06x" % row["pendingStringVa"]
            out.write("%-28s branch=0x%06x string=0x%06x path=%s writes=%s%s\n" % (
                key, row["sourceVa"], row["stringVa"], row["path"] or "-",
                writes or "-", pending))


def write_header(rows, path):
    with open(path, "w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write("#include \"mmu/core/Types.h\"\n\n")
        out.write("#include <cstddef>\n")
        out.write("#include <string_view>\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_completion_cutscenes.py.\n")
        out.write("// rawFlagId is the argument to FUN_00441d90, whose save-data source is\n")
        out.write("// not recovered. managerState is the raw manager value tested before\n")
        out.write("// the normal loader-id routing. selector 0 is unconditional, 1 means\n")
        out.write("// manager byte +0x59 is false, 2 means it is true. writeCount and\n")
        out.write("// writeOffsets/writeValues record the literal manager dword writes\n")
        out.write("// verified in the branch disassembly (byte offset, value).\n")
        out.write("// For manager state 0x100, FUN_004308c0 also copies the chosen global\n")
        out.write("// string into manager +0x40 and sets byte +0x60 to 1; pendingStringVa\n")
        out.write("// identifies that global string. FUN_004308a0 clears two fields on the\n")
        out.write("// object at global 0x6c55f8 (offset, access size, literal value).\n")
        out.write("inline constexpr u32 kCompletionPendingCallVa = 0x%06xu;\n" %
                  PENDING_CALL_VA)
        out.write("inline constexpr u8 kCompletionPendingStringOffset = 0x%02xu;\n" %
                  PENDING_STRING_OFFSET)
        out.write("inline constexpr u8 kCompletionPendingFlagOffset = 0x%02xu;\n" %
                  PENDING_FLAG_OFFSET)
        out.write("inline constexpr u8 kCompletionPendingFlagValue = 0x%02xu;\n" %
                  PENDING_FLAG_VALUE)
        out.write("inline constexpr u32 kCompletionClearCallVa = 0x%06xu;\n" %
                  CLEAR_CALL_VA)
        out.write("inline constexpr u32 kCompletionClearGlobalVa = 0x%06xu;\n" %
                  CLEAR_GLOBAL_VA)
        out.write("struct CompletionClearWrite { u16 offset; u8 size; u32 value; };\n")
        out.write("inline constexpr CompletionClearWrite kCompletionClearWrites[] = {\n")
        for offset, size, value in CLEAR_WRITES:
            out.write("    {0x%03xu, %uu, 0x%04xu},\n" % (offset, size, value))
        out.write("};\n")
        out.write("inline constexpr std::size_t kCompletionClearWriteCount =\n")
        out.write("    sizeof(kCompletionClearWrites) / sizeof(kCompletionClearWrites[0]);\n\n")
        out.write("struct RecoveredCompletionCutscene {\n")
        out.write("    u32 rawFlagId;\n")
        out.write("    u32 managerState;\n")
        out.write("    u8 selector;\n")
        out.write("    u32 stringVa;\n")
        out.write("    u32 pendingStringVa;\n")
        out.write("    std::string_view path;\n")
        out.write("    u8 writeCount;\n")
        out.write("    u8 writeOffsets[2];\n")
        out.write("    u32 writeValues[2];\n")
        out.write("};\n\n")
        out.write("inline constexpr RecoveredCompletionCutscene kRecoveredCompletionCutscenes[] = {\n")
        for row in rows:
            escaped = row["path"].replace("\\", "\\\\").replace('"', '\\"')
            writes = list(row["writes"])
            offsets = [w[0] for w in writes] + [0] * (2 - len(writes))
            values = [w[1] for w in writes] + [0] * (2 - len(writes))
            out.write(
                "    {%du, 0x%03xu, %uu, 0x%06xu, 0x%06xu, \"%s\", %uu, "
                "{0x%02xu, 0x%02xu}, {0x%04xu, 0x%04xu}},\n" % (
                    row["flagId"], row["managerState"], row["selector"],
                    row["stringVa"], row["pendingStringVa"], escaped,
                    len(writes),
                    offsets[0], offsets[1], values[0], values[1]))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kRecoveredCompletionCutsceneCount =\n")
        out.write("    sizeof(kRecoveredCompletionCutscenes) /\n")
        out.write("    sizeof(kRecoveredCompletionCutscenes[0]);\n\n")
        out.write("inline const RecoveredCompletionCutscene* findCompletionCutsceneFlag(u32 flagId) {\n")
        out.write("    for (const RecoveredCompletionCutscene& row : kRecoveredCompletionCutscenes)\n")
        out.write("        if (row.rawFlagId == flagId && row.managerState == 0)\n")
        out.write("            return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline const RecoveredCompletionCutscene* findCompletionCutsceneState(\n")
        out.write("    u32 managerState, bool flag59) {\n")
        out.write("    const u8 selector = flag59 ? 2u : 1u;\n")
        out.write("    for (const RecoveredCompletionCutscene& row : kRecoveredCompletionCutscenes) {\n")
        out.write("        if (row.managerState != managerState) continue;\n")
        out.write("        if (row.selector == 0 || row.selector == selector) return &row;\n")
        out.write("    }\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("} // namespace mmu\n")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    report = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        root, "docs", "completion_cutscenes.txt")
    header = sys.argv[3] if len(sys.argv) > 3 else os.path.join(
        root, "core", "include", "mmu", "core", "CompletionCutsceneTable.h")
    pe = PE(open(sys.argv[1], "rb").read())
    globals_ = discover_global_strings(pe)
    insns = disasm_function(pe, FUNCTION_VA)
    verify_helper_evidence(pe, insns)
    pending_calls = pending_call_strings(insns)
    rows = collect_flag_rows(pe, insns, globals_)
    rows.extend(collect_special_rows(pe, insns, globals_, pending_calls))
    expected = {7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE}
    if {row["flagId"] for row in rows if row["flagId"]} != expected:
        raise RuntimeError("unexpected completion flag id set: %s" %
                           sorted(row["flagId"] for row in rows if row["flagId"]))
    write_report(rows, report)
    write_header(rows, header)
    print("entries=%d report=%s header=%s" % (len(rows), report, header))
    return 0


if __name__ == "__main__":
    sys.exit(main())
