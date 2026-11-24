#!/usr/bin/env python3
"""Recover raw branch facts for the recovered menu state-graph edges.

This selects the twenty menu classes from docs/state_graph.txt and reuses the
verbatim operand collector from analyze_enemy_edge_facts.py. It records the raw
comparison/test instructions, memory operands, qword constants and printable
setState keys found before each setter/cue call in a menu callback. No trigger,
timing, label, field name or gameplay meaning is assigned.

Usage:
  analyze_flow_edge_facts.py MMU.exe report.txt header.h
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from analyze_enemy_edge_facts import collect, read, write_header, write_report
from analyze_rtti import PE

TOOL = "tools/analyze_flow_edge_facts.py"
MENU_CLASSES = {
    "TitleMenu", "MainMenu", "NewGameMenu", "LoadMenu", "MenuStageSelect",
    "OptionsMenu", "MenuInGame", "PauseMenuStatus", "ShopMenu", "BusterMenu",
    "CharacterSelectMenu", "EndlessAttackMenu", "EndlessBossAttackMenu",
    "GauntletMenu", "SpecialMenu", "SuperRobotMastersMenuA",
    "SuperRobotMastersMenuB", "SuperRobotMastersMenuC",
    "SuperRobotMastersMenuD", "WhirlpoolManStageStartMenu",
}
SECTION_RE = re.compile(r"^=== (\S+) vftable=(0x[0-9a-f]+) "
                        r"ctor=(0x[0-9a-f]+) ===$")
EDGE_RE = re.compile(
    r"^  slot (\d+)\s+func=(0x[0-9a-f]+)\s+call=(0x[0-9a-f]+)\s+"
    r"(setState|cue)\s+hash=(0x[0-9a-f]+)\s+(.+?)\s*$")


def parse_edges(text):
    class_name = ""
    edges = []
    for line in text.splitlines():
        header = SECTION_RE.fullmatch(line)
        if header:
            class_name = header.group(1)
            continue
        if class_name not in MENU_CLASSES:
            continue
        match = EDGE_RE.fullmatch(line)
        if not match:
            continue
        slot, func, call, kind, target_hash, target = match.groups()
        edges.append({
            "class": class_name,
            "slot": int(slot),
            "func": int(func, 16),
            "call": int(call, 16),
            "isCue": kind == "cue",
            "hash": int(target_hash, 16),
            "target": target.strip(),
            "owner": "(none)",
        })
    return edges


def main():
    if len(sys.argv) != 4:
        print(__doc__)
        return 2
    exe, report, header = sys.argv[1:]
    root = os.path.dirname(os.path.dirname(report))
    graph = os.path.join(root, "docs", "state_graph.txt")
    pe = PE(open(exe, "rb").read())
    facts = collect(pe, parse_edges(read(graph)))
    write_report(report, facts, tool=TOOL)
    write_header(header, facts, prefix="FlowEdge", tool=TOOL)
    print("edges=%d comparisons=%d constants=%d" %
          (len(facts),
           sum(len(f["comparisons"]) for f in facts),
           sum(len(f["constants"]) for f in facts)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
