#!/usr/bin/env python3
"""Recover the MenuStageSelect 3x3 grid from raw MMU.exe evidence.

The value stored at this+0xbb0 by FUN_004ffc80 is the stage number consumed by
FUN_004ff050. The eight outer cells are a flat if-chain over this+0xba0 /
this+0xba4; the center cell additionally reads the raw byte this+0xba8 and the
item count of the object at DAT_006c55a4. The +0xba8 toggle predicate lives in
FUN_004ff460 and is read back from the binary with capstone.

No gameplay label is assigned to any raw field, offset, or constant.

Outputs:
  docs/stage_select_grid.txt
  core/include/mmu/core/StageSelectGrid.h
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from analyze_rtti import PE

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

DECOMPILED = 0x4FFC80
TOGGLE_FUNC = 0x4FF460
CURSOR_FUNC = 0x4FF070
SUBITEM_OBJECT = 0x6C55A4
TEST_FLAGS_CALL = 0x404B00

OUTER_RE = re.compile(
    r"\(\*\(int \*\)\(param_1 \+ 0xba0\) == (\d+)\) && "
    r"\(\*\(int \*\)\(param_1 \+ 0xba4\) == (\d+)\)\) \{\s*"
    r"\*\(undefined4 \*\)\(param_1 \+ 0xbb0\) = (0x[0-9a-f]+|\d+);",
    re.DOTALL,
)
CENTER_COUNT_RE = re.compile(
    r"iVar3 == (\d+)\) \{\s*"
    r"\*\(undefined4 \*\)\(param_1 \+ 0xbb0\) = (0x[0-9a-f]+|\d+);",
    re.DOTALL,
)
CENTER_FLAG_RE = re.compile(
    r"\*\(char \*\)\(param_1 \+ 0xba8\) == '\\0'\)\s*\{(.*?)\n      \}\s*else\s*\{\s*"
    r"\*\(undefined4 \*\)\(param_1 \+ 0xbb0\) = (0x[0-9a-f]+|\d+);",
    re.DOTALL,
)


def read_text(root, va):
    path = os.path.join(root, "docs", "decompiled", "0x%06x.c" % va)
    with open(path, encoding="utf-8", errors="replace") as handle:
        return handle.read()


def parse_grid(text):
    outer = [(int(x), int(y), int(stage, 0))
             for x, y, stage in OUTER_RE.findall(text)]
    if len(outer) != 8:
        raise RuntimeError("expected 8 outer cells, got %d" % len(outer))
    if len({(x, y) for x, y, _ in outer}) != 8:
        raise RuntimeError("duplicate outer cell")
    if (1, 1) in {(x, y) for x, y, _ in outer}:
        raise RuntimeError("center cell present in outer chain")

    match = CENTER_FLAG_RE.search(text)
    if match is None:
        raise RuntimeError("center flag branch not found")
    flag_false_stage = int(match.group(2), 0)
    counts = [(int(count), int(stage, 0))
              for count, stage in CENTER_COUNT_RE.findall(match.group(1))]
    if [count for count, _ in counts] != [1, 2, 3, 4, 5]:
        raise RuntimeError("unexpected center sub-item counts: %r" % counts)

    outer.sort(key=lambda row: (row[1], row[0]))
    return outer, dict(counts), flag_false_stage


def read_toggle_facts(pe):
    off = pe.rva_to_off(TOGGLE_FUNC - pe.image_base)
    if off is None:
        raise RuntimeError("toggle function outside image")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    facts = []
    for ins in md.disasm(pe.data[off:off + 2048], TOGGLE_FUNC):
        text = "%s %s" % (ins.mnemonic, ins.op_str)
        if ("0xba8" in text or "0xbac" in text or
                (ins.mnemonic == "push" and ins.op_str == "0xf") or
                (ins.mnemonic == "call" and
                 ins.op_str.strip().endswith("0x%x" % TEST_FLAGS_CALL)) or
                "0x6989d0" in text or ins.mnemonic == "fld1" or
                (ins.mnemonic == "call" and "0x4026b0" in text)):
            facts.append((ins.address, ins.mnemonic, ins.op_str.strip()))
        if ins.mnemonic == "ret":
            break
    if not any("0xba8" in fact[2] for fact in facts):
        raise RuntimeError("no +0xba8 access in toggle function")
    if not any(fact[1] == "push" and fact[2] == "0xf" for fact in facts):
        raise RuntimeError("no raw 0xf predicate in toggle function")
    return facts


def read_cursor_key_facts(pe):
    """Map each raw key passed to FUN_00441d90 to a cursor field and delta."""
    off = pe.rva_to_off(CURSOR_FUNC - pe.image_base)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    rows = list(md.disasm(pe.data[off:off + 512], CURSOR_FUNC))
    key_indices = [
        index for index, ins in enumerate(rows)
        if ins.mnemonic == "mov" and
        re.fullmatch(r"eax, 0x[0-9a-f]+", ins.op_str)
    ]
    facts = []
    for position, index in enumerate(key_indices):
        key = int(rows[index].op_str.split(", ")[1], 16)
        stop = key_indices[position + 1] if position + 1 < len(key_indices) \
            else index + 24
        window = rows[index:stop]
        if not any(other.mnemonic == "call" and "0x441d90" in other.op_str
                   for other in window):
            continue
        cmp_match = None
        for other in window:
            match = re.fullmatch(
                r"dword ptr \[\w+ \+ 0x(ba0|ba4)\],\s*(0x[0-9a-f]+|\d+)",
                other.op_str)
            if other.mnemonic == "cmp" and match:
                cmp_match = match
                break
        if cmp_match is None:
            continue
        field = int(cmp_match.group(1), 16)
        bound = int(cmp_match.group(2), 0)
        loaded_reg = delta = None
        for other in window:
            move = re.fullmatch(
                r"(\w+), dword ptr \[\w+ \+ 0x(ba0|ba4)\]", other.op_str)
            if other.mnemonic == "mov" and move and \
                    int(move.group(2), 16) == field:
                loaded_reg = move.group(1)
                continue
            step = re.fullmatch(r"(\w+), 1", other.op_str)
            if other.mnemonic in ("add", "sub") and step and \
                    step.group(1) == loaded_reg:
                delta = 1 if other.mnemonic == "add" else -1
                break
        if delta is None:
            continue
        facts.append((key, field, bound, delta))
    expected = [(0x1c, 0xba4, 2, 1), (0x1b, 0xba4, 0, -1),
                (0x19, 0xba0, 2, 1), (0x1a, 0xba0, 0, -1)]
    if facts != expected:
        raise RuntimeError("unexpected cursor key mapping: %r" % facts)
    return facts


def read_cursor_facts(pe):
    off = pe.rva_to_off(CURSOR_FUNC - pe.image_base)
    if off is None:
        raise RuntimeError("cursor function outside image")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    facts = []
    for ins in md.disasm(pe.data[off:off + 2048], CURSOR_FUNC):
        text = "%s %s" % (ins.mnemonic, ins.op_str)
        if ("0xba0" in text or "0xba4" in text or
                (ins.mnemonic == "call" and "0x441d90" in text) or
                (ins.mnemonic == "mov" and ins.op_str.startswith("eax, 0x1")) or
                "0x65f95c" in text):
            facts.append((ins.address, ins.mnemonic, ins.op_str.strip()))
        if ins.mnemonic == "ret":
            break
    return facts


def write_report(path, outer, counts, flag_stage, facts, cursor_facts,
                 cursor_keys):
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_stage_select_grid.py\n")
        out.write("Source: docs/decompiled/0x%06x.c (FUN_004ffc80) and "
                  "MMU.exe disassembly of FUN_004ff460.\n" % DECOMPILED)
        out.write("cell fields=this+0xba0,this+0xba4 stage=this+0xbb0 "
                  "toggle=this+0xba8 timer=this+0xbac "
                  "subitems=DAT_006c55a4\n")
        out.write("outer_cells=%d center_counts=%d\n\n" %
                  (len(outer), len(counts)))
        out.write("Outer cells (raw this+0xba0 x, this+0xba4 y -> this+0xbb0):\n")
        for x, y, stage in outer:
            out.write("  x=%d y=%d stage=%d\n" % (x, y, stage))
        out.write("\nCenter cell (x=1,y=1):\n")
        out.write("  this+0xba8 != 0 -> stage=%d\n" % flag_stage)
        for count in sorted(counts):
            out.write("  this+0xba8 == 0, GetSubItemsCount(DAT_006c55a4)=%d "
                      "-> stage=%d\n" % (count, counts[count]))
        out.write("\nFUN_004ff460 raw toggle facts:\n")
        for va, mnemonic, operands in facts:
            out.write("  0x%06x %-5s %s\n" % (va, mnemonic, operands))
        out.write("\nFUN_004ff070 raw cursor facts:\n")
        for va, mnemonic, operands in cursor_facts:
            out.write("  0x%06x %-5s %s\n" % (va, mnemonic, operands))
        out.write("\nFUN_004ff070 raw key -> cursor field mapping:\n")
        for key, field, bound, delta in cursor_keys:
            predicate = "this+0x%x < %d" % (field, bound) if delta > 0 else \
                        "this+0x%x > %d" % (field, bound)
            out.write("  key=0x%x -> this+0x%x %s when %s\n" %
                      (key, field, "+1" if delta > 0 else "-1", predicate))
        out.write("\nFUN_004ff050 raw consumer:\n")
        out.write("  FUN_004032c0(*(undefined4 *)(this + 0xbb0));\n")


def write_header(path, outer, counts, flag_stage, cursor_keys):
    lines = [
        "#pragma once",
        "",
        "#include \"mmu/core/Types.h\"",
        "",
        "namespace mmu {",
        "",
        "// Generated by tools/analyze_stage_select_grid.py. No gameplay labels",
        "// are assigned to the raw fields, offsets, or constants below.",
        "struct StageSelectGridOuterCell {",
        "    int x;",
        "    int y;",
        "    int stage;",
        "};",
        "",
        "inline constexpr StageSelectGridOuterCell kStageSelectOuterCells[] = {",
    ]
    for x, y, stage in outer:
        lines.append("    {%d, %d, %d}," % (x, y, stage))
    lines.extend([
        "};",
        "",
        "inline constexpr int kStageSelectCenterFlagStage = %d;" % flag_stage,
        "inline constexpr int kStageSelectCenterCountMin = %d;" % min(counts),
        "inline constexpr int kStageSelectCenterCountMax = %d;" % max(counts),
        "",
        "constexpr int stageSelectGridOuterStage(int x, int y) {",
        "    for (const StageSelectGridOuterCell& cell : kStageSelectOuterCells) {",
        "        if (cell.x == x && cell.y == y) return cell.stage;",
        "    }",
        "    return -1;",
        "}",
        "",
        "constexpr int stageSelectGridCenterStage(bool flag0xba8, int subItemCount) {",
        "    if (flag0xba8) return kStageSelectCenterFlagStage;",
        "    if (subItemCount < kStageSelectCenterCountMin ||",
        "        subItemCount > kStageSelectCenterCountMax) return -1;",
        "    return kStageSelectCenterFlagStage + subItemCount;",
        "}",
        "",
        "struct StageSelectCursorKey {",
        "    u32 key;",
        "    u32 fieldOffset;",
        "    int bound;",
        "    int delta;",
        "};",
        "",
        "inline constexpr StageSelectCursorKey kStageSelectCursorKeys[] = {",
    ])
    for key, field, bound, delta in cursor_keys:
        lines.append("    {0x%x, 0x%x, %d, %d}," % (key, field, bound, delta))
    lines.extend([
        "};",
        "",
        "constexpr int stageSelectGridStage(int x, int y, bool flag0xba8,",
        "                                   int subItemCount) {",
        "    if (x == 1 && y == 1) return stageSelectGridCenterStage(flag0xba8,",
        "                                                             subItemCount);",
        "    return stageSelectGridOuterStage(x, y);",
        "}",
        "",
        "} // namespace mmu",
        "",
    ])
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def main():
    if len(sys.argv) != 5:
        print(__doc__)
        return 2
    exe_path, root, report_path, header_path = sys.argv[1:]
    with open(exe_path, "rb") as handle:
        pe = PE(handle.read())
    outer, counts, flag_stage = parse_grid(read_text(root, DECOMPILED))
    facts = read_toggle_facts(pe)
    cursor_facts = read_cursor_facts(pe)
    cursor_keys = read_cursor_key_facts(pe)
    write_report(report_path, outer, counts, flag_stage, facts, cursor_facts,
                 cursor_keys)
    write_header(header_path, outer, counts, flag_stage, cursor_keys)
    print("outer=%d center_counts=%s flag_stage=%d toggle_facts=%d keys=%d" %
          (len(outer), ",".join(str(counts[k]) for k in sorted(counts)),
           flag_stage, len(facts), len(cursor_keys)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
