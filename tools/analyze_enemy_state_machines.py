#!/usr/bin/env python3
"""Join raw-evidence reports into enemy state machines.

The caller map in docs/boss_damage.txt (and its generated header) identifies
thirteen RTTI-verified owners of the raw damage helpers at 0x420380/0x420690.
Two of them are resolved by unique RTTI vftable membership. The state
graph in docs/state_graph.txt records each owner's constructor-registered
states, initial setState call, and setState/cue setter edges. This tool joins
those two reports by exact class name and vftable address.

It also reads MMU.exe directly to recover the three-slot virtual block that
each constructor assigns to a registered state: immediately before each
0x401370 registration call, the constructor materialises three text thunks,
and each thunk is a `jmp dword ptr [eax + 0xNN]`. A graph edge's slot is
attributed to the state whose block contains it; slots outside all blocks
(e.g. base-class slot 3) are left unattributed. No timing, triggers, gameplay
meaning, or unobserved transitions are inferred.

Usage:
  analyze_enemy_state_machines.py MMU.exe root report.txt header.h

Outputs:
  docs/enemy_state_machines.txt
  core/include/mmu/core/EnemyStateGraph.h
"""

import bisect
import os
import re
import sys

from analyze_rtti import PE
from analyze_state_graph import text_regions
from analyze_state_names import name_before, registration_sites

from capstone import Cs, CS_ARCH_X86, CS_MODE_32

MD = Cs(CS_ARCH_X86, CS_MODE_32)
MOV_IMMEDIATE_RE = re.compile(
    r"dword ptr \[ebp - 0x[0-9a-f]+\], (0x[0-9a-f]+)")
JMP_SLOT_RE = re.compile(r"\[eax \+ (?:0x)?([0-9a-f]+)\]")
STATE_LOOKBACK = 64

SECTION_RE = re.compile(r"^(\S+) vftable=(0x[0-9a-f]+) "
                        r"ctor=(0x[0-9a-f]+) ===$")
STATES_RE = re.compile(r"^  states \((\d+)\): (.+)$")
INITIAL_RE = re.compile(r"^  initial: (\S+) \(setState at (0x[0-9a-f]+)\)$")
EDGE_RE = re.compile(
    r"^  slot (\d+)\s+func=(0x[0-9a-f]+)\s+call=(0x[0-9a-f]+)\s+"
    r"(setState|cue)\s+hash=(0x[0-9a-f]+)\s+(.+)$")
CALLER_RE = re.compile(
    r'^\s*\{"(\w+)", (0x[0-9a-f]+)u, (0x[0-9a-f]+)u, '
    r'(0x[0-9a-f]+)u, (\d+)u, (0x[0-9a-f]+)u, "([^"]*)"\},$',
    re.MULTILINE)


def read(path):
    with open(path, encoding="utf-8", errors="replace") as handle:
        return handle.read()


def parse_state_graph(text):
    machines = {}
    for part in re.split(r"(?m)^=== ", text)[1:]:
        lines = part.rstrip().splitlines()
        header = SECTION_RE.fullmatch(lines[0])
        if not header:
            continue
        name, vftable, ctor = header.groups()
        states = initial = None
        edges = []
        for line in lines[1:]:
            match = STATES_RE.fullmatch(line)
            if match:
                states = [item.strip() for item in match.group(2).split(",")]
                if len(states) != int(match.group(1)):
                    raise RuntimeError("state count mismatch for %s" % name)
                continue
            match = INITIAL_RE.fullmatch(line)
            if match:
                initial = (match.group(1), int(match.group(2), 16))
                continue
            match = EDGE_RE.fullmatch(line)
            if match:
                slot, func, call, kind, target_hash, target = match.groups()
                edges.append({
                    "slot": int(slot),
                    "func": int(func, 16),
                    "call": int(call, 16),
                    "cue": kind == "cue",
                    "hash": int(target_hash, 16),
                    "target": target.strip(),
                })
        if states is None or initial is None:
            raise RuntimeError("missing states/initial for %s" % name)
        machines[name] = {
            "vftable": int(vftable, 16),
            "ctor": int(ctor, 16),
            "states": states,
            "initial": initial[0],
            "initialVa": initial[1],
            "edges": edges,
        }
    return machines


def parse_damage_callers(text):
    callers = {}
    for match in CALLER_RE.finditer(text):
        name, call, handler, vftable, slot, bits, resolution = match.groups()
        callers.setdefault(name, []).append({
            "call": int(call, 16),
            "handler": int(handler, 16),
            "vftable": int(vftable, 16),
            "slot": int(slot),
            "bits": int(bits, 16),
            "resolution": resolution,
        })
    return callers


def trampoline_slot(pe, va):
    """Read the vtable slot from a `jmp dword ptr [eax + 0xNN]` thunk."""
    off = pe.rva_to_off(va - pe.image_base)
    if off is None:
        return None
    for ins in list(MD.disasm(pe.data[off:off + 16], va))[:3]:
        if ins.mnemonic != "jmp":
            continue
        match = JMP_SLOT_RE.search(ins.op_str)
        if match:
            return int(match.group(1), 16) // 4
    return None


def constructor_state_slots(pe, registrations, ctor):
    """Recover the three thunk slots assigned before each registration call."""
    _base, data, bounds = text_regions(pe)
    index = bisect.bisect_right(bounds, ctor) - 1
    if index < 0 or index + 1 >= len(bounds):
        raise RuntimeError("constructor 0x%x outside .text" % ctor)
    start, end = bounds[index], bounds[index + 1]
    sites = [(site, name) for site, name in registrations
             if start <= site < end]
    rows = list(MD.disasm(data[start - _base:end - _base], start))
    by_address = {row.address: i for i, row in enumerate(rows)}
    slots = {}
    for site, name in sites:
        row_index = by_address.get(site)
        if row_index is None:
            raise RuntimeError("registration 0x%x not disassembled" % site)
        values = []
        for row in rows[max(0, row_index - STATE_LOOKBACK):row_index]:
            match = MOV_IMMEDIATE_RE.fullmatch(row.op_str)
            if row.mnemonic != "mov" or not match:
                continue
            va = int(match.group(1), 16)
            if pe.rva_to_off(va - pe.image_base) is None:
                continue
            if va not in values:
                values.append(va)
        found = []
        for va in values[-3:]:
            slot = trampoline_slot(pe, va)
            if slot is not None and slot not in found:
                found.append(slot)
        found.sort()
        if len(found) != 3:
            raise RuntimeError("state %s has %d thunk slots (expected 3)" %
                               (name, len(found)))
        slots[name] = (found[0], found[-1])
    return slots


def build(root, exe):
    graphs = parse_state_graph(read(os.path.join(root, "docs",
                                                 "state_graph.txt")))
    callers = parse_damage_callers(read(os.path.join(
        root, "core", "include", "mmu", "core", "BossDamageParams.h")))
    pe = PE(open(exe, "rb").read())
    registrations = sorted(
        (site, name) for site in registration_sites(pe)
        for name in [name_before(pe, site)] if name)
    joined = []
    for name, rows in callers.items():
        if name not in graphs:
            raise RuntimeError("state graph missing for damage owner %s" % name)
        for row in rows:
            graph = graphs[name]
            if graph["vftable"] != row["vftable"]:
                raise RuntimeError("vftable mismatch for %s" % name)
            slots = constructor_state_slots(pe, registrations, graph["ctor"])
            if set(slots) != set(graph["states"]):
                raise RuntimeError("state slot names differ for %s" % name)
            machine = dict(graph, name=name, **row)
            machine["stateSlots"] = [
                (state, slots[state][0], slots[state][1])
                for state in graph["states"]]
            for edge in machine["edges"]:
                edge["ownerState"] = next(
                    (state for state, first, last in machine["stateSlots"]
                     if first <= edge["slot"] <= last), "")
            joined.append(machine)
    if len(joined) != 13:
        raise RuntimeError("expected 13 joined machines, got %d" %
                           len(joined))
    return joined


def c_ident(name):
    return "kEnemyStateGraph_" + "".join(
        char if char.isalnum() else "_" for char in name)


def cpp_string(value):
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def write_report(path, machines):
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_enemy_state_machines.py\n")
        out.write("Join keys: exact RTTI class name and vftable VA. No semantics "
                  "are assigned to raw states, cues, hashes, or result bits.\n")
        out.write("classes=%d states=%d edges=%d\n\n" %
                  (len(machines),
                   sum(len(m["states"]) for m in machines),
                   sum(len(m["edges"]) for m in machines)))
        for machine in machines:
            out.write("=== %s vftable=0x%06x ctor=0x%06x ===\n" %
                      (machine["name"], machine["vftable"], machine["ctor"]))
            out.write("  damage call=0x%06x handler=0x%06x result_bits=0x%08x "
                      "vft_slot=%d source=%s\n" %
                      (machine["call"], machine["handler"], machine["bits"],
                       machine["slot"], machine["resolution"]))
            out.write("  states: %s\n" % ", ".join(machine["states"]))
            out.write("  initial: %s (setState at 0x%06x)\n" %
                      (machine["initial"], machine["initialVa"]))
            for state, first, last in machine["stateSlots"]:
                out.write("  state %-14s slots %d..%d\n" %
                          (state, first, last))
            for edge in machine["edges"]:
                out.write("  slot %-2d func=0x%06x call=0x%06x %-8s "
                          "hash=0x%08x %-14s owner=%s\n" %
                          (edge["slot"], edge["func"], edge["call"],
                           "cue" if edge["cue"] else "setState",
                           edge["hash"], edge["target"],
                           edge["ownerState"] or "(none)"))
            out.write("\n")


def write_header(path, machines):
    lines = [
        "#pragma once",
        "",
        "#include \"mmu/core/Types.h\"",
        "",
        "#include <string_view>",
        "",
        "namespace mmu {",
        "",
        "// Generated by tools/analyze_enemy_state_machines.py from",
        "// docs/boss_damage.txt, docs/state_graph.txt and MMU.exe.",
        "// Raw names, VAs, hashes, slots and result bits only; no gameplay",
        "// labels or timing. Each registered state owns three consecutive",
        "// constructor thunks; ownerState is empty for slots outside them.",
        "struct EnemyStateGraphStateSlots {",
        "    std::string_view stateName;",
        "    u16 firstSlot;",
        "    u16 lastSlot;",
        "};",
        "",
        "struct EnemyStateGraphEdge {",
        "    u16 vtableSlot;",
        "    u32 functionVa;",
        "    u32 callVa;",
        "    bool isCue;",
        "    u32 targetHash;",
        "    std::string_view targetName;",
        "    std::string_view ownerState;",
        "};",
        "",
        "struct EnemyStateGraphMachine {",
        "    std::string_view className;",
        "    u32 vftableVa;",
        "    u32 constructorVa;",
        "    u32 damageCallVa;",
        "    u32 damageHandlerVa;",
        "    u32 damageResultCodeBits;",
        "    u16 damageVtableSlot;",
        "    std::string_view damageResolution;",
        "    std::string_view initialState;",
        "    u32 initialSetStateVa;",
        "    const std::string_view* states;",
        "    u16 stateCount;",
        "    const EnemyStateGraphStateSlots* stateSlots;",
        "    u16 stateSlotCount;",
        "    const EnemyStateGraphEdge* edges;",
        "    u16 edgeCount;",
        "};",
        "",
    ]
    for machine in machines:
        ident = c_ident(machine["name"])
        lines.extend([
            "inline constexpr std::string_view %s_States[] = {" % ident,
        ])
        lines.extend("    %s," % cpp_string(state) for state in machine["states"])
        lines.append("};")
        lines.append("inline constexpr EnemyStateGraphStateSlots %s_Slots[] = {" %
                     ident)
        for state, first, last in machine["stateSlots"]:
            lines.append("    {%s, %du, %du}," %
                         (cpp_string(state), first, last))
        lines.append("};")
        if machine["edges"]:
            lines.append("inline constexpr EnemyStateGraphEdge %s_Edges[] = {" %
                         ident)
            for edge in machine["edges"]:
                lines.append(
                    "    {%du, 0x%06xu, 0x%06xu, %s, 0x%08xu, %s, %s}," %
                    (edge["slot"], edge["func"], edge["call"],
                     "true" if edge["cue"] else "false",
                     edge["hash"], cpp_string(edge["target"]),
                     cpp_string(edge["ownerState"])))
            lines.append("};")
        lines.append("")

    lines.extend([
        "inline constexpr EnemyStateGraphMachine kEnemyStateGraphMachines[] = {",
    ])
    for machine in machines:
        ident = c_ident(machine["name"])
        edges = "%s_Edges" % ident if machine["edges"] else "nullptr"
        lines.append(
            "    {%s, 0x%06xu, 0x%06xu, 0x%06xu, 0x%06xu, 0x%08xu, "
            "%du, %s, %s, 0x%06xu, %s_States, %du, %s_Slots, %du, %s, %du}," %
            (cpp_string(machine["name"]), machine["vftable"],
             machine["ctor"], machine["call"], machine["handler"],
             machine["bits"], machine["slot"],
             cpp_string(machine["resolution"]),
             cpp_string(machine["initial"]), machine["initialVa"],
             ident, len(machine["states"]),
             ident, len(machine["stateSlots"]), edges,
             len(machine["edges"])))
    lines.extend([
        "};",
        "",
        "inline const EnemyStateGraphMachine* enemyStateGraphMachine(",
        "        std::string_view className) {",
        "    for (const EnemyStateGraphMachine& machine : kEnemyStateGraphMachines)",
        "        if (machine.className == className) return &machine;",
        "    return nullptr;",
        "}",
        "",
        "inline std::string_view enemyStateGraphInitialState(",
        "        std::string_view className) {",
        "    const EnemyStateGraphMachine* machine = enemyStateGraphMachine(className);",
        "    return machine ? machine->initialState : std::string_view();",
        "}",
        "",
        "inline std::string_view enemyStateGraphSlotOwner(",
        "        std::string_view className, u16 slot) {",
        "    const EnemyStateGraphMachine* machine = enemyStateGraphMachine(className);",
        "    if (machine == nullptr) return std::string_view();",
        "    for (u32 index = 0; index < machine->stateSlotCount; ++index) {",
        "        const EnemyStateGraphStateSlots& entry = machine->stateSlots[index];",
        "        if (entry.firstSlot <= slot && slot <= entry.lastSlot)",
        "            return entry.stateName;",
        "    }",
        "    return std::string_view();",
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
    exe, root, report, header = sys.argv[1:]
    machines = build(root, exe)
    write_report(report, machines)
    write_header(header, machines)
    print("classes=%d states=%d edges=%d" %
          (len(machines),
           sum(len(m["states"]) for m in machines),
           sum(len(m["edges"]) for m in machines)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
