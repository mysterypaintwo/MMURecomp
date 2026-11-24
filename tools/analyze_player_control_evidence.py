#!/usr/bin/env python3
"""Join raw MegaMan/ProtoMan control evidence into a portable table.

Only checked-in text artifacts are read. The tool copies exact registered state
hashes, verified GameParam key hashes, raw state-graph rows, character-select
rows, the use_charge_shot proto line, inline menu resource paths, and verbatim
decompiled operand lines. It does not decode timing values, assign gameplay
meaning to masks, or derive portable physics constants.

Usage:
  python3 tools/analyze_player_control_evidence.py <root> <report> <header>
"""

import os
import re
import sys

CLASSES = (
    "MegaMan", "ProtoMan", "Bass", "Zero", "Treble", "ShadowMegaMan",
    "MavProtoChargedShot", "MegaTankShot",
)
INTEREST_PARTS = ("Jump", "Shoot", "Charge", "Buster", "Slide", "Dash")
STATE_NAMES = (
    "Stand", "Step", "Run", "Fall", "Jump", "JumpDodge", "Shoot",
    "Charge", "Charging", "ChargeUp", "Buster", "Slide", "SlideIn",
    "SlideOut", "CometDash", "Spawn", "Wait", "Death",
)
REQUIRED_STATE_NAMES = ("Stand", "Step", "Run", "Fall", "Spawn", "Wait", "Death")
KEY_NAMES = (
    "MoveSpeed", "JumpHeight", "ShootDelay", "DashSpeed", "DashMoveSpeed",
    "RunMoveSpeed", "RunSpeed", "SlideSpeed", "SlideMoveSpeed", "slide_speed",
    "SlideDistance", "ChargeTime", "ChargeFlashDuration", "JumpDelay",
    "JumpDistance", "max_fall_speed", "FallSpeed",
)
RESOURCE_NAMES = ("MegaBusterIcon.xspr", "ZDoubleJumpIcon.xspr")

DECOMPILED = (
    ("0x5b02f0.c", (
        '"Death"', '"Hurt"', '"Ride"', '"ZeroMessenko"',
        "local_18 = *param_3;", "param_3[1]", "FUN_0061daf0(3)",
        '"Stand"', '"ShotDeflect"', '"Deflect"', '"HurtSlide"',
        '"HurtKnockback"',
    )),
    ("0x5aa470.c", (
        '"Wait"', '"Transition"', '"Fall"', '"Hurt"', '"ZeroShoryuken"',
        '"ZeroMessenkoDrop"', '"Carried"', '"Death"',
        "FUN_0043d3a0(0x2000000,3,10)", "FUN_0043bc10",
    )),
    ("0x5b15d0.c", (
        '"Death"', '"Wait"', '"Fly"', "0x8000000U", '"stand"',
        '"HurtSlide"', '"Fall"', "_DAT_006c5598", '"GravityChange"',
    )),
    ("0x5aea90.c", (
        "FUN_0061daf0(6)", "FUN_0061daf0(8)", '"CometDash"',
        "0x1000", "0x2000", "0x10000000",
    )),
    ("0x5aefc0.c", (
        "FUN_0061daf0(6)", "FUN_0061daf0(8)", '"Slide"',
        "0x1000", "0x2000", "0x10000000",
    )),
    ("0x5a7a50.c", (
        '"LayingDown"', '"stand"', "FUN_0061daf0(3)",
        '"standglow"', "0x10U",
    )),
)

STATE_RE = re.compile(
    r"^#\d+ hash=(0x[0-9a-f]+) first=(0x[0-9a-f]+) refs=\s*(\d+)\s+(\S+)$")
GRAPH_HEADER_RE = re.compile(
    r"^=== (\S+) vftable=(0x[0-9a-f]+) ctor=(0x[0-9a-f]+) ===$")
GRAPH_EDGE_RE = re.compile(
    r"slot\s+(\d+)\s+func=(0x[0-9a-f]+)\s+call=(0x[0-9a-f]+)\s+"
    r"(setState|cue)\s+hash=(0x[0-9a-f]+)\s+(\S+)$")
ACTION_RE = re.compile(r"action=(0x[0-9a-f]+) kind=CharacterSelected")
MENU_RE = re.compile(
    r"^\s*(\d+) action=(0x[0-9a-f]+) flash\s+'([^']+)'\s+"
    r"labelVa=(0x[0-9a-f]+) object=(0x[0-9a-f]+)$")
ACTION_VALUE_RE = re.compile(r"0x61dc80\(U,(\d+)\)")
RESOURCE_RE = re.compile(
    r"key=(0x[0-9a-f]+)\s+(\S+\.xspr)$")


def read(path):
    with open(path, "r", encoding="utf-8") as handle:
        return handle.read()


def state_names(text):
    result = {}
    for line in text.splitlines():
        match = STATE_RE.match(line)
        if match and match.group(4) in STATE_NAMES:
            result[match.group(4)] = {
                "hash": int(match.group(1), 16),
                "firstVa": int(match.group(2), 16),
                "refs": int(match.group(3)),
            }
    missing = [name for name in REQUIRED_STATE_NAMES if name not in result]
    if missing:
        raise RuntimeError("missing registered state names %s" % missing)
    return result


def key_names(text):
    result = {}
    for line in text.splitlines():
        fields = line.split()
        if len(fields) == 2 and re.fullmatch(r"0x[0-9a-f]+", fields[0]):
            if fields[1] in KEY_NAMES:
                result[fields[1]] = int(fields[0], 16)
    missing = [name for name in KEY_NAMES if name not in result]
    if missing:
        raise RuntimeError("missing verified key names %s" % missing)
    return result


def graph_sections(text):
    lines = text.splitlines()
    sections = {}
    for start, line in enumerate(lines):
        header = GRAPH_HEADER_RE.match(line)
        if not header or header.group(1) not in CLASSES:
            continue
        rows = []
        for child in lines[start + 1:]:
            if not child.strip() or child.startswith("=== "):
                break
            rows.append(child)
        initial = None
        for row in rows:
            match = re.search(r"initial: (\S+) \(setState at (0x[0-9a-f]+)\)", row)
            if match:
                initial = (match.group(1), int(match.group(2), 16))
                break
        edges = []
        for row in rows:
            match = GRAPH_EDGE_RE.search(row)
            if not match:
                continue
            target = match.group(6)
            if not any(part in target for part in INTEREST_PARTS):
                continue
            edges.append({
                "slot": int(match.group(1)),
                "functionVa": int(match.group(2), 16),
                "callVa": int(match.group(3), 16),
                "kind": match.group(4),
                "hash": int(match.group(5), 16),
                "target": target,
            })
        sections[header.group(1)] = {
            "vftable": int(header.group(2), 16),
            "ctor": int(header.group(3), 16),
            "initial": initial,
            "initialLine": next((row for row in rows if "initial:" in row), ""),
            "lines": rows,
            "edges": edges,
        }
    missing = [name for name in CLASSES if name not in sections]
    if missing:
        raise RuntimeError("missing state graph classes %s" % missing)
    return sections


def characters(menu_actions, menu_items):
    values = {}
    lines = menu_actions.splitlines()
    for index, line in enumerate(lines):
        action = ACTION_RE.search(line)
        if not action:
            continue
        for child in lines[index + 1:index + 3]:
            value = ACTION_VALUE_RE.search(child)
            if value:
                values[action.group(1)] = int(value.group(1))
                break
    result = []
    for line in menu_items.splitlines():
        match = MENU_RE.match(line)
        if not match or match.group(2) not in values:
            continue
        result.append({
            "index": int(match.group(1)),
            "action": int(match.group(2), 16),
            "label": match.group(3),
            "labelVa": int(match.group(4), 16),
            "object": int(match.group(5), 16),
            "character": values[match.group(2)],
        })
    return result


def resources(text):
    result = []
    for line in text.splitlines():
        match = RESOURCE_RE.search(line)
        if not match:
            continue
        path = match.group(2)
        if not any(name in path for name in RESOURCE_NAMES):
            continue
        result.append({"key": int(match.group(1), 16), "path": path})
    return result


def operands(root):
    rows = []
    for filename, tokens in DECOMPILED:
        path = os.path.join(root, "docs", "decompiled", filename)
        function_va = int(filename[:-2], 16)
        for number, line in enumerate(read(path).splitlines(), 1):
            if any(token in line for token in tokens):
                rows.append((function_va, number, line.rstrip()))
    return rows


def analyze(root):
    docs = os.path.join(root, "docs")
    graph = graph_sections(read(os.path.join(docs, "state_graph.txt")))
    return {
        "states": state_names(read(os.path.join(docs, "state_names.txt"))),
        "keys": key_names(read(os.path.join(docs, "key_names.txt"))),
        "graph": graph,
        "characters": characters(read(os.path.join(docs, "menu_actions.txt")),
                                 read(os.path.join(docs, "menu_items.txt"))),
        "chargeLine": next(
            line.strip() for line in
            read(os.path.join(docs, "protos", "savedgame.proto")).splitlines()
            if "use_charge_shot" in line),
        "resources": resources(read(os.path.join(
            docs, "inline_menu_resources.txt"))),
        "operands": operands(root),
    }


def cpp_string(value):
    return '"%s"' % value.replace("\\", "\\\\").replace("\"", "\\\"")


def write_report(path, data):
    lines = [
        "Generated by tools/analyze_player_control_evidence.py",
        "Sources: docs/state_names.txt, docs/key_names.txt, docs/state_graph.txt,",
        "docs/menu_actions.txt, docs/menu_items.txt, docs/protos/savedgame.proto,",
        "docs/inline_menu_resources.txt and docs/decompiled/*.c.",
        "No portable physics constant or effect timing is derived here.",
        "",
        "Registered state name hashes (verbatim rows):",
    ]
    state_lines = read_state_rows(os.path.join(os.path.dirname(path), "state_names.txt"),
                                  data["states"])
    lines.extend(state_lines)
    lines.extend(["", "Verified GameParam key hashes (exact names, no values):"])
    for name in KEY_NAMES:
        lines.append("  0x%08x %s" % (data["keys"][name], name))
    lines.extend(["", "State graph class facts and interest edges (verbatim rows):"])
    for name in CLASSES:
        section = data["graph"][name]
        lines.append("  %s vftable=0x%06x ctor=0x%06x" %
                     (name, section["vftable"], section["ctor"]))
        lines.append("    " + section["initialLine"])
        for edge in section["edges"]:
            lines.append("    slot=%-2u func=0x%06x call=0x%06x %-7s "
                         "hash=0x%08x %s" %
                         (edge["slot"], edge["functionVa"], edge["callVa"],
                          edge["kind"], edge["hash"], edge["target"]))
    lines.extend(["", "Character selection rows (verbatim values):"])
    for row in data["characters"]:
        lines.append("  index=%u action=0x%02x label='%s' labelVa=0x%06x "
                     "object=0x%06x character=%u" %
                     (row["index"], row["action"], row["label"],
                      row["labelVa"], row["object"], row["character"]))
    lines.extend(["", "Charge-shot saved-game field (verbatim):", "  " +
                  data["chargeLine"]])
    lines.extend(["", "Inline menu resources (verbatim rows):"])
    for row in data["resources"]:
        lines.append("  key=0x%08x path=%s" % (row["key"], row["path"]))
    lines.extend(["", "Decompiled operand lines (verbatim):"])
    for function_va, number, text in data["operands"]:
        lines.append("  0x%06x.c:%u %s" % (function_va, number, text))
    lines.append("")
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def read_state_rows(state_names_path, states):
    rows = []
    for line in read(state_names_path).splitlines():
        match = STATE_RE.match(line)
        if match and match.group(4) in states:
            rows.append("  " + line)
    return rows


def write_header(path, data):
    lines = [
        "#pragma once",
        "",
        '#include "mmu/core/Types.h"',
        "",
        "#include <cstddef>",
        "#include <string_view>",
        "",
        "namespace mmu {",
        "",
        "// Generated by tools/analyze_player_control_evidence.py.",
        "// Exact names, hashes, VAs and verbatim operands only; no timing.",
        "struct PlayerControlState {",
        "    std::string_view name;",
        "    u32 hash;",
        "    u32 firstVa;",
        "    u32 refs;",
        "};",
        "",
        "inline constexpr PlayerControlState kPlayerControlStates[] = {",
    ]
    for name in data["states"]:
        row = data["states"][name]
        lines.append("    {%s, 0x%08xu, 0x%06xu, %du}," %
                     (cpp_string(name), row["hash"], row["firstVa"], row["refs"]))
    lines.extend([
        "};",
        "",
        "struct PlayerControlKey {",
        "    std::string_view name;",
        "    u32 hash;",
        "};",
        "",
        "inline constexpr PlayerControlKey kPlayerControlKeys[] = {",
    ])
    for name in KEY_NAMES:
        lines.append("    {%s, 0x%08xu}," %
                     (cpp_string(name), data["keys"][name]))
    lines.extend([
        "};",
        "",
        "struct PlayerControlEdge {",
        "    std::string_view className;",
        "    std::string_view target;",
        "    u16 slot;",
        "    u32 functionVa;",
        "    u32 callVa;",
        "    u32 hash;",
        "    bool isCue;",
        "};",
        "",
        "inline constexpr PlayerControlEdge kPlayerControlEdges[] = {",
    ])
    for class_name in CLASSES:
        for edge in data["graph"][class_name]["edges"]:
            lines.append("    {%s, %s, %du, 0x%06xu, 0x%06xu, 0x%08xu, %s}," %
                         (cpp_string(class_name), cpp_string(edge["target"]),
                          edge["slot"], edge["functionVa"], edge["callVa"],
                          edge["hash"], "true" if edge["kind"] == "cue" else "false"))
    lines.extend([
        "};",
        "",
        "struct PlayerControlCharacter {",
        "    u32 index;",
        "    u32 action;",
        "    u32 character;",
        "    u32 labelVa;",
        "    u32 object;",
        "    std::string_view label;",
        "};",
        "",
        "inline constexpr PlayerControlCharacter kPlayerControlCharacters[] = {",
    ])
    for row in data["characters"]:
        lines.append("    {%du, 0x%02xu, %du, 0x%06xu, 0x%06xu, %s}," %
                     (row["index"], row["action"], row["character"],
                      row["labelVa"], row["object"], cpp_string(row["label"])))
    lines.extend([
        "};",
        "",
        "inline constexpr std::string_view kPlayerControlChargeShotField = %s;" %
        cpp_string(data["chargeLine"]),
        "",
        "struct PlayerControlResource {",
        "    u32 key;",
        "    std::string_view path;",
        "};",
        "",
        "inline constexpr PlayerControlResource kPlayerControlResources[] = {",
    ])
    for row in data["resources"]:
        lines.append("    {0x%08xu, %s}," %
                     (row["key"], cpp_string(row["path"])))
    lines.extend([
        "};",
        "",
        "struct PlayerControlOperandLine {",
        "    u32 functionVa;",
        "    u32 sourceLine;",
        "    std::string_view text;",
        "};",
        "",
        "inline constexpr PlayerControlOperandLine kPlayerControlOperandLines[] = {",
    ])
    for function_va, number, text in data["operands"]:
        lines.append("    {0x%06xu, %du, %s}," %
                     (function_va, number, cpp_string(text)))
    lines.extend([
        "};",
        "",
        "inline constexpr std::size_t kPlayerControlStateCount =",
        "    sizeof(kPlayerControlStates) / sizeof(kPlayerControlStates[0]);",
        "inline constexpr std::size_t kPlayerControlKeyCount =",
        "    sizeof(kPlayerControlKeys) / sizeof(kPlayerControlKeys[0]);",
        "inline constexpr std::size_t kPlayerControlEdgeCount =",
        "    sizeof(kPlayerControlEdges) / sizeof(kPlayerControlEdges[0]);",
        "inline constexpr std::size_t kPlayerControlCharacterCount =",
        "    sizeof(kPlayerControlCharacters) / sizeof(kPlayerControlCharacters[0]);",
        "inline constexpr std::size_t kPlayerControlResourceCount =",
        "    sizeof(kPlayerControlResources) / sizeof(kPlayerControlResources[0]);",
        "inline constexpr std::size_t kPlayerControlOperandLineCount =",
        "    sizeof(kPlayerControlOperandLines) / sizeof(kPlayerControlOperandLines[0]);",
        "",
        "inline const PlayerControlState* playerControlState(std::string_view name) {",
        "    for (const PlayerControlState& row : kPlayerControlStates)",
        "        if (row.name == name) return &row;",
        "    return nullptr;",
        "}",
        "",
        "inline const PlayerControlKey* playerControlKey(std::string_view name) {",
        "    for (const PlayerControlKey& row : kPlayerControlKeys)",
        "        if (row.name == name) return &row;",
        "    return nullptr;",
        "}",
        "",
        "inline const PlayerControlOperandLine* playerControlOperandLine(",
        "    u32 functionVa, std::size_t index) {",
        "    std::size_t seen = 0;",
        "    for (const PlayerControlOperandLine& row : kPlayerControlOperandLines) {",
        "        if (row.functionVa != functionVa) continue;",
        "        if (seen++ == index) return &row;",
        "    }",
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
    root, report, header = sys.argv[1:]
    data = analyze(root)
    write_report(report, data)
    write_header(header, data)
    print("states=%d keys=%d edges=%d chars=%d resources=%d operands=%d" % (
        len(data["states"]), len(data["keys"]),
        sum(len(section["edges"]) for section in data["graph"].values()),
        len(data["characters"]), len(data["resources"]), len(data["operands"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
