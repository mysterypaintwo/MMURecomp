#!/usr/bin/env python3
"""Recover literal level-constructor setup evidence from MMU.exe.

The 38 constructors selected by FUN_0050e610 share a short setup prefix:
two literal (or two-way alternate) cue strings are passed to the registrar
FUN_00539770, the saved stage id is written to the level object at +0x8, an
"Environment" object from FUN_00443e30 is stored at +0x14, and a literal value
from 0x69899c or 0x6989b4 is copied to global 0x6c5598.

This analyzer records those literal values, the exact registrar/environment
call sites in the binary, and every literal zero write to the level object. It
does not assign gameplay meaning to the cue strings, the global or the fields.

Outputs:
  docs/level_setup.txt
  core/include/mmu/core/LevelSetupTable.h
"""

import os
import re
import struct
import sys

from analyze_activity_masks import find_function_start
from analyze_rtti import PE, find_all


PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DECLARE_STAGES = os.path.join(PROJECT_ROOT, "docs", "continue_stages.txt")
DECOMPILED_DIR = os.path.join(PROJECT_ROOT, "docs", "decompiled")
REPORT_PATH = os.path.join(PROJECT_ROOT, "docs", "level_setup.txt")
HEADER_PATH = os.path.join(
    PROJECT_ROOT, "core", "include", "mmu", "core", "LevelSetupTable.h")

CUE_REGISTRAR_VA = 0x539770
ENVIRONMENT_HELPER_VA = 0x443E30
GLOBAL_VA = 0x6C5598
STAGE_WRITE_OFFSET = 0x08
ENVIRONMENT_WRITE_OFFSET = 0x14

MAX_CUES = 4
MAX_ZERO_WRITES = 24

STAGE_RE = re.compile(
    r"stage=\s*(\d+)\s+0x[0-9a-f]+\s+constructor=(0x[0-9a-f]+)\s+class=(\S+)")
STRING_RE = re.compile(r'"((?:[^"\\]|\\.)*)"')
CTOR_RE = re.compile(
    r"basic_string<char,std::char_traits<char>,std::allocator<char>_>\s*"
    r"\(\s*([^,]+?)\s*,\s*([^;]+?)\s*\)", re.S)
ASSIGN_RE = re.compile(r'(\w+)\s*=\s*"((?:[^"\\]|\\.)*)"\s*;')
STAGE_WRITE_RE = re.compile(r"param_1\[2\]\s*=\s*(0x[0-9a-f]+|\d+)\s*;", re.I)
ENV_WRITE_RE = re.compile(r"param_1\[5\]\s*=\s*(\w+)\s*;")
ZERO_RE = re.compile(r"param_1\[(0x[0-9a-f]+|\d+)\]\s*=\s*0\s*;", re.I)
GLOBAL_RE = re.compile(r"_DAT_006c5598\s*=\s*_DAT_(0069899c|006989b4)\b", re.I)


def decoder_path(va):
    return os.path.join(DECOMPILED_DIR, "0x%06x.c" % va)


def unescape(raw):
    return raw.replace('\\"', '"').replace("\\\\", "\\")


def parse_continue_stages():
    rows = []
    for line in open(DECLARE_STAGES, encoding="latin1"):
        match = STAGE_RE.search(line)
        if match is None:
            continue
        rows.append({
            "stageId": int(match.group(1)),
            "constructorVa": int(match.group(2), 16),
            "className": match.group(3),
        })
    if not rows:
        raise RuntimeError("no stage rows in " + DECLARE_STAGES)
    return rows


def direct_call_sites(pe, target):
    result = []
    for offset in find_all(pe.data, b"\xe8"):
        if offset + 5 > len(pe.data):
            continue
        displacement = struct.unpack_from("<i", pe.data, offset + 1)[0]
        rva = pe.off_to_rva(offset)
        if rva is None:
            continue
        site = pe.image_base + rva
        if site + 5 + displacement == target:
            result.append(site)
    return sorted(set(result))


def parse_cues(text, constructor_va):
    """Return the first two basic_string arguments passed to the registrar."""
    matches = list(CTOR_RE.finditer(text))
    if len(matches) < 2:
        raise RuntimeError("fewer than two cue strings at 0x%x" % constructor_va)
    groups = []
    for index in (0, 1):
        match = matches[index]
        argument = re.sub(r"\s+", " ", match.group(2)).strip()
        literal = STRING_RE.fullmatch(argument)
        if literal is not None:
            values = [unescape(literal.group(1))]
        else:
            start = matches[index - 1].end() if index else 0
            segment = text[start:match.start()]
            values = []
            for assignment in ASSIGN_RE.finditer(segment):
                if assignment.group(1) != argument:
                    continue
                value = unescape(assignment.group(2))
                if value not in values:
                    values.append(value)
            if not values:
                raise RuntimeError(
                    "no literal values for cue variable '%s' at 0x%x" %
                    (argument, constructor_va))
        if len(values) > MAX_CUES:
            raise RuntimeError("too many cue alternatives at 0x%x" %
                               constructor_va)
        groups.append(values)
    return groups


def parse_setup(row):
    constructor_va = row["constructorVa"]
    path = decoder_path(constructor_va)
    if not os.path.isfile(path):
        raise RuntimeError("missing decompilation: " + path)
    text = open(path, encoding="latin1").read()
    lines = text.splitlines()
    registrar_line = next(
        (i for i, line in enumerate(lines)
         if "FUN_00539770()" in line), None)
    if registrar_line is None:
        raise RuntimeError("no cue registrar call at 0x%x" % constructor_va)
    prefix = "\n".join(lines[:registrar_line])

    loop_cues, intro_cues = parse_cues(prefix, constructor_va)

    stage_writes = [int(value, 0) for value in STAGE_WRITE_RE.findall(text)]
    if stage_writes != [row["stageId"]]:
        raise RuntimeError("stage id mismatch at 0x%x: %s" %
                           (constructor_va, stage_writes))

    environment_writes = ENV_WRITE_RE.findall(text)
    if not environment_writes:
        raise RuntimeError("no Environment write at 0x%x" % constructor_va)
    helper_seen = False
    for line in lines:
        if "FUN_00443e30()" in line:
            helper_seen = True
        if re.search(r"param_1\[5\]\s*=\s*%s\s*;" %
                     re.escape(environment_writes[0]), line):
            break
    if not helper_seen:
        raise RuntimeError("no Environment helper at 0x%x" % constructor_va)

    global_match = GLOBAL_RE.search(text)
    if global_match is None:
        raise RuntimeError("no global source write at 0x%x" % constructor_va)

    zero_offsets = sorted({int(value, 0) * 4 for value in ZERO_RE.findall(text)})
    if len(zero_offsets) > MAX_ZERO_WRITES:
        raise RuntimeError("too many zero writes at 0x%x" % constructor_va)

    result = dict(row)
    result["loopCues"] = loop_cues
    result["introCues"] = intro_cues
    result["globalSourceVa"] = int(global_match.group(1), 16)
    result["zeroWriteOffsets"] = zero_offsets
    return result


def collect_rows(pe):
    stages = parse_continue_stages()
    registrar_calls = {}
    for site in direct_call_sites(pe, CUE_REGISTRAR_VA):
        function_va = find_function_start(pe, site)
        if function_va in registrar_calls:
            raise RuntimeError("duplicate registrar owner 0x%x" % function_va)
        registrar_calls[function_va] = site

    environment_calls = {}
    for site in direct_call_sites(pe, ENVIRONMENT_HELPER_VA):
        function_va = find_function_start(pe, site)
        if function_va in registrar_calls:
            environment_calls.setdefault(function_va, []).append(site)

    rows = []
    for stage in stages:
        constructor_va = stage["constructorVa"]
        if constructor_va not in registrar_calls:
            raise RuntimeError("no registrar call for 0x%x" % constructor_va)
        registrar_call = registrar_calls[constructor_va]
        candidates = [
            site for site in environment_calls.get(constructor_va, [])
            if site > registrar_call]
        if not candidates:
            raise RuntimeError("no Environment call for 0x%x" % constructor_va)
        row = parse_setup(stage)
        row["cueRegistrarCallVa"] = registrar_call
        row["environmentCallVa"] = min(candidates)
        rows.append(row)
    rows.sort(key=lambda row: row["stageId"])
    return rows


def escape(value):
    return value.replace("\\", "\\\\").replace('"', '\\"')


def unique_cues(rows):
    values = set()
    for row in rows:
        values.update(cue for cue in row["loopCues"] + row["introCues"] if cue)
    return sorted(values)


def format_cues(values):
    return "|".join(values) if any(values) else ""


def write_report(rows):
    cues = unique_cues(rows)
    sources = {}
    for row in rows:
        sources[row["globalSourceVa"]] = sources.get(
            row["globalSourceVa"], 0) + 1
    with open(REPORT_PATH, "w", encoding="latin1") as out:
        out.write("Generated by tools/analyze_level_setup.py\n")
        out.write("Source: docs/decompiled/<constructor>.c and direct calls in MMU.exe.\n")
        out.write("constructors=%d unique_cues=%d registrar=0x%06x "
                  "env_helper=0x%06x global=0x%06x\n" %
                  (len(rows), len(cues), CUE_REGISTRAR_VA,
                   ENVIRONMENT_HELPER_VA, GLOBAL_VA))
        out.write("registrar_calls=%d env_calls=%d global_sources=%s\n\n" %
                  (len(rows), len(rows),
                   " ".join("0x%06x:%u" % (source, sources[source])
                            for source in sorted(sources))))
        for row in rows:
            zeros = ",".join("+0x%02x" % offset
                             for offset in row["zeroWriteOffsets"])
            out.write(
                "stage=%2d ctor=0x%06x class=%s registrar_call=0x%06x "
                "env_call=0x%06x stage_offset=0x%02x env_offset=0x%02x "
                "loop='%s' intro='%s' zero=%s global=0x%06x<=0x%06x\n" %
                (row["stageId"], row["constructorVa"], row["className"],
                 row["cueRegistrarCallVa"], row["environmentCallVa"],
                 STAGE_WRITE_OFFSET, ENVIRONMENT_WRITE_OFFSET,
                 format_cues(row["loopCues"]) or "-",
                 format_cues(row["introCues"]) or "-", zeros or "-",
                 GLOBAL_VA, row["globalSourceVa"]))


def write_header(rows):
    cues = unique_cues(rows)
    with open(HEADER_PATH, "w", encoding="latin1") as out:
        out.write("#pragma once\n\n")
        out.write('#include "mmu/core/Types.h"\n\n')
        out.write("#include <cstddef>\n#include <string_view>\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_level_setup.py.\n")
        out.write("// Literal setup shared by the 38 level constructors selected by\n")
        out.write("// FUN_0050e610. Cue strings and zero-write offsets are raw evidence;\n")
        out.write("// no gameplay meaning is assigned to a cue, field or global value.\n")
        out.write("inline constexpr std::size_t kLevelSetupMaxCueAlternatives = %d;\n" %
                  MAX_CUES)
        out.write("inline constexpr std::size_t kLevelSetupMaxZeroWrites = %d;\n\n" %
                  MAX_ZERO_WRITES)
        out.write("struct RecoveredLevelSetup {\n")
        out.write("    u32 stageId;\n")
        out.write("    u32 constructorVa;\n")
        out.write("    u32 cueRegistrarVa;\n")
        out.write("    u32 cueRegistrarCallVa;\n")
        out.write("    u32 environmentHelperVa;\n")
        out.write("    u32 environmentCallVa;\n")
        out.write("    u32 globalVa;\n")
        out.write("    u32 globalSourceVa;\n")
        out.write("    u16 stageWriteOffset;\n")
        out.write("    u16 environmentWriteOffset;\n")
        out.write("    u8 loopCueCount;\n")
        out.write("    u8 introCueCount;\n")
        out.write("    u8 zeroWriteCount;\n")
        out.write("    std::string_view className;\n")
        out.write("    std::string_view loopCueList;\n")
        out.write("    std::string_view introCueList;\n")
        out.write("    std::string_view loopCues[kLevelSetupMaxCueAlternatives];\n")
        out.write("    std::string_view introCues[kLevelSetupMaxCueAlternatives];\n")
        out.write("    u16 zeroWriteOffsets[kLevelSetupMaxZeroWrites];\n")
        out.write("};\n\n")
        out.write("inline constexpr RecoveredLevelSetup kRecoveredLevelSetups[] = {\n")
        for row in rows:
            loop_cues = list(row["loopCues"]) + [""] * (
                MAX_CUES - len(row["loopCues"]))
            intro_cues = list(row["introCues"]) + [""] * (
                MAX_CUES - len(row["introCues"]))
            zeros = list(row["zeroWriteOffsets"]) + [0] * (
                MAX_ZERO_WRITES - len(row["zeroWriteOffsets"]))
            out.write(
                "    {%du, 0x%06xu, 0x%06xu, 0x%06xu, 0x%06xu, 0x%06xu, "
                "0x%06xu, 0x%06xu, 0x%02xu, 0x%02xu, %uu, %uu, %uu,\n" %
                (row["stageId"], row["constructorVa"], CUE_REGISTRAR_VA,
                 row["cueRegistrarCallVa"], ENVIRONMENT_HELPER_VA,
                 row["environmentCallVa"], GLOBAL_VA, row["globalSourceVa"],
                 STAGE_WRITE_OFFSET, ENVIRONMENT_WRITE_OFFSET,
                 len(row["loopCues"]), len(row["introCues"]),
                 len(row["zeroWriteOffsets"])))
            out.write('        "%s", "%s", "%s",\n' % (
                escape(row["className"]), escape(format_cues(row["loopCues"])),
                escape(format_cues(row["introCues"]))))
            out.write("        {%s}, {%s},\n" % (
                ", ".join('"%s"' % escape(value) for value in loop_cues),
                ", ".join('"%s"' % escape(value) for value in intro_cues)))
            out.write("        {%s}},\n" % ", ".join(
                "0x%02xu" % value for value in zeros))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kRecoveredLevelSetupCount =\n")
        out.write("    sizeof(kRecoveredLevelSetups) / sizeof(kRecoveredLevelSetups[0]);\n\n")
        out.write("inline constexpr std::string_view kRecoveredLevelSetupCues[] = {\n")
        for cue in cues:
            out.write('    "%s",\n' % escape(cue))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kRecoveredLevelSetupCueCount =\n")
        out.write("    sizeof(kRecoveredLevelSetupCues) / sizeof(kRecoveredLevelSetupCues[0]);\n\n")
        out.write("inline const RecoveredLevelSetup* findLevelSetup(u32 stageId) {\n")
        out.write("    for (const RecoveredLevelSetup& row : kRecoveredLevelSetups)\n")
        out.write("        if (row.stageId == stageId) return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline const RecoveredLevelSetup* findLevelSetupByConstructor(u32 va) {\n")
        out.write("    for (const RecoveredLevelSetup& row : kRecoveredLevelSetups)\n")
        out.write("        if (row.constructorVa == va) return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline bool levelSetupHasLoopCue(const RecoveredLevelSetup& row,\n")
        out.write("                                  std::string_view cue) {\n")
        out.write("    for (u8 i = 0; i < row.loopCueCount; ++i)\n")
        out.write("        if (row.loopCues[i] == cue) return true;\n")
        out.write("    return false;\n")
        out.write("}\n\n")
        out.write("inline bool levelSetupHasIntroCue(const RecoveredLevelSetup& row,\n")
        out.write("                                   std::string_view cue) {\n")
        out.write("    for (u8 i = 0; i < row.introCueCount; ++i)\n")
        out.write("        if (row.introCues[i] == cue) return true;\n")
        out.write("    return false;\n")
        out.write("}\n\n")
        out.write("inline const RecoveredLevelSetup* findLevelSetupByLoopCue(\n")
        out.write("    std::string_view cue) {\n")
        out.write("    for (const RecoveredLevelSetup& row : kRecoveredLevelSetups)\n")
        out.write("        if (levelSetupHasLoopCue(row, cue)) return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline const RecoveredLevelSetup* findLevelSetupByIntroCue(\n")
        out.write("    std::string_view cue) {\n")
        out.write("    for (const RecoveredLevelSetup& row : kRecoveredLevelSetups)\n")
        out.write("        if (levelSetupHasIntroCue(row, cue)) return &row;\n")
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
    rows = collect_rows(pe)
    write_report(rows)
    write_header(rows)
    print("setups=%d cues=%d report=%s header=%s" %
          (len(rows), len(unique_cues(rows)), report, header))
    return 0


if __name__ == "__main__":
    sys.exit(main())
