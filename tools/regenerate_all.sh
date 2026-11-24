#!/usr/bin/env bash
# Regenerate every checked-in report/header from MMU.exe and the decoded assets.
# Override EXE/ASSETS/GUNS from the environment if the originals live elsewhere.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

EXE="${EXE:-/Users/mysterypaint/Downloads/MegaManUnlimited/MMU.exe}"
ASSETS="${ASSETS:-/Users/mysterypaint/Downloads/MegaManUnlimited/Assets}"
GUNS="${GUNS:-$ASSETS/guns.mmu}"

for path in "$EXE" "$ASSETS" "$GUNS"; do
    if [[ ! -e "$path" ]]; then
        echo "missing regeneration input: $path" >&2
        exit 1
    fi
done

python3 tools/extract_metadata.py "$EXE"
python3 tools/generate_game_flow_evidence_header.py docs/game_flow.txt core/include/mmu/core/GameFlowEvidenceTable.h
python3 tools/generate_scene_names_header.py tools/scene_names.inc core/include/mmu/core/SceneNames.h
python3 tools/analyze_state_graph.py "$EXE" docs/state_graph.txt core/include/mmu/core/RegisteredStateGraph.h
python3 tools/analyze_enemy_state_machines.py "$EXE" . docs/enemy_state_machines.txt core/include/mmu/core/EnemyStateGraph.h
python3 tools/analyze_enemy_edge_facts.py "$EXE" docs/enemy_edge_facts.txt core/include/mmu/core/EnemyEdgeFacts.h
python3 tools/analyze_flow_edge_facts.py "$EXE" docs/flow_edge_facts.txt core/include/mmu/core/FlowEdgeFacts.h
python3 tools/analyze_pause_weapon_resources.py "$EXE" docs/pause_weapon_resources.txt core/include/mmu/core/PauseWeaponResources.h
python3 tools/analyze_inline_menu_resources.py "$EXE" docs/inline_menu_resources.txt core/include/mmu/core/InlineMenuResources.h
python3 tools/analyze_state_slot_blocks.py "$EXE" docs/state_slot_blocks.txt core/include/mmu/core/StateSlotBlocks.h
python3 tools/analyze_boss_damage.py "$EXE" "$GUNS" . docs/boss_damage.txt core/include/mmu/core/BossDamageParams.h
python3 tools/analyze_damage_wrappers.py "$EXE" . docs/damage_wrappers.txt core/include/mmu/core/DamageWrapperTable.h
python3 tools/analyze_camera_evidence.py . docs/camera_evidence.txt core/include/mmu/core/CameraEvidenceTable.h
python3 tools/analyze_player_spawn_evidence.py "$ASSETS/levels.mmu" tools/scene_names.inc docs/player_spawn_evidence.txt core/include/mmu/core/PlayerSpawnEvidence.h
python3 tools/analyze_player_control_evidence.py . docs/player_control_evidence.txt core/include/mmu/core/PlayerControlEvidenceTable.h
python3 tools/analyze_stage_select_grid.py "$EXE" . docs/stage_select_grid.txt core/include/mmu/core/StageSelectGrid.h
python3 tools/analyze_wily_attack_selector.py "$EXE" docs/wily_attack_selector.txt core/include/mmu/core/WilyAttackSelector.h
python3 tools/analyze_completion_cutscenes.py "$EXE"
python3 tools/analyze_outro_handoff.py "$EXE"
python3 tools/crack_hash.py "$EXE" "$ASSETS"
python3 tools/generate_verified_key_names.py docs/key_names.txt core/include/mmu/core/VerifiedKeyNames.h
python3 tools/analyze_rtti.py "$EXE" --all > docs/vtables.txt
python3 tools/generate_rtti_class_names_header.py docs/rtti_classes.txt core/include/mmu/core/RttiClassNames.h
python3 tools/generate_rtti_vtable_header.py docs/vtables.txt core/include/mmu/core/RttiVtables.h
python3 tools/mitm_key_names.py "$ASSETS"
python3 tools/generate_key_name_candidates.py docs/key_names_mitm.txt core/include/mmu/core/KeyNameCandidates.h
python3 tools/generate_decomp_index.py . core/include/mmu/core/DecompIndex.h
python3 tools/generate_proto_index.py . core/include/mmu/core/ProtoIndex.h
python3 tools/sweep_key_names.py "$ASSETS"
python3 tools/generate_key_sweep_summary.py docs/key_names_discovered.txt core/include/mmu/core/KeySweepSummary.h
python3 tools/generate_coverage_report.py

echo "ALL_GENERATORS_DONE"
