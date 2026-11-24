#!/usr/bin/env python3
"""Recover ordered per-class state graphs from MMU.exe.

The state registrar at 0x401370 is called from a class constructor once per
state.  The names are registered in the class's own order, and the constructor
finishes by calling setState(0x4405c0) with its initial state.  Combining those
constructor regions with the vtable callback scan from analyze_state_transitions
therefore gives a class with:

  * its registered state names in registration order,
  * the initial state the constructor actually selects,
  * every setState/global-cue call observable in its virtual callbacks.

A class that registers exactly one state is still evidence: the registration
name, the constructor's initial setState call, and any vtable callback edges
are all read directly from the binary. Those rows are retained rather than
discarded (the earlier two-state minimum silently dropped classes such as
EZBlock/Wait, SawPole/Patrol and CopterCannon/Move, plus the projectile
state machines they share).

Outputs:
  docs/state_graph.txt
  core/include/mmu/core/RegisteredStateGraph.h

The generated rows are evidence, not a claim about timing or unobserved
transitions.  A callback's target is recorded; the current state that led to it
is not invented.
"""

import bisect
import os
import struct
import sys

from analyze_game_flow import cstring, short_class
from analyze_rtti import PE
from analyze_state_names import (cpp_string, name_before, registration_sites,
                                 state_hash)
from analyze_state_transitions import scan_callback
from disasm import find_vftables

SET_STATE = 0x4405C0
MIN_REGISTRATIONS = 1


def text_regions(pe):
    """Split .text at int3 padding runs into likely function regions."""
    for name, va, _vsize, rawoff, rawsize in pe.sections:
        if name != ".text":
            continue
        base = pe.image_base + va
        data = pe.data[rawoff:rawoff + rawsize]
        bounds = []
        i = 0
        while i < len(data) - 1:
            if data[i] == 0xCC and data[i + 1] == 0xCC:
                j = i
                while j < len(data) and data[j] == 0xCC:
                    j += 1
                bounds.append(base + j)
                i = j
            else:
                i += 1
        return base, data, bounds
    raise RuntimeError("MMU.exe has no .text section")


def call_target(pe, site):
    off = pe.rva_to_off(site - pe.image_base)
    if off is None or pe.data[off] != 0xE8:
        return None
    rel = struct.unpack_from("<i", pe.data, off + 1)[0]
    return site + 5 + rel


def last_rdata_string(pe, site, span=0x90):
    """Last printable .rdata immediate immediately before a call site."""
    off = pe.rva_to_off(site - pe.image_base)
    if off is None:
        return None
    chunk = pe.data[max(0, off - span):off]
    found = None
    for i in range(len(chunk) - 3):
        candidate = struct.unpack_from("<I", chunk, i)[0]
        if pe.in_rdata(candidate - pe.image_base):
            text = cstring(pe, candidate, 1, 60)
            if text and all(ch.isprintable() for ch in text):
                found = text
    return found


def constructor_regions(pe, base, data, bounds, registrations, vftable_va):
    """Find every function region that writes a vftable and registers states.

    A class can have more than one constructor with the same vftable. The
    secondary constructor is separate evidence: it registers its own states and
    calls its own initial setState, so it is retained instead of being silently
    replaced by the longest region.
    """
    needle = struct.pack("<I", vftable_va)
    found = {}
    offset = 0
    while True:
        index = data.find(needle, offset)
        if index < 0:
            break
        site = base + index
        offset = index + 1
        region = bisect.bisect_right(bounds, site) - 1
        if region < 0 or region + 1 >= len(bounds):
            continue
        start, end = bounds[region], bounds[region + 1]
        names = [name for reg, name in registrations if start <= reg < end]
        if len(names) < MIN_REGISTRATIONS:
            continue
        if start not in found:
            found[start] = (start, end, names, site)
    return [found[start] for start in sorted(found)]


def scan_set_state_calls(pe, start, end):
    targets = []
    for site in range(start, end - 5):
        if call_target(pe, site) == SET_STATE:
            text = last_rdata_string(pe, site)
            if text:
                targets.append((site, text))
    return targets


def collect(pe):
    base, data, bounds = text_regions(pe)
    registrations = sorted(
        (site, name) for site in registration_sites(pe)
        for name in [name_before(pe, site)] if name)
    tables = find_vftables(pe)
    classes = []
    seen = set()
    for class_name, entries in tables.items():
        simple = short_class(class_name)
        if "@mmu" in simple:
            continue
        for vft, funcs in entries:
            vftable_va = pe.image_base + vft
            regions = constructor_regions(pe, base, data, bounds,
                                          registrations, vftable_va)
            if not regions:
                continue
            # The vtable callbacks belong to the class, not to a particular
            # constructor, so attach them once to the region with the most
            # registered states (the first on ties, as before).
            primary = max(regions, key=lambda region: len(region[2]))
            for start, end, names, write_site in regions:
                key = (simple, start)
                if key in seen:
                    continue
                seen.add(key)
                setters = scan_set_state_calls(pe, start, end)
                initial = setters[-1] if setters else None
                edges = []
                edge_seen = set()
                if start == primary[0]:
                    for slot, func_va in enumerate(funcs):
                        if func_va < pe.image_base:
                            continue
                        for call_va, kind, target in scan_callback(pe, func_va):
                            edge_key = (slot, call_va, kind, target)
                            if edge_key in edge_seen:
                                continue
                            edge_seen.add(edge_key)
                            edges.append({
                                "class": simple,
                                "vftable": vftable_va,
                                "constructor": start,
                                "slot": slot,
                                "function": func_va,
                                "call": call_va,
                                "kind": kind,
                                "target": target,
                                "hash": state_hash(target),
                            })
                classes.append({
                    "class": simple,
                    "vftable": vftable_va,
                    "constructor": start,
                    "vtableWrite": write_site,
                    "states": names,
                    "initial": initial[1] if initial else "",
                    "initialCall": initial[0] if initial else 0,
                    "edges": edges,
                })
    classes.sort(key=lambda cls: (cls["class"], cls["vftable"],
                                  cls["constructor"]))
    return classes


def write_report(classes, path):
    state_count = sum(len(cls["states"]) for cls in classes)
    edge_count = sum(len(cls["edges"]) for cls in classes)
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_state_graph.py\n")
        out.write("Source: 0x401370 registrations in class constructors and\n")
        out.write("setState 0x4405c0 / cue setter 0x4026b0 calls in vtable callbacks.\n")
        out.write("Evidence only: no timing, unobserved transition, or destination "
                  "is inferred.\n")
        out.write("classes=%d states=%d edges=%d\n\n" %
                  (len(classes), state_count, edge_count))
        for cls in classes:
            out.write("=== %s vftable=0x%x ctor=0x%x ===\n" %
                      (cls["class"], cls["vftable"], cls["constructor"]))
            out.write("  states (%d): %s\n" %
                      (len(cls["states"]), ", ".join(cls["states"])))
            out.write("  initial: %s (setState at 0x%x)\n" %
                      (cls["initial"] or "?", cls["initialCall"]))
            for edge in cls["edges"]:
                out.write("  slot %-2d func=0x%06x call=0x%06x %-8s "
                          "hash=0x%08x %s\n" %
                          (edge["slot"], edge["function"], edge["call"],
                           edge["kind"], edge["hash"], edge["target"]))
            out.write("\n")


def write_header(classes, path):
    with open(path, "w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write("#include \"mmu/core/Types.h\"\n\n")
        out.write("#include <cstddef>\n")
        out.write("#include <string_view>\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_state_graph.py. Each row is one state\n")
        out.write("// registered by a class constructor, in the binary's own order.\n")
        out.write("struct RegisteredStateGraphRow {\n")
        out.write("    std::string_view className;\n")
        out.write("    std::string_view stateName;\n")
        out.write("    u32 targetHash;\n")
        out.write("    u32 vtableVa;\n")
        out.write("    u32 constructorVa;\n")
        out.write("    u16 order;\n")
        out.write("    u16 stateCount;\n")
        out.write("    u8 isInitial;\n")
        out.write("    u8 reserved;\n")
        out.write("};\n\n")
        out.write("inline constexpr RegisteredStateGraphRow kRegisteredStateGraph[] = {\n")
        for cls in classes:
            for order, state in enumerate(cls["states"]):
                out.write("    {\"%s\", \"%s\", 0x%08xu, 0x%06xu, 0x%06xu, %du, %du, %du, 0u},\n" % (
                    cpp_string(cls["class"]), cpp_string(state),
                    state_hash(state), cls["vftable"], cls["constructor"],
                    order, len(cls["states"]),
                    1 if state == cls["initial"] else 0))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kRegisteredStateGraphCount =\n")
        out.write("    sizeof(kRegisteredStateGraph) / sizeof(kRegisteredStateGraph[0]);\n\n")
        out.write("// Observed setter calls in the class's virtual callbacks. A row records\n")
        out.write("// the target and callback, never an invented source state.\n")
        out.write("struct RegisteredStateGraphEdge {\n")
        out.write("    std::string_view className;\n")
        out.write("    std::string_view targetName;\n")
        out.write("    u32 targetHash;\n")
        out.write("    u32 functionVa;\n")
        out.write("    u32 callVa;\n")
        out.write("    u16 vtableSlot;\n")
        out.write("    u8 flags; // 1 = setState, 2 = cue\n")
        out.write("    u8 reserved;\n")
        out.write("};\n\n")
        out.write("inline constexpr RegisteredStateGraphEdge kRegisteredStateGraphEdges[] = {\n")
        for cls in classes:
            for edge in cls["edges"]:
                out.write("    {\"%s\", \"%s\", 0x%08xu, 0x%06xu, 0x%06xu, %du, %du, 0u},\n" % (
                    cpp_string(cls["class"]), cpp_string(edge["target"]),
                    edge["hash"], edge["function"], edge["call"],
                    edge["slot"], 1 if edge["kind"] == "setState" else 2))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kRegisteredStateGraphEdgeCount =\n")
        out.write("    sizeof(kRegisteredStateGraphEdges) / "
                  "sizeof(kRegisteredStateGraphEdges[0]);\n\n")
        out.write("inline std::size_t stateGraphRegionCount() {\n")
        out.write("    std::size_t count = 0;\n")
        out.write("    for (const RegisteredStateGraphRow& row : kRegisteredStateGraph)\n")
        out.write("        if (row.order == 0) ++count;\n")
        out.write("    return count;\n")
        out.write("}\n\n")
        out.write("inline std::size_t stateGraphClassCount() {\n")
        out.write("    std::size_t count = 0;\n")
        out.write("    for (std::size_t i = 0; i < kRegisteredStateGraphCount; ++i) {\n")
        out.write("        bool seen = false;\n")
        out.write("        for (std::size_t j = 0; j < i; ++j)\n")
        out.write("            if (kRegisteredStateGraph[j].className ==\n")
        out.write("                kRegisteredStateGraph[i].className) {\n")
        out.write("                seen = true;\n")
        out.write("                break;\n")
        out.write("            }\n")
        out.write("        if (!seen) ++count;\n")
        out.write("    }\n")
        out.write("    return count;\n")
        out.write("}\n\n")
        out.write("inline const char* recoveredInitialState(std::string_view className) {\n")
        out.write("    for (const RegisteredStateGraphRow& row : kRegisteredStateGraph)\n")
        out.write("        if (row.className == className && row.isInitial != 0)\n")
        out.write("            return row.stateName.data();\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline bool graphOrdersState(std::string_view className,\n")
        out.write("                              std::string_view stateName) {\n")
        out.write("    for (const RegisteredStateGraphRow& row : kRegisteredStateGraph)\n")
        out.write("        if (row.className == className && row.stateName == stateName)\n")
        out.write("            return true;\n")
        out.write("    return false;\n")
        out.write("}\n\n")
        out.write("inline std::size_t graphStateCount(std::string_view className) {\n")
        out.write("    for (const RegisteredStateGraphRow& row : kRegisteredStateGraph)\n")
        out.write("        if (row.className == className) return row.stateCount;\n")
        out.write("    return 0;\n")
        out.write("}\n\n")
        out.write("inline std::size_t graphEdgeCount(std::string_view className) {\n")
        out.write("    std::size_t count = 0;\n")
        out.write("    for (const RegisteredStateGraphEdge& edge : kRegisteredStateGraphEdges)\n")
        out.write("        if (edge.className == className) ++count;\n")
        out.write("    return count;\n")
        out.write("}\n\n")
        out.write("} // namespace mmu\n")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    report = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        root, "docs", "state_graph.txt")
    header = sys.argv[3] if len(sys.argv) > 3 else os.path.join(
        root, "core", "include", "mmu", "core", "RegisteredStateGraph.h")
    pe = PE(open(sys.argv[1], "rb").read())
    classes = collect(pe)
    write_report(classes, report)
    write_header(classes, header)
    print("classes=%d states=%d edges=%d report=%s header=%s" % (
        len(classes),
        sum(len(cls["states"]) for cls in classes),
        sum(len(cls["edges"]) for cls in classes), report, header))
    return 0


if __name__ == "__main__":
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    raise SystemExit(main())
