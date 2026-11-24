#!/usr/bin/env python3
"""Recover raw constructor thunk/slot blocks for every registered state.

analyze_enemy_state_machines.py proved that each state registration in the
recovered enemy constructors is immediately preceded by three materialised
text thunks of the form `jmp dword ptr [eax + 0xNN]`. This tool applies the
same direct read to all 237 classes in docs/state_graph.txt.

For every state it records the registration call VA, the registration name read
from .rdata, the graph state name, the thunk VAs found in the lookback window,
and the vtable slots those thunks jump through. Incomplete blocks are kept and
marked; no slot is inferred. Nothing here is promoted to a runtime transition.

Usage:
  analyze_state_slot_blocks.py MMU.exe report.txt header.h
"""

import bisect
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from analyze_enemy_edge_facts import read
from analyze_enemy_state_machines import (JMP_SLOT_RE, MOV_IMMEDIATE_RE,
                                          STATE_LOOKBACK, trampoline_slot)
from analyze_rtti import PE
from analyze_state_graph import text_regions
from analyze_state_names import name_before, registration_sites
from capstone import CS_ARCH_X86, CS_MODE_32, Cs

TOOL = "tools/analyze_state_slot_blocks.py"
HEADER_RE = re.compile(
    r"^(\S+) vftable=(0x[0-9a-f]+) ctor=(0x[0-9a-f]+) ===$", re.MULTILINE)
STATES_RE = re.compile(r"^  states \((\d+)\): (.+)$", re.MULTILINE)
MD = Cs(CS_ARCH_X86, CS_MODE_32)


def cpp_string(value):
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def parse_state_graph(text):
    classes = []
    for part in re.split(r"(?m)^=== ", text)[1:]:
        lines = part.rstrip().splitlines()
        header = HEADER_RE.fullmatch(lines[0])
        if not header:
            continue
        class_name, vftable, ctor = header.groups()
        states = None
        for line in lines[1:]:
            match = STATES_RE.fullmatch(line)
            if match:
                states = [item.strip() for item in match.group(2).split(",")]
                if len(states) != int(match.group(1)):
                    raise RuntimeError("state count mismatch for %s" % class_name)
                break
        if states is None:
            raise RuntimeError("missing states for %s" % class_name)
        classes.append({
            "class": class_name,
            "vftable": int(vftable, 16),
            "ctor": int(ctor, 16),
            "states": states,
        })
    return classes


def registration_blocks(pe, base, data, bounds, sites, ctor, states):
    index = bisect.bisect_right(bounds, ctor) - 1
    if index < 0 or index + 1 >= len(bounds):
        raise RuntimeError("constructor 0x%x outside .text" % ctor)
    start, end = bounds[index], bounds[index + 1]
    region_sites = [site for site in sites if start <= site < end]
    if len(region_sites) != len(states):
        raise RuntimeError("constructor 0x%x has %d registrations, graph has %d" %
                           (ctor, len(region_sites), len(states)))
    rows = list(MD.disasm(data[start - base:end - base], start))
    by_address = {row.address: i for i, row in enumerate(rows)}
    blocks = []
    for state, site in zip(states, region_sites):
        row_index = by_address.get(site)
        if row_index is None:
            raise RuntimeError("registration 0x%x not disassembled" % site)
        values = []
        for row in rows[max(0, row_index - STATE_LOOKBACK):row_index]:
            match = MOV_IMMEDIATE_RE.fullmatch(row.op_str)
            if row.mnemonic != "mov" or not match:
                continue
            va = int(match.group(1), 16)
            if not pe.in_text(va) or va in values:
                continue
            values.append(va)
        thunks = []
        slots = []
        for va in values[-3:]:
            slot = trampoline_slot(pe, va)
            if slot is None:
                continue
            thunks.append(va)
            slots.append(slot)
        blocks.append({
            "state": state,
            "registrationName": name_before(pe, site) or "",
            "registrationVa": site,
            "thunks": thunks,
            "slots": slots,
            "complete": len(slots) == 3,
        })
    return blocks, start, end


def collect(pe, classes):
    base, data, bounds = text_regions(pe)
    sites = registration_sites(pe)
    rows = []
    for cls in classes:
        blocks, start, end = registration_blocks(
            pe, base, data, bounds, sites, cls["ctor"], cls["states"])
        for block in blocks:
            rows.append({
                "class": cls["class"],
                "vftable": cls["vftable"],
                "ctor": cls["ctor"],
                "regionEnd": end,
                **block,
            })
    return rows


def format_slots(block):
    if not block["slots"]:
        return "-"
    return ",".join(str(slot) for slot in block["slots"])


def format_thunks(block):
    if not block["thunks"]:
        return "-"
    return ",".join("0x%06x" % va for va in block["thunks"])


def write_report(path, blocks, classes):
    complete = sum(block["complete"] for block in blocks)
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by %s\n" % TOOL)
        out.write("Direct `jmp dword ptr [eax + 0xNN]` thunk lookback before\n")
        out.write("each 0x401370 state registration in docs/state_graph.txt.\n")
        out.write("classes=%d states=%d complete=%d incomplete=%d\n\n" %
                  (len(classes), len(blocks), complete, len(blocks) - complete))
        seen = set()
        for block in blocks:
            key = (block["class"], block["ctor"])
            if key not in seen:
                seen.add(key)
                out.write("=== %s vftable=0x%x ctor=0x%x region=0x%x..0x%x "
                          "states=%d ===\n" %
                          (block["class"], block["vftable"], block["ctor"],
                           block["ctor"], block["regionEnd"],
                           sum(1 for row in blocks
                               if row["class"] == block["class"] and
                               row["ctor"] == block["ctor"])))
            out.write("  state=%-24s reg=0x%06x regname=%-24s slots=%-12s "
                      "thunks=%-30s %s\n" %
                      (block["state"], block["registrationVa"],
                       block["registrationName"] or "?",
                       format_slots(block), format_thunks(block),
                       "complete" if block["complete"] else "incomplete"))
        out.write("\n")


def write_header(path, blocks):
    lines = [
        "#pragma once",
        "",
        "#include \"mmu/core/Types.h\"",
        "",
        "#include <string_view>",
        "",
        "namespace mmu {",
        "",
        "// Generated by %s from MMU.exe." % TOOL,
        "// Raw thunk lookback for each 0x401370 state registration; incomplete",
        "// blocks are retained and no transition meaning is assigned.",
        "struct StateSlotBlock {",
        "    std::string_view className;",
        "    u32 vftable;",
        "    u32 constructorVa;",
        "    u32 regionEnd;",
        "    std::string_view stateName;",
        "    std::string_view registrationName;",
        "    u32 registrationVa;",
        "    u8 slotCount;",
        "    u32 slots[3];",
        "    u32 thunks[3];",
        "    bool complete;",
        "};",
        "",
        "inline constexpr StateSlotBlock kStateSlotBlocks[] = {",
    ]
    for block in blocks:
        slots = list(block["slots"]) + [0] * (3 - len(block["slots"]))
        thunks = list(block["thunks"]) + [0] * (3 - len(block["thunks"]))
        lines.append(
            "    {%s, 0x%06xu, 0x%06xu, 0x%06xu, %s, %s, 0x%06xu, %du, "
            "{%du, %du, %du}, {0x%06xu, 0x%06xu, 0x%06xu}, %s}," %
            (cpp_string(block["class"]), block["vftable"], block["ctor"],
             block["regionEnd"], cpp_string(block["state"]),
             cpp_string(block["registrationName"]), block["registrationVa"],
             len(block["slots"]), slots[0], slots[1], slots[2],
             thunks[0], thunks[1], thunks[2],
             "true" if block["complete"] else "false"))
    lines.extend([
        "};",
        "",
        "inline const StateSlotBlock* firstStateSlotBlock(std::string_view className) {",
        "    for (const StateSlotBlock& block : kStateSlotBlocks)",
        "        if (block.className == className) return &block;",
        "    return nullptr;",
        "}",
        "",
        "inline const StateSlotBlock* stateSlotBlock(std::string_view className,",
        "                                            std::string_view stateName) {",
        "    for (const StateSlotBlock& block : kStateSlotBlocks)",
        "        if (block.className == className && block.stateName == stateName)",
        "            return &block;",
        "    return nullptr;",
        "}",
        "",
        "} // namespace mmu",
        "",
    ])
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def main():
    if len(sys.argv) != 4:
        print(__doc__)
        return 2
    exe, report, header = sys.argv[1:]
    root = os.path.dirname(os.path.dirname(report))
    pe = PE(open(exe, "rb").read())
    classes = parse_state_graph(
        read(os.path.join(root, "docs", "state_graph.txt")))
    blocks = collect(pe, classes)
    write_report(report, blocks, classes)
    write_header(header, blocks)
    complete = sum(block["complete"] for block in blocks)
    print("classes=%d states=%d complete=%d incomplete=%d" %
          (len(classes), len(blocks), complete, len(blocks) - complete))
    return 0


if __name__ == "__main__":
    sys.exit(main())
