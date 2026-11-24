#!/usr/bin/env python3
"""Join raw Camera/CameraTarget evidence into one portable table.

This tool only copies raw rows already present in the checked-in decompilation
and analysis reports: RTTI vftable slots, CameraTarget constructor state
registrations, observed setter edges, and verbatim operand lines from the
Camera update functions and the MegaMan slot-11 position function. It assigns
no scroll speed, direction, timing, trigger, room destination or gameplay
meaning to any raw constant.

Usage:
  python3 tools/analyze_camera_evidence.py <root> <report> <header>
"""

import os
import re
import sys

DECOMPILED = [
    "0x444d90.c",  # Camera vftable slot 15
    "0x444d10.c",  # Camera vftable slot 11
    "0x43e510.c",  # delta helper called by the Camera update
    "0x5aa470.c",  # MegaMan vftable slot 11 position operands
]

OPERAND_TOKENS = (
    "_DAT_006987d0", "_DAT_00698448", "_DAT_00698e48", "_DAT_00698e40",
    "_DAT_00698458", "_DAT_00698790", "_DAT_00698788", "_DAT_006c55e4",
    "_DAT_006c1fd8", "rand()", "FUN_00444af0", "FUN_006253a0",
    "FUN_0043e510", "FUN_00401930", "FUN_0043bc10", "FUN_00441b70",
    '"Carried"',
)

VFTABLE_HEADER_RE = re.compile(
    r"^\.?\?AV(?P<name>[A-Za-z0-9_]+)@@  vftable RVA "
    r"(?P<rva>0x[0-9a-f]+) \(VA (?P<va>0x[0-9a-f]+)\)$")
EDGE_RE = re.compile(
    r"slot\s+(?P<slot>\d+)\s+func=(?P<func>0x[0-9a-f]+)\s+"
    r"call=(?P<call>0x[0-9a-f]+)\s+setState hash=(?P<hash>0x[0-9a-f]+)\s+"
    r"(?P<name>\S+)")
SLOT_BLOCK_RE = re.compile(
    r"state=(?P<name>\S+)\s+reg=(?P<reg>0x[0-9a-f]+)\s+"
    r"regname=(?P<regname>\S+)\s+slots=(?P<slots>[0-9,]+)\s+"
    r"thunks=(?P<thunks>(?:0x[0-9a-f]+,?)+)(?P<complete>\s+complete)?")


def read(path):
    with open(path, "r", encoding="utf-8") as handle:
        return handle.read()


def vtable(text, class_name):
    lines = text.splitlines()
    for index, line in enumerate(lines):
        match = VFTABLE_HEADER_RE.match(line)
        if not match or match.group("name") != class_name:
            continue
        functions = [int(value, 16) for value in lines[index + 1].split()]
        return int(match.group("va"), 16), functions
    raise RuntimeError("missing vftable %s" % class_name)


def section(text, prefix):
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if line.startswith(prefix):
            result = []
            for child in lines[index:]:
                if result and (not child.strip() or child.startswith("=== ")):
                    break
                result.append(child)
            return result
    raise RuntimeError("missing section %s" % prefix)


def state_names(text, registration_vas):
    wanted = set(registration_vas)
    result = {}
    for line in text.splitlines():
        fields = line.split()
        if len(fields) >= 3 and fields[0] in wanted:
            result[fields[0]] = (int(fields[1], 16), fields[2])
    missing = wanted - set(result)
    if missing:
        raise RuntimeError("missing state names for %s" % sorted(missing))
    return result


def operands(root):
    rows = []
    for filename in DECOMPILED:
        path = os.path.join(root, "docs", "decompiled", filename)
        function_va = int(filename[:-2], 16)
        for number, line in enumerate(read(path).splitlines(), 1):
            if any(token in line for token in OPERAND_TOKENS):
                rows.append((function_va, number, line.rstrip()))
    return rows


def analyze(root):
    vtables = read(os.path.join(root, "docs", "vtables.txt"))
    graph = read(os.path.join(root, "docs", "state_graph.txt"))
    blocks = read(os.path.join(root, "docs", "state_slot_blocks.txt"))
    names = read(os.path.join(root, "docs", "state_names.txt"))

    camera_va, camera_slots = vtable(vtables, "Camera")
    target_va, target_slots = vtable(vtables, "CameraTarget")

    graph_lines = section(graph, "=== CameraTarget ")
    constructor_va = int(re.search(r"ctor=(0x[0-9a-f]+)",
                                   graph_lines[0]).group(1), 16)
    edges = []
    for line in graph_lines:
        match = EDGE_RE.search(line)
        if match:
            edges.append((int(match.group("slot")), int(match.group("func"), 16),
                          int(match.group("call"), 16),
                          int(match.group("hash"), 16), match.group("name")))

    block_lines = section(blocks, "=== CameraTarget ")
    states = []
    for line in block_lines:
        match = SLOT_BLOCK_RE.search(line)
        if not match:
            continue
        slots = [int(value) for value in match.group("slots").split(",")]
        if len(slots) != 3:
            raise RuntimeError("unexpected CameraTarget slot block: %s" % line)
        states.append({
            "name": match.group("name"),
            "reg": match.group("reg"),
            "regname": match.group("regname"),
            "slots": slots,
            "thunks": match.group("thunks"),
            "complete": bool(match.group("complete")),
        })
    hashes = state_names(names, [state["reg"] for state in states])
    for state in states:
        state["hash"] = hashes[state["reg"]][0]

    return {
        "cameraVa": camera_va,
        "cameraSlots": camera_slots,
        "targetVa": target_va,
        "targetSlots": target_slots,
        "constructorVa": constructor_va,
        "graphLines": graph_lines,
        "blockLines": block_lines,
        "states": states,
        "edges": edges,
        "operands": operands(root),
    }


def write_report(path, data):
    lines = [
        "Generated by tools/analyze_camera_evidence.py",
        "Raw Camera/CameraTarget RTTI, state and decompiled operand evidence.",
        "No scroll speed, direction, timing, trigger or room destination is inferred.",
        "",
        "Camera vftable VA 0x%06x slots=%d" % (
            data["cameraVa"], len(data["cameraSlots"])),
    ]
    for slot, function in enumerate(data["cameraSlots"]):
        lines.append("  slot %-2d func=0x%06x" % (slot, function))
    lines.extend([
        "",
        "CameraTarget vftable VA 0x%06x ctor=0x%06x slots=%d" % (
            data["targetVa"], data["constructorVa"], len(data["targetSlots"])),
    ])
    for slot, function in enumerate(data["targetSlots"]):
        lines.append("  slot %-2d func=0x%06x" % (slot, function))
    lines.extend(["", "CameraTarget state graph (verbatim):"])
    lines.extend(data["graphLines"])
    lines.extend(["", "CameraTarget state slot blocks (verbatim):"])
    lines.extend(data["blockLines"])
    lines.extend(["", "CameraTarget state name hashes (verbatim):"])
    for state in data["states"]:
        lines.append("  %s 0x%08x %s" % (
            state["reg"], state["hash"], state["regname"]))
    lines.extend(["", "Decompiled Camera/MegaMan operand lines (verbatim):"])
    for function_va, number, text in data["operands"]:
        filename = "0x%06x.c" % function_va
        lines.append("  %s:%d %s" % (filename, number, text))
    lines.append("")
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def cpp_string(value):
    return '"%s"' % value.replace("\\", "\\\\").replace('"', '\\"')


def write_header(path, data):
    dats = sorted(set(re.findall(r"_DAT_([0-9a-f]+)",
                                 "\n".join(row[2] for row in data["operands"]))))
    lines = [
        "#pragma once",
        "",
        "#include \"mmu/core/Types.h\"",
        "",
        "#include <string_view>",
        "",
        "namespace mmu {",
        "",
        "// Generated by tools/analyze_camera_evidence.py from checked-in raw",
        "// decompilation and state reports. No gameplay meaning is assigned.",
        "struct CameraVtableSlot {",
        "    u16 slot;",
        "    u32 functionVa;",
        "};",
        "",
        "inline constexpr u32 kCameraVtableVa = 0x%06xu;" % data["cameraVa"],
        "inline constexpr CameraVtableSlot kCameraVtableSlots[] = {",
    ]
    for slot, function in enumerate(data["cameraSlots"]):
        lines.append("    {%du, 0x%06xu}," % (slot, function))
    lines.extend([
        "};",
        "",
        "inline constexpr u32 kCameraTargetVtableVa = 0x%06xu;" % data["targetVa"],
        "inline constexpr u32 kCameraTargetConstructorVa = 0x%06xu;" %
        data["constructorVa"],
        "inline constexpr CameraVtableSlot kCameraTargetVtableSlots[] = {",
    ])
    for slot, function in enumerate(data["targetSlots"]):
        lines.append("    {%du, 0x%06xu}," % (slot, function))
    lines.extend([
        "};",
        "",
        "struct CameraTargetState {",
        "    std::string_view name;",
        "    u32 registrationVa;",
        "    u32 hash;",
        "    u16 slot0;",
        "    u16 slot1;",
        "    u16 slot2;",
        "    std::string_view thunks;",
        "    bool complete;",
        "};",
        "",
        "inline constexpr CameraTargetState kCameraTargetStates[] = {",
    ])
    for state in data["states"]:
        lines.append(
            "    {%s, 0x%06xu, 0x%08xu, %du, %du, %du, %s, %s}," %
            (cpp_string(state["name"]), int(state["reg"], 16), state["hash"],
             state["slots"][0], state["slots"][1], state["slots"][2],
             cpp_string(state["thunks"]), "true" if state["complete"] else "false"))
    lines.extend([
        "};",
        "",
        "struct CameraTargetEdge {",
        "    u16 slot;",
        "    u32 functionVa;",
        "    u32 callVa;",
        "    u32 hash;",
        "    std::string_view stateName;",
        "};",
        "",
        "inline constexpr CameraTargetEdge kCameraTargetEdges[] = {",
    ])
    for slot, function, call, hash_value, name in data["edges"]:
        lines.append("    {%du, 0x%06xu, 0x%06xu, 0x%08xu, %s}," %
                     (slot, function, call, hash_value, cpp_string(name)))
    lines.extend([
        "};",
        "",
        "struct CameraOperandLine {",
        "    u32 functionVa;",
        "    u32 sourceLine;",
        "    std::string_view text;",
        "};",
        "",
    ])
    for dat in dats:
        lines.append("inline constexpr u32 kCameraDat%s = 0x%08xu;" %
                     (dat, int(dat, 16)))
    lines.extend([
        "",
        "inline constexpr CameraOperandLine kCameraOperandLines[] = {",
    ])
    for function_va, number, text in data["operands"]:
        lines.append("    {0x%06xu, %du, %s}," %
                     (function_va, number, cpp_string(text)))
    lines.extend([
        "};",
        "",
        "inline constexpr std::size_t kCameraVtableSlotCount =",
        "    sizeof(kCameraVtableSlots) / sizeof(kCameraVtableSlots[0]);",
        "inline constexpr std::size_t kCameraTargetVtableSlotCount =",
        "    sizeof(kCameraTargetVtableSlots) / sizeof(kCameraTargetVtableSlots[0]);",
        "inline constexpr std::size_t kCameraTargetStateCount =",
        "    sizeof(kCameraTargetStates) / sizeof(kCameraTargetStates[0]);",
        "inline constexpr std::size_t kCameraTargetEdgeCount =",
        "    sizeof(kCameraTargetEdges) / sizeof(kCameraTargetEdges[0]);",
        "inline constexpr std::size_t kCameraOperandLineCount =",
        "    sizeof(kCameraOperandLines) / sizeof(kCameraOperandLines[0]);",
        "",
        "inline const CameraOperandLine* cameraOperandLine(u32 functionVa,",
        "                                                  std::size_t index) {",
        "    std::size_t seen = 0;",
        "    for (const CameraOperandLine& row : kCameraOperandLines) {",
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
    print("camera_slots=%d camera_target_slots=%d target_states=%d "
          "target_edges=%d operands=%d" %
          (len(data["cameraSlots"]), len(data["targetSlots"]),
           len(data["states"]), len(data["edges"]), len(data["operands"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
