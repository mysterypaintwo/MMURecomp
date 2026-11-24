#!/usr/bin/env python3
"""Recover raw outro/handoff call-site evidence from MMU.exe.

This pass records only direct call sites whose target and pushed string VA can
be verified in the PE: cue/set-state/state-name calls, the Wily 5 outro cutscene
loader, and the `FUN_005a8870` handoff calls. Every hash is recomputed from the
recovered literal with the project's existing name/CRC hash functions.

The table intentionally does not assign gameplay meaning to a call, literal or
owner. It is evidence for a later state-machine pass, not a model of the outro.

Outputs:
  docs/outro_handoff.txt
  core/include/mmu/core/OutroHandoffTable.h
"""

import os
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM

from analyze_activity_masks import DECOMPILED_DIR
from analyze_game_flow import cstring
from analyze_rtti import PE
from analyze_state_names import state_hash
from crack_hash import mmu_crc32
from disasm import find_vftables


PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPORT_PATH = os.path.join(PROJECT_ROOT, "docs", "outro_handoff.txt")
HEADER_PATH = os.path.join(
    PROJECT_ROOT, "core", "include", "mmu", "core", "OutroHandoffTable.h")

KIND_BY_TARGET = {
    0x4026B0: "CueRegistrar",
    0x4405C0: "StateSetter",
    0x420A90: "NamedStateCall",
    0x621890: "CutsceneLoader",
    0x5A8870: "DirectHandoff",
    0x443E30: "EnvironmentHelper",
    0x462A90: "StateQueryCall",
}

KIND_NAMES = [
    "CueRegistrar",
    "StateSetter",
    "NamedStateCall",
    "CutsceneLoader",
    "DirectHandoff",
    "EnvironmentHelper",
    "StateQueryCall",
]

FACT_KIND_NAMES = ["FieldOffset", "GlobalConstant", "LiteralConstant"]
NO_OFFSET = 0xffffffff

# (function VA, decompiled line, fact kind, byte offset or NO_OFFSET,
#  raw constant, required token in that decompiled line)
# Byte offsets are the literal object offsets or int-array offsets already
# present in the Ghidra output; no field is named from them.
RAW_FACTS = [
    (0x5A8870, 14, "FieldOffset", 0x18C, 1, "param_1 + 0x18c"),
    (0x5A8870, 15, "FieldOffset", 0x150, 0, "param_1 + 0x150"),
    (0x5A8870, 16, "FieldOffset", 0x18C, 0, "*(undefined4 *)(param_1 + 0x18c) = 0"),
    (0x5A8870, 19, "FieldOffset", 0x150, 0,
     "*(int *)(param_1 + 0x150) = *(int *)(param_1 + 0x150) + 1"),
    (0x5A8870, 24, "FieldOffset", 0x18C, 0, "+ -1"),
    (0x5A8870, 26, "FieldOffset", 0x190, 1, "param_1 + 400"),
    (0x5A8870, 28, "GlobalConstant", NO_OFFSET, 0x698B48, "_DAT_00698b48"),
    (0x5A8870, 29, "FieldOffset", 0x190, 0, "*(undefined4 *)(param_1 + 400) = 0"),
    (0x5A8870, 33, "GlobalConstant", NO_OFFSET, 0x697CE8, "_DAT_00697ce8"),
    (0x5A8870, 46, "LiteralConstant", NO_OFFSET, 0x1B, "0x1b"),
    (0x5A8870, 49, "FieldOffset", 0x17C, 0xffffffff, "0xffffffff"),
    (0x5A8870, 51, "LiteralConstant", NO_OFFSET, 1, "FUN_00507e90(1)"),
    (0x5A8870, 54, "LiteralConstant", NO_OFFSET, 2, "FUN_00507e90(2)"),
    (0x5A8870, 55, "GlobalConstant", NO_OFFSET, 0x698B44, "_DAT_00698b44"),
    (0x5A8870, 56, "FieldOffset", 0x17C, 0,
     "*(undefined4 *)(param_1 + 0x17c) = uVar3"),
    (0x5B02F0, 373, "FieldOffset", 0x18C, 0, "param_1[99]"),
    (0x5B02F0, 378, "FieldOffset", 0x190, 0, "param_1[100]"),
    (0x5B02F0, 379, "FieldOffset", 0x3DC, 0, "param_1[0xf7]"),
    (0x5B02F0, 383, "LiteralConstant", NO_OFFSET, 10, "local_164 = 10"),
    (0x5B02F0, 385, "FieldOffset", 0x194, 0, "param_1[0x65] = local_164"),
    (0x5B02F0, 387, "GlobalConstant", NO_OFFSET, 0x698B48, "_DAT_00698b48"),
    (0x5B02F0, 389, "GlobalConstant", NO_OFFSET, 0x698A88, "_DAT_00698a88"),
    (0x5B02F0, 390, "LiteralConstant", NO_OFFSET, 0xD, "local_2c < 0xd"),
    (0x5B02F0, 394, "FieldOffset", 0x194, 0, "param_1[0x65] = local_2c"),
    (0x5262C0, 38, "FieldOffset", 0x4, 0xD, "*(int *)(param_1 + 4) == 0xd"),
    (0x5262C0, 38, "FieldOffset", 0x10, 0, "*(int *)(param_1 + 0x10) == 0"),
    (0x5262C0, 39, "FieldOffset", 0xA0, 0,
     "*(char *)(param_1 + 0xa0) == '\\0'"),
    (0x5262C0, 46, "FieldOffset", 0x10, 0,
     "*(undefined4 *)(param_1 + 0x10) = local_1c"),
    (0x5262C0, 47, "FieldOffset", 0xA0, 1,
     "*(undefined1 *)(param_1 + 0xa0) = 1"),
    (0x5262C0, 49, "FieldOffset", 0x4, 0x13, "*(int *)(param_1 + 4) == 0x13"),
    (0x5262C0, 54, "GlobalConstant", NO_OFFSET, 0x698F98, "_DAT_00698f98"),
    (0x5262C0, 85, "FieldOffset", 0x144, 0,
     "*(char *)(param_1 + 0x144) != '\\0'"),
    (0x5262C0, 87, "FieldOffset", 0x140, 0, "*(float *)(param_1 + 0x140)"),
    (0x5262C0, 89, "FieldOffset", 0x148, 0, "*(float *)(param_1 + 0x148)"),
    (0x5262C0, 91, "GlobalConstant", NO_OFFSET, 0x697CE0, "_DAT_00697ce0"),
    (0x5262C0, 95, "LiteralConstant", NO_OFFSET, 0xD, "iVar5 % 0xd"),
    (0x5262C0, 96, "GlobalConstant", NO_OFFSET, 0x698F90, "_DAT_00698f90"),
    (0x5262C0, 100, "GlobalConstant", NO_OFFSET, 0x698A30, "_DAT_00698a30"),
    (0x5262C0, 104, "GlobalConstant", NO_OFFSET, 0x698818, "_DAT_00698818"),
    (0x5262C0, 127, "LiteralConstant", NO_OFFSET, 100, "local_28 % 100"),
    (0x5262C0, 128, "LiteralConstant", NO_OFFSET, 0x32, "local_28 < 0x32"),
    (0x5262C0, 134, "FieldOffset", 0x14C, 0,
     "*(char *)(param_1 + 0x14c) == '\\0'"),
    (0x5262C0, 138, "FieldOffset", 0x14C, 1,
     "*(undefined1 *)(param_1 + 0x14c) = 1"),
]

# (owner function VA, call VA, direct call target, pushed string VA or 0,
#  owner label, recovered literal or "")
CALLS = [
    (0x5A8870, 0x5A8A97, 0x420A90, 0x67D744, "FUN_005a8870", "EnergyMeter3s"),
    (0x5B02F0, 0x5B0A31, 0x4026B0, 0x67DF08, "MegaMan", "EnergyMeter3s"),
    (0x5B02F0, 0x5B0A57, 0x5A8870, 0, "MegaMan", ""),
    (0x5B02F0, 0x5B0A70, 0x4026B0, 0x67DF18, "MegaMan", "EnergyMeter3s"),
    (0x5B02F0, 0x5B0B69, 0x5A8870, 0, "MegaMan", ""),
    (0x5A73E0, 0x5A73EA, 0x5A8870, 0, "FUN_005a73e0", ""),
    (0x5262C0, 0x52633E, 0x443E30, 0x66F034, "LevelWilyFive", "Zero"),
    (0x5262C0, 0x5263B8, 0x462A90, 0x66F03C, "LevelWilyFive", "Stand"),
    (0x5262C0, 0x526410, 0x621890, 0x66F044, "LevelWilyFive",
     r"..\Cutscenes\CSNZero\csnWily5Outro.xcut"),
    (0x5262C0, 0x526632, 0x4026B0, 0x66F06C, "LevelWilyFive", "BigExplosion"),
    (0x5A3690, 0x5A36CA, 0x4026B0, 0x67D19C, "Boss", "EnergyMeter3s"),
    (0x5A3E20, 0x5A3E67, 0x4026B0, 0x67D0F0, "Boss", "Death"),
    (0x575490, 0x5754CE, 0x4026B0, 0x679B10, "WilyBoss", "EnergyMeter3s"),
    (0x575780, 0x575879, 0x4026B0, 0x679B30, "WilyBoss", "BigExplosion"),
    (0x56A900, 0x56A9CD, 0x4405C0, 0x678CB8, "WilyCapsuleMech", "Rise"),
    (0x56A900, 0x56AA33, 0x4405C0, 0x678CC8, "WilyCapsuleMech", "WilyBossExplosion"),
    (0x5C6760, 0x5C682F, 0x4405C0, 0x67FA70, "TeleporterGauntlet", "ConcludeBossFight"),
    (0x5C8560, 0x5C8625, 0x4405C0, 0x67FC48, "TeleporterEndlessBoss", "ConcludeBossFight"),
    (0x5D3890, 0x5D3955, 0x4405C0, 0x68138C, "TeleporterEndless", "ConcludeBossFight"),
]


def literal_hash(literal):
    """Existing hashes: state hash for names, paked key hash for .xcut paths."""
    if literal.startswith("..\\Cutscenes\\"):
        return mmu_crc32(literal.encode("latin1"))
    return state_hash(literal)


def function_instructions(md, pe, function_va):
    off = pe.rva_to_off(function_va - pe.image_base)
    if off is None:
        raise RuntimeError("function VA is not mapped: 0x%x" % function_va)
    return list(md.disasm(pe.data[off:off + 0x8000], function_va))


def verify_call(md, pe, row):
    function_va, call_va, target_va, string_va, owner, literal = row
    if target_va not in KIND_BY_TARGET:
        raise RuntimeError("no raw kind for target 0x%x" % target_va)

    instructions = function_instructions(md, pe, function_va)
    by_address = {ins.address: ins for ins in instructions}
    call = by_address.get(call_va)
    if call is None or call.mnemonic != "call" or not call.operands:
        raise RuntimeError("missing call instruction at 0x%x" % call_va)
    operand = call.operands[0]
    if operand.type != X86_OP_IMM or operand.imm != target_va:
        raise RuntimeError("call 0x%x does not target 0x%x" % (call_va, target_va))

    pushes = [
        ins.operands[0].imm for ins in instructions
        if call_va - 64 <= ins.address <= call_va and ins.mnemonic == "push"
        and ins.operands and ins.operands[0].type == X86_OP_IMM
    ]
    if string_va:
        if string_va not in pushes:
            raise RuntimeError("call 0x%x has no push of string VA 0x%x" %
                               (call_va, string_va))
        actual = cstring(pe, string_va, min_len=1, max_len=120)
        if actual != literal:
            raise RuntimeError("string VA 0x%x is %r, expected %r" %
                               (string_va, actual, literal))

    return {
        "functionVa": function_va,
        "callVa": call_va,
        "targetVa": target_va,
        "stringVa": string_va,
        "hash": literal_hash(literal) if string_va else 0,
        "kind": KIND_BY_TARGET[target_va],
        "owner": owner,
        "literal": literal,
    }


def verify_all(pe):
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    rows = sorted((verify_call(md, pe, row) for row in CALLS),
                  key=lambda row: row["callVa"])
    verify_owners(pe, rows)
    return rows


def verify_owners(pe, rows):
    """Class labels must appear in an RTTI vftable owning that function VA."""
    owners_by_function = {}
    for mangled, tables in find_vftables(pe).items():
        if not mangled.startswith(".?AV"):
            continue
        class_name = mangled[4:].split("@@", 1)[0]
        for _vftable_va, functions in tables:
            for function_va in functions:
                owners_by_function.setdefault(function_va, set()).add(class_name)

    for row in rows:
        owner = row["owner"]
        if owner.startswith("FUN_"):
            continue
        owners = owners_by_function.get(row["functionVa"], set())
        if owner not in owners:
            raise RuntimeError("RTTI owner mismatch at 0x%x: %s not in %s" % (
                row["callVa"], owner, sorted(owners)))


def verify_facts():
    """Check each curated fact against the exact decompiled source line."""
    cache = {}
    facts = []
    for function_va, line_number, kind, byte_offset, value, token in RAW_FACTS:
        if function_va not in cache:
            path = os.path.join(DECOMPILED_DIR, "0x%06x.c" % function_va)
            with open(path, encoding="utf-8") as source:
                cache[function_va] = source.read().splitlines()
        lines = cache[function_va]
        if not (1 <= line_number <= len(lines)):
            raise RuntimeError("missing decompiled line 0x%x:%d" %
                               (function_va, line_number))
        expression = lines[line_number - 1].strip()
        if token not in expression:
            raise RuntimeError("fact token mismatch at 0x%x:%d: %s not in %s" %
                               (function_va, line_number, token, expression))
        facts.append({
            "functionVa": function_va,
            "lineNumber": line_number,
            "kind": kind,
            "byteOffset": byte_offset,
            "value": value,
            "expression": expression,
        })
    return facts


def cpp_string(value):
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def write_report(rows, facts):
    strings = {}
    for row in rows:
        if row["stringVa"]:
            strings[row["stringVa"]] = (row["literal"], row["hash"])

    with open(REPORT_PATH, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_outro_handoff.py\n")
        out.write("Source: direct calls in MMU.exe; literals and hashes verified from PE.\n")
        out.write("hash: state-name hash for names, cutscene key CRC-32 for .xcut paths.\n")
        out.write("owner: RTTI vftable class name or raw Ghidra function name.\n")
        out.write("calls=%d strings=%d raw_facts=%d\n" %
                  (len(rows), len(strings), len(facts)))
        out.write("No gameplay timing, transition or destination is inferred.\n\n")
        out.write("=== call sites ===\n")
        for row in rows:
            out.write(
                "func=0x%06x call=0x%06x target=0x%06x string_va=0x%06x "
                "hash=0x%08x kind=%-16s owner=%-20s literal=%s\n" % (
                    row["functionVa"], row["callVa"], row["targetVa"],
                    row["stringVa"], row["hash"], row["kind"], row["owner"],
                    row["literal"]))
        out.write("\n=== recovered string VAs ===\n")
        for va in sorted(strings):
            literal, hash_value = strings[va]
            out.write("va=0x%06x hash=0x%08x %s\n" % (va, hash_value, literal))
        out.write("\n=== raw offsets and constants ===\n")
        for fact in facts:
            offset = "none" if fact["byteOffset"] == NO_OFFSET else \
                "0x%x" % fact["byteOffset"]
            out.write(
                "func=0x%06x line=%4d kind=%-15s offset=%-6s value=0x%08x "
                "expr=%s\n" % (
                    fact["functionVa"], fact["lineNumber"], fact["kind"],
                    offset, fact["value"], fact["expression"]))


def write_header(rows, facts):
    strings = {}
    for row in rows:
        if row["stringVa"]:
            strings[row["stringVa"]] = (row["literal"], row["hash"])

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
        "// Generated by tools/analyze_outro_handoff.py.",
        "// Every row is a direct call site verified in MMU.exe. Hashes and RTTI",
        "// class owners are verified from the binary; no gameplay semantics are",
        "// assigned to an owner, call target, literal or offset.",
        "enum class OutroHandoffKind : u8 {",
    ]
    lines += ["    %s," % name for name in KIND_NAMES]
    lines += [
        "};",
        "",
        "struct OutroHandoffCall {",
        "    u32 functionVa;",
        "    u32 callVa;",
        "    u32 targetVa;",
        "    u32 stringVa;",
        "    u32 hash;",
        "    OutroHandoffKind kind;",
        "    std::string_view owner;",
        "    std::string_view literal;",
        "};",
        "",
        "inline constexpr OutroHandoffCall kOutroHandoffCalls[] = {",
    ]
    for row in rows:
        lines.append(
            "    {0x%06xu, 0x%06xu, 0x%06xu, 0x%06xu, 0x%08xu, "
            "OutroHandoffKind::%s, %s, %s}," % (
                row["functionVa"], row["callVa"], row["targetVa"],
                row["stringVa"], row["hash"], row["kind"],
                cpp_string(row["owner"]), cpp_string(row["literal"])))
    lines += [
        "};",
        "",
        "inline constexpr std::size_t kOutroHandoffCallCount =",
        "    sizeof(kOutroHandoffCalls) / sizeof(kOutroHandoffCalls[0]);",
        "",
        "struct OutroHandoffString {",
        "    u32 stringVa;",
        "    u32 hash;",
        "    std::string_view literal;",
        "};",
        "",
        "inline constexpr OutroHandoffString kOutroHandoffStrings[] = {",
    ]
    for va in sorted(strings):
        literal, hash_value = strings[va]
        lines.append("    {0x%06xu, 0x%08xu, %s}," %
                     (va, hash_value, cpp_string(literal)))
    lines += [
        "};",
        "",
        "inline constexpr std::size_t kOutroHandoffStringCount =",
        "    sizeof(kOutroHandoffStrings) / sizeof(kOutroHandoffStrings[0]);",
        "",
        "// Raw facts are verbatim decompiled expressions with their source line.",
        "// Byte offsets, globals and constants stay uninterpreted.",
        "enum class OutroHandoffFactKind : u8 {",
    ]
    lines += ["    %s," % name for name in FACT_KIND_NAMES]
    lines += [
        "};",
        "",
        "inline constexpr u32 kOutroHandoffNoOffset = 0xffffffffu;",
        "",
        "struct OutroHandoffFact {",
        "    u32 functionVa;",
        "    u16 lineNumber;",
        "    OutroHandoffFactKind kind;",
        "    u32 byteOffset;",
        "    u32 value;",
        "    std::string_view expression;",
        "};",
        "",
        "inline constexpr OutroHandoffFact kOutroHandoffFacts[] = {",
    ]
    for fact in facts:
        lines.append(
            "    {0x%06xu, %du, OutroHandoffFactKind::%s, 0x%08xu, 0x%08xu, %s},"
            % (fact["functionVa"], fact["lineNumber"], fact["kind"],
               fact["byteOffset"], fact["value"],
               cpp_string(fact["expression"])))
    lines += [
        "};",
        "",
        "inline constexpr std::size_t kOutroHandoffFactCount =",
        "    sizeof(kOutroHandoffFacts) / sizeof(kOutroHandoffFacts[0]);",
        "",
        "inline constexpr bool hasOutroHandoffFactOffset(u32 byteOffset) {",
        "    if (byteOffset == kOutroHandoffNoOffset) return false;",
        "    for (const OutroHandoffFact& fact : kOutroHandoffFacts) {",
        "        if (fact.byteOffset == byteOffset) return true;",
        "    }",
        "    return false;",
        "}",
        "",
        "inline constexpr const OutroHandoffCall* findOutroHandoffCall(u32 callVa) {",
        "    for (const OutroHandoffCall& row : kOutroHandoffCalls) {",
        "        if (row.callVa == callVa) return &row;",
        "    }",
        "    return nullptr;",
        "}",
        "",
        "inline constexpr bool hasOutroHandoffOwner(u32 functionVa) {",
        "    for (const OutroHandoffCall& row : kOutroHandoffCalls) {",
        "        if (row.functionVa == functionVa) return true;",
        "    }",
        "    return false;",
        "}",
        "",
        "} // namespace mmu",
        "",
    ]
    with open(HEADER_PATH, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    if not os.path.isdir(DECOMPILED_DIR):
        raise RuntimeError("missing decompiled evidence directory: " + DECOMPILED_DIR)
    pe = PE(open(sys.argv[1], "rb").read())
    rows = verify_all(pe)
    facts = verify_facts()
    write_report(rows, facts)
    write_header(rows, facts)
    print("outro handoff: calls=%d strings=%d raw_facts=%d" % (
        len(rows), len({row["stringVa"] for row in rows if row["stringVa"]}),
        len(facts)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
