#!/usr/bin/env python3
"""Recover raw activity-mask setter call sites from MMU.exe.

FUN_00507e90(mask) overwrites the activity mask returned by FUN_00507ec0 and
copies the previous value to manager +4. Both helpers operate on the global
manager object at 0x6c55a0. The mask gates keyed save-flag records whose
category has no bit in common with it (FUN_00441820).

This analyzer does not assign gameplay semantics to the mask values. It records
the literal mask pushed at each direct call site, the containing function, the
vtable owner when one is known, and the state/cue evidence already recovered by
docs/state_graph.txt and the Ghidra decompilation.

Outputs:
  docs/activity_masks.txt
  core/include/mmu/core/ActivityMaskTable.h
"""

import os
import re
import struct
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM

from analyze_rtti import PE, find_all

SETTER_VA = 0x507E90
GETTER_VA = 0x507EC0
MANAGER_GLOBAL_VA = 0x6C55A0

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VTABLES_PATH = os.path.join(PROJECT_ROOT, "docs", "vtables.txt")
STATE_GRAPH_PATH = os.path.join(PROJECT_ROOT, "docs", "state_graph.txt")
DECOMPILED_DIR = os.path.join(PROJECT_ROOT, "docs", "decompiled")
REPORT_PATH = os.path.join(PROJECT_ROOT, "docs", "activity_masks.txt")
HEADER_PATH = os.path.join(
    PROJECT_ROOT, "core", "include", "mmu", "core", "ActivityMaskTable.h")

MD = Cs(CS_ARCH_X86, CS_MODE_32)
MD.skipdata = True
MD.detail = True

STRING_RE = re.compile(r'"((?:[^"\\]|\\.)*)"')


def read_literal_push(pe, call_va):
    """Return the nearest validated immediate push before a call VA."""
    off = pe.rva_to_off(call_va - pe.image_base)
    if off is None:
        return None
    candidates = []
    for back in range(1, 97):
        pos = off - back
        if pos < 0:
            break
        opcode = pe.data[pos]
        if opcode == 0x68 and pos + 5 <= off:
            imm = struct.unpack_from("<I", pe.data, pos + 1)[0]
            size = 5
        elif opcode == 0x6A and pos + 2 <= off:
            imm = struct.unpack_from("<b", pe.data, pos + 1)[0] & 0xFFFFFFFF
            size = 2
        else:
            continue
        start_va = call_va - back
        insns = list(MD.disasm(pe.data[pos:off], start_va))
        if not insns:
            continue
        if insns[0].address != start_va or insns[0].mnemonic != "push":
            continue
        if not insns[0].operands or insns[0].operands[0].type != X86_OP_IMM:
            continue
        if insns[-1].address + insns[-1].size != call_va:
            continue
        if (insns[0].operands[0].imm & 0xFFFFFFFF) != imm:
            continue
        candidates.append((back, imm, size))
    if not candidates:
        return None
    return min(candidates)[1]


def find_function_start(pe, call_va):
    """Find the closest preceding standard prologue with a decompilation."""
    off = pe.rva_to_off(call_va - pe.image_base)
    if off is None:
        return 0
    fallback = 0
    for back in range(1, 4097):
        pos = off - back
        if pos < 0:
            break
        if pe.data[pos:pos + 3] != b"\x55\x8b\xec":
            continue
        va = pe.image_base + pe.off_to_rva(pos)
        if fallback == 0:
            fallback = va
        path = os.path.join(DECOMPILED_DIR, "0x%06x.c" % va)
        if os.path.exists(path):
            return va
    return fallback


def demangle(name):
    if name.startswith(".?AV") and name.endswith("@@"):
        return name[4:-2]
    return name


def parse_vtable_owners():
    owners = {}
    if not os.path.exists(VTABLES_PATH):
        return owners
    lines = open(VTABLES_PATH, encoding="latin1").read().splitlines()
    for i, line in enumerate(lines[:-1]):
        m = re.match(r"(\S+)\s+vftable", line)
        if not m:
            continue
        name = demangle(m.group(1))
        for slot, value in enumerate(
                re.findall(r"0x([0-9a-fA-F]+)", lines[i + 1])):
            owners.setdefault(int(value, 16), []).append((name, slot))
    return owners


def parse_state_graph():
    evidence = {}
    if not os.path.exists(STATE_GRAPH_PATH):
        return evidence
    current = ""
    for line in open(STATE_GRAPH_PATH, encoding="latin1"):
        m = re.match(r"=== (\S+) vftable=", line)
        if m:
            current = m.group(1)
            continue
        m = re.search(
            r"slot\s+(\d+)\s+func=0x([0-9a-fA-F]+)\s+call=0x[0-9a-fA-F]+\s+"
            r"(cue|setState)\s+hash=0x[0-9a-fA-F]+\s+(\S+)",
            line)
        if not m:
            continue
        slot, func, kind, name = m.groups()
        evidence.setdefault(int(func, 16), []).append(
            (current, int(slot), kind, name))
    return evidence


def parse_decompiled_strings(function_va):
    path = os.path.join(DECOMPILED_DIR, "0x%06x.c" % function_va)
    if not os.path.exists(path):
        return []
    result = []
    for line in open(path, encoding="latin1"):
        for raw in STRING_RE.findall(line):
            if "\\x" in raw or len(raw) > 120:
                continue
            value = raw.replace('\\"', '"').replace("\\\\", "\\")
            if value not in result:
                result.append(value)
    return result


def escape(value):
    return value.replace("\\", "\\\\").replace('"', '\\"')


def collect_rows(pe):
    call_vas = []
    for off in find_all(pe.data, b"\xe8"):
        if off + 5 > len(pe.data):
            continue
        disp = struct.unpack_from("<i", pe.data, off + 1)[0]
        rva = pe.off_to_rva(off)
        if rva is None:
            continue
        site_va = pe.image_base + rva
        call_va = site_va + 5 + disp
        if call_va == SETTER_VA:
            call_vas.append(site_va)

    owners = parse_vtable_owners()
    states = parse_state_graph()
    rows = []
    for call_va in sorted(set(call_vas)):
        mask = read_literal_push(pe, call_va)
        if mask is None:
            raise RuntimeError("no literal mask at 0x%06x" % call_va)
        function_va = find_function_start(pe, call_va)
        owner_rows = owners.get(function_va, [])
        if owner_rows:
            owner, slot = owner_rows[0]
        else:
            owner, slot = "", 0xFFFF
        state_rows = states.get(function_va, [])
        state_evidence = ",".join(sorted({"%s:%s" % (kind, name)
                                          for _cls, _slot, kind, name
                                          in state_rows}))
        decompiled = parse_decompiled_strings(function_va)
        rows.append({
            "functionVa": function_va,
            "callVa": call_va,
            "mask": mask,
            "owner": owner,
            "slot": slot,
            "stateEvidence": state_evidence,
            "strings": decompiled,
        })
    return rows


def write_report(rows):
    masks = sorted({row["mask"] for row in rows})
    with open(REPORT_PATH, "w", encoding="latin1") as out:
        out.write("Generated by tools/analyze_activity_masks.py\n")
        out.write("Source: direct calls to FUN_00507e90(mask) in MMU.exe.\n")
        out.write("setter=0x%06x getter=0x%06x manager=[0x%06x]\n" % (
            SETTER_VA, GETTER_VA, MANAGER_GLOBAL_VA))
        out.write("sites=%d unique_masks=%d masks=%s\n\n" % (
            len(rows), len(masks),
            " ".join("0x%x" % mask for mask in masks)))
        for row in rows:
            owner = row["owner"] or "-"
            slot = ("" if row["slot"] == 0xFFFF
                    else " slot=%u" % row["slot"])
            state = row["stateEvidence"] or "-"
            strings = "|".join(row["strings"]) or "-"
            out.write(
                "func=0x%06x call=0x%06x mask=0x%03x owner=%s%s "
                "states=%s strings=%s\n" % (
                    row["functionVa"], row["callVa"], row["mask"], owner,
                    slot, state, strings))


def write_header(rows):
    with open(HEADER_PATH, "w", encoding="latin1") as out:
        out.write("#pragma once\n\n")
        out.write('#include "mmu/core/Types.h"\n\n')
        out.write("#include <cstddef>\n#include <string_view>\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_activity_masks.py.\n")
        out.write("// FUN_00507e90(mask) sets manager +0 to mask and copies the\n")
        out.write("// previous value to +4; FUN_00507ec0 reads manager +0. The\n")
        out.write("// mask gates keyed save-flag records in FUN_00441820. These\n")
        out.write("// sites record literal call evidence only: no gameplay state\n")
        out.write("// or category meaning is inferred from the mask value.\n")
        out.write("inline constexpr u32 kActivityMaskSetterVa = 0x%06xu;\n" %
                  SETTER_VA)
        out.write("inline constexpr u32 kActivityMaskGetterVa = 0x%06xu;\n" %
                  GETTER_VA)
        out.write("inline constexpr u32 kActivityMaskManagerVa = 0x%06xu;\n\n" %
                  MANAGER_GLOBAL_VA)
        out.write("struct RecoveredActivityMaskCall {\n")
        out.write("    u32 functionVa;\n")
        out.write("    u32 callVa;\n")
        out.write("    u32 mask;\n")
        out.write("    std::string_view owner;\n")
        out.write("    u16 slot;\n")
        out.write("    std::string_view stateEvidence;\n")
        out.write("    std::string_view decompiledStrings;\n")
        out.write("};\n\n")
        out.write("inline constexpr RecoveredActivityMaskCall "
                  "kRecoveredActivityMaskCalls[] = {\n")
        for row in rows:
            strings = "|".join(row["strings"])
            out.write(
                "    {0x%06xu, 0x%06xu, 0x%03xu, \"%s\", 0x%04xu, \"%s\", "
                "\"%s\"},\n" % (
                    row["functionVa"], row["callVa"], row["mask"],
                    escape(row["owner"]), row["slot"],
                    escape(row["stateEvidence"]), escape(strings)))
        out.write("};\n")
        out.write("inline constexpr std::size_t kRecoveredActivityMaskCallCount =\n")
        out.write("    sizeof(kRecoveredActivityMaskCalls) /\n")
        out.write("    sizeof(kRecoveredActivityMaskCalls[0]);\n\n")
        out.write("inline const RecoveredActivityMaskCall* "
                  "findActivityMaskCall(u32 callVa) {\n")
        out.write("    for (const auto& row : kRecoveredActivityMaskCalls)\n")
        out.write("        if (row.callVa == callVa) return &row;\n")
        out.write("    return nullptr;\n}\n\n")
        out.write("inline const RecoveredActivityMaskCall* "
                  "findActivityMaskFunction(u32 functionVa) {\n")
        out.write("    for (const auto& row : kRecoveredActivityMaskCalls)\n")
        out.write("        if (row.functionVa == functionVa) return &row;\n")
        out.write("    return nullptr;\n}\n\n")
        out.write("inline std::size_t countActivityMaskCalls(u32 mask) {\n")
        out.write("    std::size_t count = 0;\n")
        out.write("    for (const auto& row : kRecoveredActivityMaskCalls)\n")
        out.write("        if (row.mask == mask) ++count;\n")
        out.write("    return count;\n")
        out.write("}\n\n")
        out.write("} // namespace mmu\n")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    pe = PE(open(sys.argv[1], "rb").read())
    if pe.image_base != 0x400000:
        raise RuntimeError("unexpected image base 0x%x" % pe.image_base)
    rows = collect_rows(pe)
    if not rows:
        raise RuntimeError("no activity-mask setter calls found")
    write_report(rows)
    write_header(rows)
    print("sites=%d report=%s header=%s" % (
        len(rows), REPORT_PATH, HEADER_PATH))
    return 0


if __name__ == "__main__":
    sys.exit(main())
