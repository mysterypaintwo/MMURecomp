#!/usr/bin/env python3
"""Recompute bounded reconstruction coverage from generated artifacts.

Every percentage here has an explicit generated-artifact denominator. No
gameplay-completeness percentage is inferred: the total number of functions,
rules, or in-game behaviours in MMU.exe is not recovered, so it cannot be used
as a denominator. The only unverified-name count is reported separately.

Outputs:
  docs/reconstruction_coverage.txt
  core/include/mmu/core/CoverageSummary.h

Usage:
  python3 tools/generate_coverage_report.py [root]
"""

import glob
import os
import re
import sys

TOOL = "tools/generate_coverage_report.py"

# Curated derivation map: raw report -> portable header copied from it.
# A None entry means the report has no direct portable-header counterpart.
# New docs/*.txt reports must be added here; the generator fails on drift.
REPORT_HEADER = {
    "activity_mask_readers.txt": "ActivityMaskReaderTable.h",
    "activity_masks.txt": "ActivityMaskTable.h",
    "boot_cutscenes.txt": "BootCutsceneTable.h",
    "boss_damage.txt": "BossDamageParams.h",
    "camera_evidence.txt": "CameraEvidenceTable.h",
    "damage_wrappers.txt": "DamageWrapperTable.h",
    "completion_cutscenes.txt": "CompletionCutsceneTable.h",
    "continue_stages.txt": "ContinueStageTable.h",
    "cutscene_keys.txt": "RegisteredCutscenes.h",
    "enemy_edge_facts.txt": "EnemyEdgeFacts.h",
    "enemy_state_machines.txt": "EnemyStateGraph.h",
    "flow_edge_facts.txt": "FlowEdgeFacts.h",
    "game_flow.txt": "GameFlowEvidenceTable.h",
    "gun_params.txt": "GunParams.h",
    "inline_menu_resources.txt": "InlineMenuResources.h",
    "key_names.txt": "VerifiedKeyNames.h",
    "key_names_discovered.txt": "KeySweepSummary.h",
    "key_names_mitm.txt": "KeyNameCandidates.h",
    "level_setup.txt": "LevelSetupTable.h",
    "manager_fields.txt": "ManagerFieldTable.h",
    "menu_actions.txt": "MenuActionTable.h",
    "menu_items.txt": "MenuItemTables.h",
    "outro_handoff.txt": "OutroHandoffTable.h",
    "pause_weapon_resources.txt": "PauseWeaponResources.h",
    "player_control_evidence.txt": "PlayerControlEvidenceTable.h",
    "player_spawn_evidence.txt": "PlayerSpawnEvidence.h",
    "reconstruction_coverage.txt": "CoverageSummary.h",
    "rtti_classes.txt": "RttiClassNames.h",
    "save_flags.txt": "SaveFlagTable.h",
    "stage_select_grid.txt": "StageSelectGrid.h",
    "state_graph.txt": "RegisteredStateGraph.h",
    "state_names.txt": "RegisteredStateNames.h",
    "state_slot_blocks.txt": "StateSlotBlocks.h",
    "state_transitions.txt": "RegisteredStateTransitions.h",
    "vtables.txt": "RttiVtables.h",
    "wily_attack_selector.txt": "WilyAttackSelector.h",
}


def read(path):
    with open(path, "r", encoding="utf-8") as handle:
        return handle.read()


def find_int(text, pattern, label):
    match = re.search(pattern, text)
    if not match:
        raise RuntimeError("missing %s" % label)
    return int(match.group(1))


def array_body(text, name):
    match = re.search(r"\b" + re.escape(name) + r"\[\]\s*=\s*\{", text)
    if not match:
        raise RuntimeError("missing array %s" % name)
    index = match.end()
    depth = 1
    in_string = False
    escaped = False
    while index < len(text) and depth:
        char = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
        elif char == '"':
            in_string = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
        index += 1
    if depth:
        raise RuntimeError("unterminated array %s" % name)
    return text[match.end():index - 1]


def count_struct_rows(text, name):
    return sum(1 for line in array_body(text, name).splitlines()
               if re.match(r"^\s*\{", line))


def count_hex_rows(text, name):
    return len(re.findall(r"0x[0-9a-fA-F]+u?\b", array_body(text, name)))


def count_string_rows(text, name):
    return sum(1 for line in array_body(text, name).splitlines()
               if re.match(r'^\s*"', line))


def basis_points(numerator, denominator):
    if denominator == 0:
        raise RuntimeError("zero denominator")
    return (numerator * 10000 + denominator // 2) // denominator


def percent(numerator, denominator):
    return 100.0 * numerator / denominator


def cpp_string(value):
    return '"%s"' % value.replace("\\", "\\\\").replace('"', '\\"')


def reachable_core_headers(root, main_text):
    """Follow quoted mmu/core includes from main.cpp through local headers.

    Direct includes are tracked separately; transitive consumption is still
    real harness use, not a coverage gap.
    """
    include_re = re.compile(r'#include\s+"([^"]+)"')
    queue = [name for name in include_re.findall(main_text)
             if name.startswith("mmu/core/")]
    seen = set()
    while queue:
        name = queue.pop()
        if name in seen:
            continue
        seen.add(name)
        path = os.path.join(root, "core", "include", name)
        if not os.path.exists(path):
            raise RuntimeError("missing core include %s" % name)
        for child in include_re.findall(read(path)):
            if child.startswith("mmu/core/") and child not in seen:
                queue.append(child)
    return {os.path.basename(name) for name in seen}


def collect(root):
    docs = os.path.join(root, "docs")
    headers = os.path.join(root, "core", "include", "mmu", "core")
    main = read(os.path.join(root, "platform", "sdl2", "src", "main.cpp"))
    readme = read(os.path.join(root, "README.md"))

    key_report = read(os.path.join(docs, "key_names.txt"))
    resolved = find_int(key_report, r"\bresolved=(\d+)", "resolved keys")
    collected = find_int(key_report, r"\bcollected=(\d+)", "collected keys")
    mitm_report = read(os.path.join(docs, "key_names_mitm.txt"))
    mitm = find_int(mitm_report, r"\bnew_matches=(\d+)", "mitm candidates")
    sweep_report = read(os.path.join(docs, "key_names_discovered.txt"))
    sweep_candidates = find_int(sweep_report, r"\bcandidates=(\d+)",
                                "sweep candidates")
    sweep_matches = find_int(sweep_report, r"\bnew_matches=(\d+)",
                             "sweep new matches")

    slots_report = read(os.path.join(docs, "state_slot_blocks.txt"))
    slot_states = find_int(slots_report, r"\bstates=(\d+)", "slot states")
    slot_complete = find_int(slots_report, r"\bcomplete=(\d+)",
                             "complete slots")
    slot_incomplete = find_int(slots_report, r"\bincomplete=(\d+)",
                               "incomplete slots")
    if slot_complete + slot_incomplete != slot_states:
        raise RuntimeError("state slot totals disagree")

    vtables_report = read(os.path.join(docs, "vtables.txt"))
    vtable_rows = find_int(vtables_report, r"\bvtables=(\d+)", "vtables")

    rtti = read(os.path.join(headers, "RttiVtables.h"))
    vtable_header_rows = count_struct_rows(rtti, "kRttiVtables")
    slot_header_rows = count_hex_rows(rtti, "kRttiVtableSlots")
    if vtable_header_rows != vtable_rows:
        raise RuntimeError("RttiVtables.h row count disagrees with report")

    verified = read(os.path.join(headers, "VerifiedKeyNames.h"))
    verified_rows = count_struct_rows(verified, "kVerifiedKeyNames")
    if verified_rows != resolved:
        raise RuntimeError("VerifiedKeyNames.h row count disagrees with report")
    candidates = read(os.path.join(headers, "KeyNameCandidates.h"))
    candidate_rows = count_struct_rows(candidates,
                                       "kUnverifiedKeyNameCandidates")
    if candidate_rows != mitm:
        raise RuntimeError("KeyNameCandidates.h row count disagrees with report")

    decomp = read(os.path.join(headers, "DecompIndex.h"))
    decompiled_functions = count_hex_rows(decomp, "kDecompiledFunctionVas")
    proto = read(os.path.join(headers, "ProtoIndex.h"))
    proto_schemas = count_struct_rows(proto, "kProtoSchemas")
    proto_messages = count_struct_rows(proto, "kProtoMessages")
    proto_fields = count_struct_rows(proto, "kProtoFields")
    scene_names = count_string_rows(read(os.path.join(headers, "SceneNames.h")),
                                    "kSceneNames")

    commands = sorted(set(re.findall(r'registerCommand\("([^"]+)"', main)))
    block = readme[readme.index("## Debug console"):
                   readme.index("## Sprite viewer")]
    documented = [name for name in commands
                  if re.search(r"`" + re.escape(name) + r"\b", block)]

    header_paths = sorted(glob.glob(os.path.join(headers, "*.h")))
    generated = [os.path.basename(path) for path in header_paths
                 if re.search(r"//\s*Generated by", read(path))]
    direct = set(re.findall(r'#include\s+"mmu/core/([^"]+)"', main))
    reachable = reachable_core_headers(root, main)
    generated_direct = [name for name in generated if name in direct]
    generated_in_harness = [name for name in generated if name in reachable]
    generated_not_in_harness = [name for name in generated
                                if name not in reachable]

    report_paths = sorted(glob.glob(os.path.join(docs, "*.txt")))
    report_names = [os.path.basename(path) for path in report_paths]
    if set(report_names) != set(REPORT_HEADER):
        raise RuntimeError("REPORT_HEADER drift: %s" % sorted(
            set(report_names) ^ set(REPORT_HEADER)))
    unmapped_reports = []
    for name in report_names:
        header_name = REPORT_HEADER[name]
        if header_name is None:
            unmapped_reports.append("docs/" + name)
        elif not os.path.exists(os.path.join(headers, header_name)):
            raise RuntimeError("missing mapped header %s" % header_name)

    return {
        "resolved": resolved,
        "collected": collected,
        "unresolved": collected - resolved,
        "mitm": mitm,
        "sweep_candidates": sweep_candidates,
        "sweep_matches": sweep_matches,
        "slot_states": slot_states,
        "slot_complete": slot_complete,
        "slot_incomplete": slot_incomplete,
        "vtable_rows": vtable_rows,
        "slot_header_rows": slot_header_rows,
        "decompiled_functions": decompiled_functions,
        "proto_schemas": proto_schemas,
        "proto_messages": proto_messages,
        "proto_fields": proto_fields,
        "scene_names": scene_names,
        "commands": len(commands),
        "documented": len(documented),
        "missing_commands": [name for name in commands
                            if name not in documented],
        "generated_headers": len(generated),
        "generated_direct": len(generated_direct),
        "generated_in_harness": len(generated_in_harness),
        "generated_not_in_harness": generated_not_in_harness,
        "raw_reports": len(report_names),
        "header_backed_reports": len(report_names) - len(unmapped_reports),
        "unmapped_reports": unmapped_reports,
    }


def write_report(path, data):
    lines = [
        "Generated by %s" % TOOL,
        "Bounded reconstruction coverage; no gameplay-completeness percentage "
        "is claimed.",
        "Each denominator is an explicit generated-artifact count. The total "
        "number of",
        "MMU.exe functions, rules, and in-game behaviours is not recovered.",
        "",
        "verified keys=%d/%d (%.1f%%) unresolved=%d" % (
            data["resolved"], data["collected"],
            percent(data["resolved"], data["collected"]),
            data["unresolved"]),
        "mitm unverified=%d sweep candidates=%d sweep new_matches=%d" % (
            data["mitm"], data["sweep_candidates"], data["sweep_matches"]),
        "state slots=%d/%d (%.1f%%) incomplete=%d" % (
            data["slot_complete"], data["slot_states"],
            percent(data["slot_complete"], data["slot_states"]),
            data["slot_incomplete"]),
        "raw reports=%d/%d (%.1f%%) without portable header=%d" % (
            data["header_backed_reports"], data["raw_reports"],
            percent(data["header_backed_reports"], data["raw_reports"]),
            len(data["unmapped_reports"])),
    ]
    for name in data["unmapped_reports"]:
        lines.append("  without portable header=%s" % name)
    lines.extend([
        "generated headers=%d reachable from SDL harness=%d (%.1f%%) direct=%d" % (
            data["generated_headers"], data["generated_in_harness"],
            percent(data["generated_in_harness"], data["generated_headers"]),
            data["generated_direct"]),
        "  not reachable=%s" % ",".join(data["generated_not_in_harness"]),
        "registered commands=%d documented=%d (%.1f%%) missing=%d" % (
            data["commands"], data["documented"],
            percent(data["documented"], data["commands"]),
            len(data["missing_commands"])),
        "decompiled functions=%d (denominator unknown)" %
        data["decompiled_functions"],
        "rtti vtables=%d slots=%d" % (data["vtable_rows"],
                                      data["slot_header_rows"]),
        "proto schemas=%d messages=%d fields=%d" % (
            data["proto_schemas"], data["proto_messages"],
            data["proto_fields"]),
        "scene names=%d" % data["scene_names"],
        "",
    ])
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def write_header(path, data):
    pairs = [
        ("kCoverageVerifiedKeys", data["resolved"]),
        ("kCoverageCollectedKeys", data["collected"]),
        ("kCoverageUnresolvedKeys", data["unresolved"]),
        ("kCoverageVerifiedKeyPercentBp",
         basis_points(data["resolved"], data["collected"])),
        ("kCoverageMitmUnverifiedKeys", data["mitm"]),
        ("kCoverageSweepCandidates", data["sweep_candidates"]),
        ("kCoverageSweepNewMatches", data["sweep_matches"]),
        ("kCoverageSlotStates", data["slot_states"]),
        ("kCoverageSlotComplete", data["slot_complete"]),
        ("kCoverageSlotIncomplete", data["slot_incomplete"]),
        ("kCoverageSlotCompletePercentBp",
         basis_points(data["slot_complete"], data["slot_states"])),
        ("kCoverageRawReports", data["raw_reports"]),
        ("kCoverageHeaderBackedReports", data["header_backed_reports"]),
        ("kCoverageReportPercentBp", basis_points(
            data["header_backed_reports"], data["raw_reports"])),
        ("kCoverageGeneratedHeaders", data["generated_headers"]),
        ("kCoverageGeneratedHeadersDirect", data["generated_direct"]),
        ("kCoverageGeneratedHeadersInHarness", data["generated_in_harness"]),
        ("kCoverageHarnessHeaderPercentBp", basis_points(
            data["generated_in_harness"], data["generated_headers"])),
        ("kCoverageRegisteredCommands", data["commands"]),
        ("kCoverageDocumentedCommands", data["documented"]),
        ("kCoverageDecompiledFunctions", data["decompiled_functions"]),
        ("kCoverageRttiVtables", data["vtable_rows"]),
        ("kCoverageRttiVtableSlots", data["slot_header_rows"]),
        ("kCoverageProtoSchemas", data["proto_schemas"]),
        ("kCoverageProtoMessages", data["proto_messages"]),
        ("kCoverageProtoFields", data["proto_fields"]),
        ("kCoverageSceneNames", data["scene_names"]),
    ]
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
        "// Generated by %s." % TOOL,
        "// Bounded artifact coverage only; no gameplay-completeness estimate.",
    ]
    for name, value in pairs:
        lines.append("inline constexpr u32 %s = %du;" % (name, value))
    lines.extend(["", "// Raw reports with no direct portable-header counterpart."])
    for name in data["unmapped_reports"]:
        lines.append("inline constexpr std::string_view kCoverageUnmappedReport%d = %s;" % (
            data["unmapped_reports"].index(name), cpp_string(name)))
    # Keep the array declaration valid in strict C++ even when the list is empty.
    lines.append("inline constexpr std::string_view kCoverageUnmappedReports[%d] = {" %
                 max(1, len(data["unmapped_reports"])))
    for index in range(len(data["unmapped_reports"])):
        lines.append("    kCoverageUnmappedReport%d," % index)
    lines.extend([
        "};",
        "inline constexpr std::size_t kCoverageUnmappedReportCount = %du;" %
        len(data["unmapped_reports"]),
        "",
        "} // namespace mmu",
        "",
    ])
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))
    data = collect(root)
    write_report(os.path.join(root, "docs", "reconstruction_coverage.txt"),
                 data)
    write_header(os.path.join(root, "core", "include", "mmu", "core",
                              "CoverageSummary.h"), data)
    print("coverage keys=%d/%d states=%d/%d reports=%d/%d" % (
        data["resolved"], data["collected"], data["slot_complete"],
        data["slot_states"], data["header_backed_reports"],
        data["raw_reports"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
