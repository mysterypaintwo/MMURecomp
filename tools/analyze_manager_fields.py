#!/usr/bin/env python3
"""Recover raw global-manager field evidence from decompiled MMU functions.

The global at 0x6c55a0 is the menu/flow manager. This analyzer records only
literal or structurally observable facts from Ghidra output: literal writes in
the constructor and immediate path, tiny accessor helpers with their exact byte
offset and access size, and direct field accesses in the known consumers. A
second global object at 0x6c55f8 is included where completion evidence uses it,
but every row identifies its own object global.

No field is named and no gameplay semantics are inferred from an offset, size or
helper call.

Outputs:
  docs/manager_fields.txt
  core/include/mmu/core/ManagerFieldTable.h
"""

import os
import re
import sys

from analyze_activity_masks import DECOMPILED_DIR, PROJECT_ROOT
from analyze_level_setup import direct_call_sites
from analyze_rtti import PE


REPORT_PATH = os.path.join(PROJECT_ROOT, "docs", "manager_fields.txt")
HEADER_PATH = os.path.join(
    PROJECT_ROOT, "core", "include", "mmu", "core", "ManagerFieldTable.h")

MANAGER_GLOBAL_VA = 0x6C55A0
COMPLETION_OBJECT_GLOBAL_VA = 0x6C55F8

# FUN_0050bbb0 constructs the object that its caller stores into 0x6c55a0.
# FUN_0050ba40 is excluded: its callers pass assorted local objects, not the
# global manager, even though the helper also writes 0x80 at offset zero.
LITERAL_WRITE_SOURCES = {
    0x50BBB0: MANAGER_GLOBAL_VA,
}

CONSUMER_SOURCES = {
    0x50F7D0: MANAGER_GLOBAL_VA,
    0x511350: MANAGER_GLOBAL_VA,
    0x507FD0: MANAGER_GLOBAL_VA,
    0x4308A0: COMPLETION_OBJECT_GLOBAL_VA,
    0x4308C0: COMPLETION_OBJECT_GLOBAL_VA,
}

FORWARDED_ACCESSORS = {
    0x4033E0: 0x4032E0,
    0x403400: 0x403300,
    0x403D40: 0x403CC0,
}

ACCESSOR_FUNCTIONS = [
    0x403220,
    0x4032E0,
    0x403300,
    0x4033E0,
    0x403400,
    0x403CC0,
    0x403D40,
    0x507E90,
    0x507EC0,
]

PARAM_INDEX_RE = re.compile(r"param_1\[(0x[0-9a-f]+|\d+)\]", re.I)
ROOT_ACCESS_RE = re.compile(r"\*param_1\b")
PARAM_PTR_RE = re.compile(
    r"\*\((\w+)\s*\*+\)\s*\(\s*(?:\(int\))?\s*param_1\s*\+\s*"
    r"(0x[0-9a-f]+|\d+)\s*\)", re.I)
ROOT_WRITE_RE = re.compile(r"\*param_1\s*=\s*(0x[0-9a-f]+|\d+)\s*;", re.I)
LITERAL_RE = re.compile(r"(0x[0-9a-f]+|\d+)\s*;", re.I)
CALL_RE = re.compile(r"\b(FUN_[0-9a-f]+)\s*\(", re.I)

SIZE_BY_TYPE = {
    "undefined1": 1,
    "undefined2": 2,
    "undefined4": 4,
    "bool": 1,
    "char": 1,
    "uchar": 1,
    "short": 2,
    "ushort": 2,
    "int": 4,
    "uint": 4,
    "float": 4,
    "uint4": 4,
}


def decoder_path(va):
    return os.path.join(DECOMPILED_DIR, "0x%06x.c" % va)


def read_function_lines(va):
    path = decoder_path(va)
    if not os.path.isfile(path):
        raise RuntimeError("missing decompilation: " + path)
    return open(path, encoding="latin1").read().splitlines()


def iter_body_lines(va):
    """Yield (line number, line) after the decompiled function's opening brace."""
    lines = read_function_lines(va)
    start = next((i for i, line in enumerate(lines) if "{" in line), 0)
    for line_number in range(start + 1, len(lines) + 1):
        yield line_number, lines[line_number - 1]


def access_size(type_name):
    return SIZE_BY_TYPE.get(type_name, 0)


def assignment_position(line, expression):
    """Return the index of a real '=' after expression, or -1 for no write."""
    start = line.find(expression)
    if start < 0:
        return -1
    position = line.find("=", start + len(expression))
    while position >= 0 and position + 1 < len(line) and line[position + 1] == "=":
        position = line.find("=", position + 2)
    if position < 0:
        return -1
    return position


def access_kind(line, expression):
    return "Write" if assignment_position(line, expression) >= 0 else "Read"


def collect_field_accesses(function_va, object_va):
    """Return a map of (offset, size, kind) -> [count, first_line]."""
    result = {}
    for line_number, line in iter_body_lines(function_va):
        for match in ROOT_ACCESS_RE.finditer(line):
            key = (0, 4, access_kind(line, match.group(0)))
            entry = result.setdefault(key, [0, line_number])
            entry[0] += 1
        for match in PARAM_PTR_RE.finditer(line):
            size = access_size(match.group(1))
            if size == 0:
                continue
            offset = int(match.group(2), 0)
            key = (offset, size, access_kind(line, match.group(0)))
            entry = result.setdefault(key, [0, line_number])
            entry[0] += 1
        for match in PARAM_INDEX_RE.finditer(line):
            offset = int(match.group(1), 0) * 4
            key = (offset, 4, access_kind(line, match.group(0)))
            entry = result.setdefault(key, [0, line_number])
            entry[0] += 1
    return result


def collect_literal_writes():
    rows = []
    for function_va, object_va in LITERAL_WRITE_SOURCES.items():
        lines = read_function_lines(function_va)
        for line_number, line in enumerate(lines, 1):
            for match in ROOT_WRITE_RE.finditer(line):
                value = int(match.group(1), 0)
                rows.append({
                    "functionVa": function_va,
                    "objectGlobalVa": object_va,
                    "offset": 0,
                    "size": 4,
                    "value": value,
                    "line": line_number,
                })
            for match in PARAM_INDEX_RE.finditer(line):
                tail = line[match.end():]
                value_match = re.match(r"\s*=\s*", tail)
                if value_match is None:
                    continue
                literal = LITERAL_RE.match(tail[value_match.end():])
                if literal is None:
                    continue
                rows.append({
                    "functionVa": function_va,
                    "objectGlobalVa": object_va,
                    "offset": int(match.group(1), 0) * 4,
                    "size": 4,
                    "value": int(literal.group(1), 0),
                    "line": line_number,
                })
            for match in PARAM_PTR_RE.finditer(line):
                size = access_size(match.group(1))
                if size == 0:
                    continue
                tail = line[match.end():]
                value_match = re.match(r"\s*=\s*", tail)
                if value_match is None:
                    continue
                literal = LITERAL_RE.match(tail[value_match.end():])
                if literal is None:
                    continue
                rows.append({
                    "functionVa": function_va,
                    "objectGlobalVa": object_va,
                    "offset": int(match.group(2), 0),
                    "size": size,
                    "value": int(literal.group(1), 0),
                    "line": line_number,
                })
    rows.sort(key=lambda row: (row["functionVa"], row["line"], row["offset"]))
    return rows


def collect_accessors(pe):
    direct = {}
    for function_va in ACCESSOR_FUNCTIONS:
        accesses = collect_field_accesses(function_va, MANAGER_GLOBAL_VA)
        if accesses:
            direct[function_va] = accesses
    rows = []
    for function_va in ACCESSOR_FUNCTIONS:
        forwarded = FORWARDED_ACCESSORS.get(function_va, 0)
        accesses = direct.get(function_va)
        if accesses is None and forwarded != 0:
            forwarded_rows = [row for row in rows
                              if row["functionVa"] == forwarded]
            accesses = {
                (row["offset"], row["size"], row["kind"]): [row["callCount"],
                                                            row["line"]]
                for row in forwarded_rows
            }
        if accesses is None:
            raise RuntimeError("no field access in accessor 0x%x" % function_va)
        call_count = len(direct_call_sites(pe, function_va))
        for (offset, size, kind), (count, line) in sorted(accesses.items()):
            rows.append({
                "functionVa": function_va,
                "objectGlobalVa": MANAGER_GLOBAL_VA,
                "offset": offset,
                "size": size,
                "kind": kind,
                "forwardedVa": forwarded,
                "callCount": call_count,
                "line": line,
            })
    return rows


def collect_consumers():
    rows = []
    for function_va, object_va in CONSUMER_SOURCES.items():
        accesses = collect_field_accesses(function_va, object_va)
        for (offset, size, kind), (count, line) in sorted(accesses.items()):
            rows.append({
                "functionVa": function_va,
                "objectGlobalVa": object_va,
                "offset": offset,
                "size": size,
                "kind": kind,
                "count": count,
                "line": line,
            })
    return rows


def write_report(writes, accessors, accesses):
    with open(REPORT_PATH, "w", encoding="latin1") as out:
        out.write("Generated by tools/analyze_manager_fields.py\n")
        out.write("Source: docs/decompiled/<function>.c and direct calls in MMU.exe.\n")
        out.write("manager=[0x%06x] completion_object=[0x%06x]\n" %
                  (MANAGER_GLOBAL_VA, COMPLETION_OBJECT_GLOBAL_VA))
        out.write("literal_writes=%d accessors=%d consumer_offsets=%d\n\n" %
                  (len(writes), len(accessors), len(accesses)))
        out.write("=== literal writes ===\n")
        for row in writes:
            out.write("func=0x%06x line=%4u obj=0x%06x +0x%03x size=%u "
                      "value=0x%x\n" %
                      (row["functionVa"], row["line"], row["objectGlobalVa"],
                       row["offset"], row["size"], row["value"]))
        out.write("\n=== accessor helpers ===\n")
        for row in accessors:
            forwarded = (" forward=0x%06x" % row["forwardedVa"]
                         if row["forwardedVa"] else "")
            out.write("func=0x%06x obj=0x%06x +0x%03x size=%u kind=%-5s "
                      "calls=%u%s\n" %
                      (row["functionVa"], row["objectGlobalVa"], row["offset"],
                       row["size"], row["kind"], row["callCount"], forwarded))
        out.write("\n=== observed consumer accesses ===\n")
        for row in accesses:
            out.write("func=0x%06x line=%4u obj=0x%06x +0x%03x size=%u "
                      "kind=%-5s count=%u\n" %
                      (row["functionVa"], row["line"], row["objectGlobalVa"],
                       row["offset"], row["size"], row["kind"], row["count"]))


def write_header(writes, accessors, accesses):
    with open(HEADER_PATH, "w", encoding="latin1") as out:
        out.write("#pragma once\n\n")
        out.write('#include "mmu/core/Types.h"\n\n')
        out.write("#include <cstddef>\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_manager_fields.py.\n")
        out.write("// Every row is a raw offset, size, literal or call-count observation.\n")
        out.write("// No field name or gameplay meaning is assigned to an offset.\n")
        out.write("inline constexpr u32 kManagerGlobalVa = 0x%06xu;\n" %
                  MANAGER_GLOBAL_VA)
        out.write("inline constexpr u32 kManagerCompletionObjectGlobalVa = 0x%06xu;\n\n" %
                  COMPLETION_OBJECT_GLOBAL_VA)
        out.write("enum class ManagerFieldAccessKind : u8 { Read, Write };\n\n")
        out.write("struct ManagerLiteralWrite {\n")
        out.write("    u32 functionVa;\n")
        out.write("    u32 objectGlobalVa;\n")
        out.write("    u16 offset;\n")
        out.write("    u8 size;\n")
        out.write("    u16 lineNumber;\n")
        out.write("    u32 value;\n")
        out.write("};\n\n")
        out.write("struct ManagerAccessor {\n")
        out.write("    u32 functionVa;\n")
        out.write("    u32 objectGlobalVa;\n")
        out.write("    u16 offset;\n")
        out.write("    u8 size;\n")
        out.write("    ManagerFieldAccessKind kind;\n")
        out.write("    u16 callCount;\n")
        out.write("    u32 forwardedVa;\n")
        out.write("    u16 lineNumber;\n")
        out.write("};\n\n")
        out.write("struct ManagerFieldAccess {\n")
        out.write("    u32 functionVa;\n")
        out.write("    u32 objectGlobalVa;\n")
        out.write("    u16 offset;\n")
        out.write("    u8 size;\n")
        out.write("    ManagerFieldAccessKind kind;\n")
        out.write("    u16 count;\n")
        out.write("    u16 lineNumber;\n")
        out.write("};\n\n")
        out.write("inline constexpr ManagerLiteralWrite kManagerLiteralWrites[] = {\n")
        for row in writes:
            out.write("    {0x%06xu, 0x%06xu, 0x%03xu, %uu, %uu, 0x%xu},\n" % (
                row["functionVa"], row["objectGlobalVa"], row["offset"],
                row["size"], row["line"], row["value"]))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kManagerLiteralWriteCount =\n")
        out.write("    sizeof(kManagerLiteralWrites) / sizeof(kManagerLiteralWrites[0]);\n\n")
        out.write("inline constexpr ManagerAccessor kManagerAccessors[] = {\n")
        for row in accessors:
            out.write(
                "    {0x%06xu, 0x%06xu, 0x%03xu, %uu, "
                "ManagerFieldAccessKind::%s, %uu, 0x%06xu, %uu},\n" %
                (row["functionVa"], row["objectGlobalVa"], row["offset"],
                 row["size"], row["kind"], row["callCount"],
                 row["forwardedVa"], row["line"]))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kManagerAccessorCount =\n")
        out.write("    sizeof(kManagerAccessors) / sizeof(kManagerAccessors[0]);\n\n")
        out.write("inline constexpr ManagerFieldAccess kManagerFieldAccesses[] = {\n")
        for row in accesses:
            out.write(
                "    {0x%06xu, 0x%06xu, 0x%03xu, %uu, "
                "ManagerFieldAccessKind::%s, %uu, %uu},\n" %
                (row["functionVa"], row["objectGlobalVa"], row["offset"],
                 row["size"], row["kind"], row["count"], row["line"]))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kManagerFieldAccessCount =\n")
        out.write("    sizeof(kManagerFieldAccesses) / sizeof(kManagerFieldAccesses[0]);\n\n")
        out.write("inline std::size_t countManagerLiteralWrites(u16 offset) {\n")
        out.write("    std::size_t count = 0;\n")
        out.write("    for (const ManagerLiteralWrite& row : kManagerLiteralWrites)\n")
        out.write("        if (row.offset == offset) ++count;\n")
        out.write("    return count;\n")
        out.write("}\n\n")
        out.write("inline bool managerHasLiteralWrite(u16 offset, u32 value) {\n")
        out.write("    for (const ManagerLiteralWrite& row : kManagerLiteralWrites)\n")
        out.write("        if (row.offset == offset && row.value == value) return true;\n")
        out.write("    return false;\n")
        out.write("}\n\n")
        out.write("inline const ManagerAccessor* findManagerAccessor(u32 functionVa) {\n")
        out.write("    for (const ManagerAccessor& row : kManagerAccessors)\n")
        out.write("        if (row.functionVa == functionVa) return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline const ManagerAccessor* findManagerAccessorAt(u16 offset) {\n")
        out.write("    for (const ManagerAccessor& row : kManagerAccessors)\n")
        out.write("        if (row.offset == offset) return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline const ManagerFieldAccess* findManagerFieldAccess(\n")
        out.write("    u32 functionVa, u16 offset) {\n")
        out.write("    for (const ManagerFieldAccess& row : kManagerFieldAccesses)\n")
        out.write("        if (row.functionVa == functionVa && row.offset == offset)\n")
        out.write("            return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("} // namespace mmu\n")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    report = sys.argv[2] if len(sys.argv) > 2 else REPORT_PATH
    header = sys.argv[3] if len(sys.argv) > 3 else HEADER_PATH
    pe = PE(open(sys.argv[1], "rb").read())
    writes = collect_literal_writes()
    accessors = collect_accessors(pe)
    accesses = collect_consumers()
    write_report(writes, accessors, accesses)
    write_header(writes, accessors, accesses)
    print("writes=%d accessors=%d accesses=%d report=%s header=%s" %
          (len(writes), len(accessors), len(accesses), report, header))
    return 0


if __name__ == "__main__":
    sys.exit(main())
