# Mega Man Unlimited — portable reimplementation (milestone 11)

Native, readable C++ reconstruction of the 2015 Windows fangame. No Wine, no
protobuf runtime, no SDL 1.2 dependency in shipped code. The original `.mmu`
protobuf databases are decoded offline into a flat `assets.pak`, which both the
macOS host build and the Switch 2 build read directly.

**Important:** the current visible loop is a **debug harness**, not the
reconstructed game. It exists to test imported assets and recovered state
machines. The real top-level game state machine, menus, stage flow, HUD logic,
cutscenes, bosses, and weapon/damage tables are still being recovered from
`MMU.exe`.

## What works today

- **Static recompilation (`mmu_recomp`)**: the original MMU.exe plus its
  MSVC, SDL and Vorbis DLLs are translated to C and run on SDL2, aiming at
  bit-exact Windows behaviour. It boots and plays the intro sequence; see
  `docs/recompilation.md` for usage, debug switches and open issues.
- **MegaMan's movement is reconstructed from MMU.exe**, not approximated:
  Stand, Step, Run, Fall (jump, jump-cut, air control, landing buffers), Slide,
  Ladder, LadderOut and Hurt follow the original state callbacks, use the
  shipped `guns.mmu` GameParams (move 90, jump -325, gravity 990, ...), and
  collide **per pixel** through the engine's colour-coded collision types and
  pixel-stepping sweep mover. MegaMan is drawn with his real animations. See
  `docs/player_reconstruction.md`; shooting/weapons, object collision, camera
  and save/load are still open.
- All eight original `.mmu` assets decode natively (1,725 sprites, 5,853
  animation frames, 791 scenes, 4,926 spawns, 316 game objects, 346 sounds,
  80 cutscenes, 7,513 cutscene commands, 8 standalone surfaces).
- SDL 2 front-end with a 60 Hz **debug harness** (evidence-backed title and
  pause menus driven by `GameFlowRuntime`, recovered `MenuInGame` rows,
  title → MainMenu → `MenuStageSelect` → playable packed-scene gameplay handoff, global
  keyboard/gamepad input, 256x224 viewport, diagnostic bars, scene progression
  and game-over/retry) and Dear ImGui debug tools toggled with
  **F1**: console, asset inspector, sprite viewer (animation playback,
  collision overlays), audio panel (273 raw PCM SFX, 73 Ogg Vorbis music
  tracks), level viewer (791 named scenes, 4,926 spawns), and a harness view.
  The harness drives a fixed-step world with follow camera, animated
  backgrounds, a **state-driven MegaMan controller** (28 recovered states,
  18 message IDs), **packed collision geometry** for all 791 scenes, and an
  **enemy/boss state layer** reconstructed from recovered vtables. The ImGui
  layer has a 2s2h-inspired touch profile with 2x/3x/4x widget scaling and
  gamepad navigation; it is debug chrome, not the game's UI. The top menu bar has
  **Menu** (harness/profile info, **Reset Window Layout**, **Quit Game** with a
  controller-owned yes/no warning), **Debug Tools** (**Drag Player**: right-click
  and drag in the game view to move MegaMan) and **View**; **ZL+R3** edge-toggles that menu without held-key repeat. Window
  defaults follow the renderer output, and the host window opens larger on
  high-resolution displays. Builds on macOS and cross-compiles to Switch 2
  (libounce).
- Dependency-free proto2 wire reader, asset decoders, and platform-neutral
  world simulation, level-art/collision resolution, player and enemy state
  machines in `core/`.
- Offline baker producing a portable packed archive with pixel-format metadata,
  palettes, audio blobs, **sprite/animation, scene/spawn, and level-name tables**.
- Ghidra 12.2 analysis of `MMU.exe` recovered 347 MSVC vtables and 404
  decompiled functions (9,127 vtable slots, up to 64 per table). The ordered
  per-class state graphs are recovered directly from constructor registrations:
  238 constructor regions across 237 classes, 671 registered states, and
  1,446 observed setter/cue edges. Their
  initial states now drive the flow runtime (menu intros are real states, not
  cosmetic waits). Static menu item objects and the global action dispatcher
  yielded 26 item-table suppliers, 254 fixed slots across 15 menus, 32 action
  codes, and 23 screen destinations; the runtime now selects rows, skips blank
  spacers, and emits real action/label/destination events. See
  `docs/gameplay_reconstruction.md` for the virtual layout, MegaMan's message
  IDs, enemy state names, and the Telly callback reconstruction.

## Layout

```
proto/                  exact proto2 schemas recovered from MMU.exe
tools/
  extract_protos.py     extract embedded .proto descriptors from MMU.exe
  inspect_assets.py     exploratory asset probing
  classify_assets.py    top-level schema classification
  bake_assets.cpp       original .mmu -> assets.pak (MMUPAK v5)
  extract_metadata.py   MMU.exe -> scene_names.inc + docs/rtti_classes.txt
  scene_names.inc       generated table of 791 editor-side .xscn paths
  generate_scene_names_header.py SceneNames.h from scene_names.inc
  analyze_rtti.py       PE RTTI -> vtable map (--all for docs/vtables.txt)
  disasm.py             capstone function/vtable inspector
  crack_hash.py         resolves the 0x43c010 CRC-32 GameParam key dictionary
  generate_verified_key_names.py VerifiedKeyNames.h from verified key report
  generate_rtti_vtable_header.py RttiVtables.h from docs/vtables.txt
  generate_key_name_candidates.py KeyNameCandidates.h (unverified only)
  generate_decomp_index.py DecompIndex.h from offline decompilation reports
  generate_proto_index.py ProtoIndex.h from proto/*.proto
  generate_key_sweep_summary.py KeySweepSummary.h (counters only)
  generate_coverage_report.py reconstruction_coverage.txt + CoverageSummary.h
  sweep_key_names.py    token-dictionary sweep for additional key names
  mitm_key_names.py     CRC-linearity sweep over curated gameplay vocabulary
  extract_gun_params.py scan MMU.exe GameParam getter call sites per vftable
  analyze_boss_damage.py raw boss-meter depletion key branch + guns.mmu value
  analyze_damage_wrappers.py raw callers of the two generic damage wrappers
  analyze_camera_evidence.py raw Camera/CameraTarget slots, states + operands
  analyze_player_spawn_evidence.py exact decoded spawn-token checks
  analyze_player_control_evidence.py raw control states, keys + edges
  analyze_wily_attack_selector.py WilyMachine phase jump table + counter clears
  analyze_enemy_state_machines.py 13 RTTI enemies: states, thunk slots + caller join
  analyze_enemy_edge_facts.py verbatim enemy edge branch operands + key strings
  analyze_flow_edge_facts.py verbatim menu edge branch operands + key strings
  analyze_pause_weapon_resources.py exact inline PauseMenuStatus resource pushes
  analyze_inline_menu_resources.py exact .xspr pushes in 20 *Menu constructors
  analyze_state_slot_blocks.py raw thunk/slot blocks for all 671 states
  resolve_gun_objects.py map constructors to decoded .xgun objects
  state_callback_report.py summarize recovered state callbacks
  analyze_state_graph.py constructor state order + observed setter edges
  analyze_menu_items.py static menu labels/actions -> item tables
  analyze_menu_actions.py action dispatcher -> terminal screen table
  analyze_continue_stages.py CONTINUE stage ids -> level classes
  analyze_stage_select_grid.py 3x3 stage-select grid + center-cell raw inputs
  analyze_level_setup.py literal level-constructor setup and cue evidence
  analyze_manager_fields.py raw manager/global field writes and accessors
  analyze_outro_handoff.py raw outro/handoff call sites and literal hashes
  analyze_cutscene_keys.py cutscenedefs.mmu keys -> original .xcut paths
  analyze_completion_cutscenes.py stage-completion flags and .xcut branches
  analyze_boot_cutscenes.py manager constructor boot mask and cutscenes
  analyze_save_flags.py   keyed save-flag writers, readers and raw masks
  analyze_activity_masks.py  raw FUN_00507e90 setter sites and mask evidence
  analyze_activity_mask_readers.py raw FUN_00507ec0 getter tests and sites
  generate_game_flow_evidence_header.py verbatim game_flow.txt string table
  generate_rtti_class_names_header.py verbatim RTTI class-name string table
  generate_coverage_report.py bounded artifact coverage report + header
  DecompileTargets.java Ghidra headless decompiler batch script
  DecompileAll.java     Ghidra headless: decompile every function (+ register
                        signatures, false no-return repair); see
                        docs/player_reconstruction.md
  register_signatures.txt 155 helpers that take register arguments
  re_annotate.py        annotate decompiled C/disassembly with constant values,
                        strings and hash names; CRC-32 helper

The Python analyzers are standalone, not CMake targets. With the original
sources at `$HOME/Downloads/MegaManUnlimited`, run
`tools/regenerate_all.sh` to regenerate every checked-in raw-evidence report
and generated header in the repository's documented order. It accepts `EXE`,
`ASSETS` and `GUNS` environment overrides. The explicit command list is:

```sh
EXE=$HOME/Downloads/MegaManUnlimited/MMU.exe
ASSETS=$HOME/Downloads/MegaManUnlimited/Assets
GUNS="$ASSETS/guns.mmu"
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
```

docs/
  rtti_classes.txt      348 recovered non-protobuf RTTI class names
  vtables.txt           all 347 MSVC vtables (9,127 slots, up to 64 each)
  game_flow.txt          raw dispatcher and screen-flow evidence
  state_graph.txt       238 ordered constructor state graphs and setter evidence
  state_names.txt       raw registration-site state name literals
  state_transitions.txt raw setter-edge candidates (not runtime transitions)
  enemy_state_machines.txt 13 RTTI enemies with thunk slots and caller joins
  menu_items.txt        26 item-table suppliers and 254 recovered rows
  menu_actions.txt      32 action codes and their dispatched destinations
  continue_stages.txt   38 saved stage ids -> level classes
  level_setup.txt       38 constructor setup rows and 33 unique cue strings
  stage_select_grid.txt 8 outer cells, center rule and 4 cursor keys
  wily_attack_selector.txt 4 raw phases, jump table and counter-clear sites
  boss_damage.txt       raw meter-depletion branch and 15 caller rows
  damage_wrappers.txt   4 raw call sites into the 2 generic wrappers
  camera_evidence.txt   18/29 vtable slots, CameraTarget states/edges + operands
  player_spawn_evidence.txt 791 scenes, 4926 spawns, 77 substring candidates
  player_control_evidence.txt 16 states, 17 key hashes, 33 edges, 2 characters
  manager_fields.txt    raw manager literal writes, accessors and consumer fields
  outro_handoff.txt     19 call sites, 16 string VAs and 43 raw facts
  cutscene_keys.txt     80 database keys matched to legacy .xcut paths
  completion_cutscenes.txt  12 stage-completion branches and manager writes
  boot_cutscenes.txt        manager boot mask, paths and manager writes
  save_flags.txt            138 raw writers, reader-key coverage and masks
  activity_masks.txt        24 raw setter sites and six literal mask values
  activity_mask_readers.txt 36 raw getter sites and test forms
  enemy_edge_facts.txt      35 enemy edge callback facts and verbatim operands
  flow_edge_facts.txt       181 menu edge callback facts and verbatim operands
  pause_weapon_resources.txt 44 inline PauseMenuStatus resource pushes
  inline_menu_resources.txt 231 exact .xspr pushes in 20 *Menu constructors
  state_slot_blocks.txt     671 raw state thunk/slot blocks (671 complete)
  reconstruction_coverage.txt generated bounded artifact coverage
  function_inventory.txt    all 16,957 MMU.exe functions (coverage denominator)
  reconstructed_functions.txt ledger of ported functions (full/partial/data)
  player_reconstruction.md  engine timing/input/params, per-pixel collision,
                            MegaMan states, RE workflow and open items
  gameplay_reconstruction.md  virtual layout, MegaMan states and messages
  gun_params.txt       constructor keys grouped by vftable
  key_names.txt        137 verified GameParam key -> name mappings
  key_names_discovered.txt     84651 candidates, new_matches=0
  key_names_mitm.txt          5 exact-hash candidates awaiting verification
  decompiled/           404 decompiled functions (C)
  decompiled_xref/      12 class-specific decompilations and index
  protos/               6 recovered .proto schemas (24 messages, 166 fields)
core/
  include/mmu/core/     public headers (Types, ProtoReader, AssetDatabase,
                        PackedArchive, CutsceneRuntime, World, LevelArt,
                        Collision,
                        PlayerState, EnemyState, GunParams, DebugConsole,
                        GameFlow, FlowEvidence, GameFlowEvidenceTable,
                        RegisteredStateGraph,
                        RegisteredCutscenes, RegisteredStateNames,
                        RegisteredStateTransitions, MenuItemTables,
                        MenuActionTable, ContinueStageTable,
                        CompletionCutsceneTable, BootCutsceneTable,
                        SaveFlagTable, KeyedFlagRuntime, VerifiedKeyNames,
                        KeyNameCandidates, KeySweepSummary, CoverageSummary,
                        RttiClassNames, RttiVtables,
                        DecompIndex, ProtoIndex, SceneNames, ActivityMaskTable,
                        ActivityMaskReaderTable, BossDamageParams, DamageWrapperTable,
                        EnemyEdgeFacts, EnemyStateGraph, FlowEdgeFacts,
                        InlineMenuResources, LevelSetupTable, ManagerFieldTable,
                        OutroHandoffTable, PauseWeaponResources, StageSelectGrid,
                        StateSlotBlocks, WilyAttackSelector)
  src/                  implementations
platform/sdl2/          SDL2 + ImGui front-end (main.cpp, AudioEngine.cpp)
third_party/            vendored stb_vorbis (public domain)
cmake/Switch2Toolchain.cmake
```

## Build with make (all platforms)

```sh
make            # = make all: host build + Switch 2 when devkitPro is installed
make macos      # build-macos/      make linux     # build-linux/
make windows    # build-windows/   make switch2   # build-switch2/*.nro
make test       # host --selftest + flow selftest (non-zero exit on failure)
make run        # build and launch the host binary
make bake ASSETS=/path/to/MegaManUnlimited/Assets   # assets.pak next to the binary
make clean      # removes build-macos/linux/windows/switch2 (never build/)
```

Variables: `ASSETS`, `PAK`, `DEVKITPRO` (default `/opt/devkitpro`), `CONFIG`
(default `RelWithDebInfo`), `JOBS`, `IMGUI`. The raw CMake commands below
still work.

## Build (macOS)

```sh
brew install sdl2        # sdl2-compat provides SDL2 headers/libs
cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew \
  -DMMU_ASSET_DIR=$HOME/Downloads/MegaManUnlimited/Assets
cmake --build build -j8
cmake --build build --target bake_assets   # writes assets.pak next to the binary
./build/platform/sdl2/mmu_sdl2 --selftest
./build/tools/mmu_gameflow_runtime_selftest
./build/platform/sdl2/mmu_sdl2
```

The host runtime looks for `assets.pak` in the executable's own directory (or
an `asset/` child directory). An explicit path argument overrides that. The
`.mmu` files are never needed at runtime.

`--selftest [asset_dir]` decodes every file headlessly and prints counts,
including a probe + full decode of the first Ogg track, plus the baked scene
counts and spawn-type resolution, the first scene's name and background-art
frames, art and collision coverage across all scenes, a deterministic 60-step
world simulation, player damage/recovery and grounded-landing checks, Telly
`Idle → MoveH/MoveV → Idle` transitions (including axis handoff), enemy
contact damage and cutscene image resolution. `--selftest` now returns a
nonzero exit code if any of its 132 assertions fail (including MegaMan's
GameParams, typed collision, landing, run speed and jump height).
`mmu_gameflow_runtime_selftest` is the separate asset-free check for the
reconstructed flow runtime and named-state dispatcher.
`--sprite=<key>` opens the sprite viewer preselected on that key.
`--screenshot=<file.bmp>` saves the last rendered frame when `--quit-after`
ends the run (headless visual checks).
`--sound=<key>` / `--music=<key>` play a sound at startup, `--level=<key|index>`
opens the level viewer on a scene, and `--play=<key|index>` instantiates a
scene in the live world and starts it running. `--quit-after=<seconds>` exits
the event loop cleanly after a delay and prints the console log, so headless
smoke tests can run with a deterministic zero exit code.

A build directory is tied to the absolute source path it was configured from.
If the repository moves, configure a fresh directory (e.g. `build-macos`).

## Build (Linux / Windows)

The code is portable C++17 with SDL2 and Dear ImGui (expected at `../imgui`, or
pass `-DMMU_IMGUI_DIR=...`); no platform APIs are used outside `main.cpp`'s
SDL calls. These configurations have not been verified yet:

```sh
# Linux (Debian/Ubuntu)
sudo apt install libsdl2-dev cmake g++
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMMU_ASSET_DIR=/path/to/Assets
cmake --build build-linux -j

# Windows (Visual Studio 2022 + vcpkg)
vcpkg install sdl2:x64-windows
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
      -DMMU_ASSET_DIR=C:/path/to/Assets
cmake --build build-win --config RelWithDebInfo
```

## Gameplay controls (harness)

Keyboard follows the original `options.xopts` defaults where possible: arrows
or WASD move, **Z**/Space jump, **Left Shift** (or Ctrl/C) fire, **Left Alt**
(or X) slide, Down+Jump also slides, Esc/P pauses. Gamepad: d-pad/left stick,
confirm button jumps, cancel button slides, X fires. The controller derives
pressed/released edges itself, so all inputs are reported as held state.

## Build (Switch 2)

```sh
make switch2                       # sources $DEVKITPRO/switch2vars.sh itself
make switch2 PAK=/path/to/assets.pak
```

or by hand:

```sh
source /opt/devkitpro/switch2vars.sh
cmake -S . -B build-switch2 \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/Switch2Toolchain.cmake" \
  -DMMU_PAK=/absolute/path/to/assets.pak
cmake --build build-switch2 -j8
```

`MMU_PAK` copies just that archive into `build-switch2/platform/sdl2/romfs/`,
and `elf2nro` embeds that folder (≈430 MB `.nro`). The app mounts it at
startup via `romfsMountSelf("romfs")`. Without `MMU_PAK` the `.nro` builds
with no embedded assets. **Do not** point the older `MMU_ROMFS_DIR` at the
repository or a build folder: everything inside gets embedded (that produced
a 2.4 GB `.nro`). `make switch2` always clears it. Output:
`build-switch2/platform/sdl2/mmu_sdl2.nro`.

Notes:
- Use an **absolute** `-DCMAKE_TOOLCHAIN_FILE` path. A relative path cached in a
  reused build directory can silently stop applying the toolchain, producing
  host-compiler/link errors.
- The Switch 2 build does **not** need Homebrew. It was verified from a clean
  build tree with `-DCMAKE_IGNORE_PREFIX_PATH=/opt/homebrew`.
- `-lEGL` must resolve to OunceVK's tiny shim in `portlibs/switch2/lib`, which
  is why the toolchain puts that directory before `portlibs/switch/lib`. Using
  the full Switch 1 `libEGL.a` pulls in unresolved `nouveau_pushbuf_space`
  symbols. The toolchain links by library name to preserve this ordering.

## Asset pipeline

```sh
./build/tools/mmu_bake build/platform/sdl2/assets.pak /path/to/MegaManUnlimited/Assets
# or: cmake --build build --target bake_assets
# smaller development archive:
./build/tools/mmu_bake assets.pak /path/to/MegaManUnlimited/Assets \
  menus.mmu --max-images 8
```

`assets.pak` (magic `MMUPAK` v6; v2–v5 files still load with less metadata;
v6 adds the `guns.mmu` GameParam table, and frames without their own collision
surface are linked to the sprite's `single_collision`) is a
little-endian 128-byte header followed by tables of 136-byte image/audio
entries, 24-byte animation frames, 40-byte sprites, 24-byte scenes, and
32-byte spawns, then a string table and a blob area. Frames store
image/collision entry indices, duration, and x/y offsets; sprites store key,
handle, source file, and a frame range. Audio entries carry the original byte
count and volume. Each scene stores its resource key, a contiguous range of
spawns, and a string-table offset to its editor-side `.xscn` path (v4 only);
each spawn stores x/y/auto-respawn floats, five boolean flags packed into a
bitfield, and string-table offsets for its `type`, `attr1`, and `attr2`.
Images retain raw SDL pixel rows, pitch, bit depth, channel masks/shifts/losses,
and palettes, so the runtime can convert to RGBA identically on any platform.
The archive reader is `core/src/PackedArchive.cpp`; no protobuf runtime is
linked. Images are uploaded lazily as SDL textures, so only sprites actually
viewed consume GPU memory.

## Debug console

In the ImGui window, type `help` for the command list. Available commands:
`help`, `clear`, `stats`, `ui [desktop|touch]`, `load <file.mmu>`,
`sprites [n]`, `sprite <key>`,
`pak [file] [n]`, `pakimage <index>`, `audio`, `sfx <key>`, `music <key>`,
`stopsound`,
`scenenames [summary|list [filter]|path <text>|index <n>]`,
`levels [n]`, `level <key|#index>`, `play <key|#index>`, `types`,
`state [name]`, `states`, `event <id> [value]`, `hurt [sourceX]`, `heal [amount]`,
`collision`,
`cutscene [list|keys [list|key <hex>|path <text>]|start <key|#index|suffix>|status|stop|step <sec>|events]`,
`enemies [n]`, `enemy <index> [Idle|MoveH|MoveV|Wait|Death]`, `projectiles`,
`flow [screens|levels|name|#index]`,
`flowraw [summary|line <n>|find <text>]`,
`flowrun [status|reset|screen|supplier|flags|manager|continue|complete|boot|keyflag|masks|mask|maskreads|maskread|confirm|cancel|up|down|left|right|step|cue|levelcue|events]`,
`flowboot [summary|writes|play|pending]`,
`flowcontinue [summary|list|stage <id>|class <name>|ctor <va>]`,
`flowlevel [list|cues|stage <id>|ctor <va>|loop <cue>|intro <cue>|apply <id>]`,
`flowgrid [summary|cells|keys|center <0|1> <count>|stage <x> <y> <0|1> <count>]`,
`flowcomplete [summary|rows|flag <id>|state <hex> [0|1]|pending]`,
`flowmgr [summary|writes|accessors|accesses|offset <hex>|accessor <va>|func <va>|write <offset>]`,
`saveflags [summary|writers|writer <va>|mask8|mask4|readers|key <n>]`
(with `flags` accepting `19=`, `1a=`, `59=`, `pause=`, `cut=` and the
action-`0x06` gate; `keyflag <key> <0|1>` exercises a recovered writer key),
`flowmenu [items [class]|actions [code|kind]]`, `flownames [filter]`,
`flowtrans [all|to <name>|class <name>]`,
`flowgraph [initial|to <name>|<class>]`,
`enemygraph [initial|slots|to <name>|damage|<class>]`,
`enemyfacts [summary|constants <class>|call <va>|<class>]`,
`flowfacts [summary|constants <class>|call <va>|<class>]`,
`bossdamage [summary|calls|owner <name>|call <va>|bits <hex>]`,
`camera [summary|slots|target|states|edges|operands|func <va>]`,
`playerspawn [summary|checks|candidates|scene <n>|exact <token>]`,
`playerctl [summary|states|keys|edges|chars|resources|operands|state <name>|key <name>|func <va>]`,
`keynames [summary|list|key <hex>|name <text>]`,
`coverage [summary|keys|states|reports]`,
`keycandidates [summary|list|key <hex>]`, `keysweep [summary]`,
`gunparams [summary|objects|object <name>|key <hex>|lookup <object> <key>]`,
`wily [summary|phases|phase <n>|raw]`,
`pauseres [summary|list|key <hex>|path <text>|kind <Icon|Colors>]`,
`menures [summary|list|class <name>|key <hex>|path <text>]`,
`stateslots [summary|class <name>|state <name>|incomplete]`,
`rtti [summary|classes [filter]|class <name>|vftable <hex>|slot <hex>]`,
`rtticlasses [summary|list [filter]|name <text>]`,
`decomp [summary|list [filter]|func <va>|targets [kind]|xrefs|xref <class>]`,
`protos [summary|list|schema <name>|messages <name>|message <schema> <name>|field <name>]`,
`outro [summary|calls|call <va>|owner <name>|strings|facts|func <va>|offset <hex>]`,
`quit`.
For headless verification, pass `--exec="flowrun status"` (repeatable) to run
console commands at startup; its output appears in the normal log/smoke output.
The console API itself lives in
`core/include/mmu/core/DebugConsole.h` and is platform-agnostic.

## Sprite viewer

Open with **View → Sprite Viewer**, or select a sprite with `sprite <key>` in
the console. The left pane lists all 1,725 packed sprites (filterable); the
right pane shows the current frame at an adjustable scale, plays the animation
using each frame's recorded duration, tints the collision mask in red, and
draws a green crosshair at the sprite handle. Use the Frame slider to scrub and
Play/Collision checkboxes to toggle overlays. Collision is **off by default**
because its red tint obscures the real colors; only ~34% of frames have a mask
anyway. `res_root` is absent from the shipped assets, so sprites are labeled by
their first frame's image path. Pixel conversion uses the stored SDL channel
masks/shifts/losses (and palettes for 8-bit images); textures use
`SDL_PIXELFORMAT_RGBA32`, the endian-safe RGBA byte-array format.

## Level viewer

Open with **View → Level Viewer**, or select a scene with `level <key>` (or
`level #<index>`) in the console. The left pane lists all 791 baked scenes;
the right pane draws their spawns on an 8-pixel grid with the original 256x224
logical screen marked in yellow. Drag to pan and use the wheel to zoom. Markers
show the exact spawn point (yellow = collectible, blue = facing right), and the
spawn table lists every object with its type and coordinates. Hovering a marker
shows its flags, auto-respawn delay, and `attr1`/`attr2`.

The `.mmu` schema has no tilemap, background reference, or camera data — a
scene is only a key plus a flat spawn list — so exact level geometry is still
unavailable. However, the 791 editor-side `.xscn` paths recovered from MMU.exe
are baked into v4 archives, giving every scene a real name (e.g.
`\Levels\YoyoMan_Section_1.xscn`). `mmu::resolveSceneArt()` matches those names
against packed `levelart.mmu` paths, so 738 of 791 scenes now draw their actual
animated background art. Object art is resolved heuristically by matching each
spawn's `type` string against packed image paths (55 of 82 types resolve).
`types` in the console reports the spawn-type resolution table.
`scenenames [summary|list [filter]|path <text>|index <n>]` lists the exact 791
recovered `.xscn` literals from `tools/scene_names.inc` without assigning them
runtime keys.

Scene keys themselves remain opaque: no CRC-32, FNV, or Murmur variant tested
against the shipped path/key pairs reproduces them. The best explanation is
that the baker hashed a canonical resource name that was stripped from the
shipped `.mmu` files. `tools/extract_metadata.py` documents the recovery of the
path table, and `docs/rtti_classes.txt` lists the 348 non-protobuf RTTI class
names recovered from the same binary.

## Debug harness / world simulation

The executable starts in a deliberately labelled **DEBUG HARNESS - NOT THE GAME**
card: press Enter/A to run the harness. Arrows or A/D move, Space/Z jumps,
S/Down climbs ladders, X/Shift dashes, LCTRL/C fires, and Escape/P pauses.
Completing a scene loads the next one; death retries after a short game-over
beat. A full-screen boot splash shows staged loading progress while the baked
archive is read. **F1** toggles the ImGui debug tools. **Menu → Quit Game** opens
an enlarged warning where Dpad/left stick choose, physical A confirms and
physical B cancels; the warning consumes those inputs so the harness does not
also act on them. On Switch the physical A/B buttons are mapped to the matching
SDL face constants explicitly (the console backend reports them by Xbox
position), so the prompt and the input agree. **Menu → Reset Window
Layout** reapplies resolution-aware positions and sizes, and **ZL+R3** hides or
shows the top menu on a press edge. The console's `play <key|#index>` command
still loads a scene and hands it to the harness, and **View → Harness View**
remains available as a debug camera/label viewer. None of this reproduces the
original game's menus, rules, or presentation.

A baked scene is loaded into `mmu::World` (`core/include/mmu/core/World.h`), a
dependency-free simulation of one scene's live objects. Every `PackedSpawn`
becomes a `WorldObject` with velocity, flags, and resolved art; a fixed 60 Hz
step integrates positions and drives a follow camera clamped to the scene's
spawn-derived bounds. Movement is filtered through `mmu::PlayerController`
using the 28 state names and 18 message IDs recovered from `MMU.exe`: damage
enters `Hurt` with knockback and invincibility, stun/glue/death lock controls,
and grounded movement selects `Stand`/`Step`/`Run`. The canvas renders the same
art as the level viewer with camera-relative coordinates plus animated
background frames. Every scene also gets a packed collision mask: all 791
scenes resolve to a collision image, including eight 24/32-bpp maps decoded via
their stored channel masks. 8-bit indexed maps use white = solid, red = solid
hazard, black = open. The player uses a ~7x12 pixel box with axis-separated
resolution and lands on the original room floor instead of the old
spawn-height approximation.

Each spawn with a recognized enemy type also gets an `mmu::EnemyController`
(`core/include/mmu/core/EnemyState.h`). Telly, EZBlock, MineE, SawPole and
SuperCannon now use state names and transitions recovered from their vftables,
including Telly's `Idle`/`MoveH`/`MoveV` axis handoff, EZBlock's `Wait`/`Idle`
toggle, MineE's `Scan`/`Explode`/`Death` chain, SawPole's bounded `Patrol`, and
SuperCannon's `Stand`/`Turn`/`Attack` countdown with recovered 1.8 s fire
interval. Boss controllers cover the base `Boss`, `MidBoss`, `WilyBoss`,
`WilyMachine` and `WilyCapsule` vtables. The base Boss starts in `Spawn`, fills
an integer HP meter from 0 to 28 over 2.24 s in `Pose`, and exposes an attack
hook in slot 29; WilyBoss starts in `Idle` and enters `EnergyFill`. The machine
starts in `Wait` (its constructor's final `setState` at `0x56eb95`), and its
selector cycles `PillarDrop`, `Lightning`, `Attack3` and `Ram` through the four
`GotoAttackN` setup states. `docs/gameplay_reconstruction.md` records the
slot-to-callback mapping, the 186 px/s ram speeds and the resolved GameParam
intervals. Six more concrete archetypes now run their recovered cycles:
ExplosivePlatform (`Idle`/`Flash`, 0.8 s fuse then a visible explosion),
YokuBlock/YokuSpike (`Wait`/`Appear`/`Idle`), the Met family
(`MetStand`/`MetSwim`/`MetLaser`/`MetTank` with hidden hitboxes), CrossShooter and
SpikePlatform (`Wait`/`Shoot` turrets), ShieldAttacker (`Init`/`Move`/`Turn`
with two shots per volley), and CannonBall (six-state `Up`/`Down` cycle with
shots in both firing states). These emit real `EnemyShot` projectiles aimed at
the player. WilyMachine callbacks emit real `WilyMachinePillar`,
`WilyMachineLightning`, `WilyMachineShot` and `WilyMachineBlock` objects. The
machine spawns one persistent pillar in `Wait`, cues it with `0xf` on
`PillarDrop` and `0x20` from `Crush`; the pillar then fires three blocks at
`(-10,-10)`, `(-10,-26)`, `(-10,-42)`. Gun objects decoded from `guns.mmu` give
shots speed `250`/damage `4`, blocks speed `230`/damage `4`, lightning damage
`6` over `1.6 s`, and pillar damage `6`. The three shots are emitted on
`GotoAttack3` enter with `1/2/3 s` delays; `Attack3` lasts `0.75 s`, and
`Hover`/`GotoAttack1` steer toward player-relative `(128,63)`/`(90,80)` targets.
The capsule's `FlyUp`/`Wait`/`FlyDown` transitions are position-conditional rather
than timer-driven. Numeric parameters that could be tied to a hashed key are
read from `core/include/mmu/core/GunParams.h`, generated from `MMU.exe` and
`guns.mmu` by `tools/resolve_gun_objects.py`.
`gunparams [summary|objects|object <name>|key <hex>|lookup <object> <key>]`
lists the six decoded objects and their exact key/value rows without assigning
a gameplay role to an inferred object owner.
`tools/analyze_boss_damage.py` recovers the raw meter-depletion branch: helpers
`0x420380`/`0x420690` subtract a computed amount from `+0x150`/`+0x160`, and at
zero read GameParam `0xa96bff8d` and compare it with the `1.0` constant at
`0x697ce8`. The only decoded value for that key is `1.0` (object `0xebecc443`),
so equality selects raw code `4`, which `0x420450` dispatches to a spawn plus
`FUN_004047a0(1)`. The report in `docs/boss_damage.txt` keeps the helper
instructions and all 15 callers; 11 resolve through RTTI vftable stores to
LuggageMan, Skoripion, ChickenBomb, BulletTrap, SniperJoe, TankJoe,
GrenadeSoldier, MineP, MineE, Leprechaun and LaserSpreader, with each caller's
compared result codes preserved as raw bits; `bossdamage [summary|calls|owner <name>|call <va>|bits <hex>]`
lists those caller rows.
`tools/analyze_damage_wrappers.py` joins the callers of those two wrappers using
the same owner rules: `docs/damage_wrappers.txt` / `DamageWrapperTable.h` keep
four raw call sites (`0x5858e5`, `0x590675`, `0x5a6f5f`, `0x56438f`), their
calling handlers, vftable/slot evidence and compared result-code bits. Three
callers resolve (Sweeper, Leprechaun, Bullet); `0x5a6ef0` remains unresolved
because no unique vftable owns it. No wrapper owner is guessed from a shared
base handler. The priority player/camera artifacts are raw-only:
`tools/analyze_camera_evidence.py` joins Camera/CameraTarget vtable slots,
state blocks and verbatim operand lines without inferring scroll speed or room
destinations; `camera [summary|slots|target|states|edges|operands|func <va>]`
exposes them. `tools/analyze_player_spawn_evidence.py` decodes levels.mmu with
the recovered `GameSpawnObject` schema and confirms that no exact `MegaMan`,
`ProtoMan`, `Bass`, `Zero` or `player` type exists; the lowercase `megaman`
substring matches only the distinct `ShadowMegaMan` records, so World's proxy
now uses exact token equality instead of that substring.
`playerspawn [summary|checks|candidates|scene <n>|exact <token>]` lists the raw
checks. `tools/analyze_player_control_evidence.py` joins registered state
hashes, verified parameter names, playable-class state edges, character-select
actions and verbatim virtual operands without deriving timing; `playerctl
[summary|states|keys|edges|chars|resources|operands|state <name>|key <name>|func
<va>]` exposes that table. Current physics constants and the slide/shoot hooks
remain explicitly labelled unverified placeholders where no exact numeric
evidence is available. Enemies are stepped in the same
fixed loop as the player, have contact damage, and can be inspected/forced with
`enemies`, `enemy <index> [state|hurt N]`, `enemy states` and `projectiles`.
The simulation has no SDL or ImGui dependency, so it compiles on any target;
`--selftest` reports 127 explicit PASS/FAIL checks covering the recovered flow
catalogue, stage-handoff scene loading and player input acceptance, raw camera,
player-spawn and player-control evidence tables, player damage/recovery,
collision coverage (791/791 loadable),
every reconstructed enemy/boss transition (including all six new archetypes),
projectile spawning/homing, cue routing, block volleys, contact damage and
cutscene sprite resolution. The
separate `mmu_gameflow_runtime_selftest` reports 155/155 flow checks.

### Touch/debug UI profile

The ImGui debug layer uses a 2ship2harkinian-style touch profile on Switch 2 and
other touch targets: `SDL_HINT_TOUCH_MOUSE_EVENTS` routes SDL finger input to
ImGui as touch-tagged mouse events, `ImGuiConfigFlags_IsTouchScreen` and
`NavEnableGamepad` are enabled, and the style is scaled 2x at 720p, 3x at
1080p/Switch 2, and 4x at 4K. The default embedded font is the scalable vector
variant at 13px, multiplied by 1.5x on 1080p and higher outputs; padding,
spacing, scrollbars, and grab handles are scaled with the style. Use
`--touch-ui` / `--desktop-ui`, `MMU_TOUCH_UI=1` / `MMU_DESKTOP_UI=1`, or the
console command `ui touch` / `ui desktop` to override the detected profile.
On Switch hardware the debug UI also uses the Nintendo Standard shared font and
opens the system Swkbd for focused ImGui text fields. The submit key is labelled
**Send** and the applet is prefilled with the field's current contents so typing
is never cleared by accident. After the keyboard closes (Send or B) the field is
de-focused and the request is latched, so the closing press cannot immediately
reopen the applet. The debug console also has an on-screen **Send** button next
to its command field; it executes the field verbatim, and Send from the Swkbd on
that field executes the typed command directly instead of only inserting it.
Cancelling with B leaves the field's prompt untouched, so reopening the applet
restores it. This affects only the debug UI; the game's own UI is not
implemented yet.

## Audio

`platform/sdl2/src/AudioEngine.cpp` opens one SDL audio device (44.1 kHz,
16-bit, stereo) and mixes all voices in a single callback. SFX are the original
raw 16-bit PCM from `guns.mmu` (no WAV header; the `.wav` paths are only labels)
and music is Ogg Vorbis decoded by vendored `stb_vorbis`. Both are resampled to
the device format, and music loops. No `SDL2_mixer` dependency is needed, so
host and Switch 2 use the same code. Music voices are activated when queued, so
Ogg loops mix on both targets; the headless selftest checks that path. Open
**View → Audio** to browse and play sounds, adjust master volume, or stop
playback.

## Measured coverage

`tools/generate_coverage_report.py` recomputes
`docs/reconstruction_coverage.txt` and `CoverageSummary.h` from the generated
artifacts. These are bounded artifact ratios, **not** a gameplay-completeness
percentage: the total number of functions, rules, or in-game behaviours in
`MMU.exe` is not recovered, so there is no honest global denominator.

| Metric | Covered | Denominator | Percent | Remaining |
|---|---:|---:|---:|---|
| Verified key names | 137 | 490 | 28.0% | 353 unresolved; 5 MITM candidates unverified; 84,651 sweep candidates, 0 new matches |
| State slot blocks | 671 | 671 | 100% | 0 |
| Raw reports with portable headers | 36 | 36 | 100% | 0 |
| Generated headers reachable from SDL harness | 39 | 39 | 100% | 0 (37 direct; 2 via `FlowEvidence.h`) |
| Registered commands documented | 64 | 64 | 100% | 0 |

Counts without a known denominator are reported separately: 404 decompiled
functions, 347 vtables / 9,127 slots, 6 proto schemas / 24 messages / 166
fields, and 791 editor-side scene paths. `coverage [summary|keys|states|reports]`
prints the same generated counts at runtime.

## Status / next milestones

Done: schemas, asset classification, native decoders, packed archive (with
sprite/animation/audio/scene/name and decoded cutscene metadata), asset-indexed sprite viewer,
software audio mixer with Ogg music, named scene/spawn level viewer with real
background art, platform-neutral world simulation with camera and fixed-step
updates, Ghidra RTTI/vtable map and decompiled gameplay functions, state-driven
MegaMan controller, packed collision geometry for all scenes, exact enemy state
machines (Telly, EZBlock, MineE, SawPole, SuperCannon, Flasher, Electrode,
ExplosivePlatform, YokuBlock/YokuSpike, four Met variants, CrossShooter,
SpikePlatform, ShieldAttacker and CannonBall) plus five boss machines,
recovered gun-parameter tables and the resolved 0x43c010 key dictionary, a
labelled debug harness with global input and diagnostic bars, F1-toggled ImGui
tools with a touch-friendly profile, 127 harness selftest assertions, 155
independent game-flow runtime checks, host and
Switch 2 builds, and a deterministic headless smoke-test flag.

`tools/analyze_game_flow.py` writes `docs/game_flow.txt`, a conservative
vtable/callback evidence report; the data-only catalogue lives in
`core/include/mmu/core/GameFlow.h` (`flow` command). The executable menu-level
reconstruction is `core/include/mmu/core/GameFlowRuntime.h` (`flowrun` command),
with `FlowLevelPhase` values recovered from `BaseLevel +0xb0`. The original
named-state dispatcher is reproduced by
`core/include/mmu/core/StateDispatcher.h`, including the exact CRC-style hash
from `0x43c010`. `tools/analyze_state_names.py` recovers the 266 original state
names from 671 registration calls into `docs/state_names.txt` and
`core/include/mmu/core/RegisteredStateNames.h`; the runtime mirrors its states
through the dispatcher and exposes the hashed name in `flowrun status`.
`tools/analyze_state_transitions.py` adds 222 observed setter calls (67
`setState`, 155 cues) in `docs/state_transitions.txt` and
`core/include/mmu/core/RegisteredStateTransitions.h`. They are queryable from
`flowtrans [all|to <name>|class <name>]` or the **View → Flow Evidence** debug
panel. `tools/analyze_state_graph.py` now recovers constructor registration
order, initial states, and observed setter edges for 238 constructor regions
across 237 classes (including one-state enemies and projectiles) into
`docs/state_graph.txt` and `core/include/mmu/core/RegisteredStateGraph.h`;
`flowgraph [initial|to <name>|<class>]` queries them. Those initial states now
drive `flowrun`, so title/main/stage-select/shop intros are real registered
states rather than cosmetic waits.
`tools/analyze_enemy_state_machines.py` joins the thirteen RTTI-verified
boss-damage caller owners to their constructor states and setter/cue edges by
exact class name and vftable VA, producing `docs/enemy_state_machines.txt` and
`EnemyStateGraph.h` (28 states, 35 edges). `MetTrois` and `SpreadMine` are
resolved by unique RTTI vftable membership because no preceding vftable store
exists for their damage handlers. The tool also reads `MMU.exe` directly to
recover the three constructor thunk slots assigned to each registered state
(28 state/slot blocks), so each setter/cue edge is attributed to the state
whose block contains its vtable slot; slot 3 remains unattributed. Those
initial states now drive the portable enemy controllers, and
`enemygraph [initial|slots|to <name>|damage|<class>]` queries the raw join
without inventing timing or transition triggers.
`tools/analyze_enemy_edge_facts.py` reads those 35 setter/cue callbacks from
`MMU.exe` and records 126 comparison/test operands, 15 referenced qword
constants with their decoded double values, register/stack memory operands, and
the printable `.rdata` string pushed before each `setState` call. The generated
`docs/enemy_edge_facts.txt` / `EnemyEdgeFacts.h` preserve VAs and operands
verbatim; `enemyfacts [summary|constants <class>|call <va>|<class>]` queries
them without assigning triggers, timing, field names, or gameplay meanings.
`tools/analyze_flow_edge_facts.py` applies the same collector to the twenty
recovered menu classes in `docs/state_graph.txt`, producing 181 setter/cue
facts with 1,390 comparison/test operands and 144 qword constants in
`docs/flow_edge_facts.txt` / `FlowEdgeFacts.h`. For example, BusterMenu slot 2
cues `0x618e28` after `fcomp` against `-1.0` and `0.0`, and MenuStageSelect slot
2 sets `SelectLevel` at `0x5000f4` after a `0.0` compare with constants `0.0`
and `400.0`. `flowfacts [summary|constants <class>|call <va>|<class>]` queries
the raw table; it does not turn those branches into runtime transitions.
`tools/analyze_pause_weapon_resources.py` reads the inline `PauseMenuStatus`
constructor at `0x4f9160` and records the exact order of 44 pushed
`..\\Menus\\Pause\\*.xspr` paths (31 `Icon`, 13 `Colors`) up to the enclosing
`ret` at `0x4faf96`. `docs/pause_weapon_resources.txt` and
`PauseWeaponResources.h` keep each push VA, string VA, path, and non-reflected
CRC-32 key verbatim, including duplicate keys such as `0x7c728d9f` at sequences
35 and 36. `pauseres [summary|list|key <hex>|path <text>|kind <Icon|Colors>]`
queries that order; it does not assign weapon meanings, slots, or gameplay
effects to any path.
`tools/analyze_inline_menu_resources.py` generalizes the raw push collector to
all 20 `docs/state_graph.txt` classes whose registered name contains `Menu`.
Using int3-delimited constructor regions, it records 231 exact `.xspr` push
sites (153 unique paths) in `docs/inline_menu_resources.txt` /
`InlineMenuResources.h`, including constructor/VAs, string VAs, and keys.
`menures [summary|list|class <name>|key <hex>|path <text>]` queries the table;
no path is promoted to a row, sprite role, or runtime action.
`tools/analyze_state_slot_blocks.py` generalises the enemy thunk-block reader to
all 238 constructor regions across 237 classes: 671 state rows, all with three
recovered `jmp dword ptr [eax + NN]` thunks. The reader accepts both `0xNN` and short-form
`NN` displacement operands (e.g. `[eax + 8]`).
`docs/state_slot_blocks.txt` / `StateSlotBlocks.h` keep the registration VA/name,
graph state name, thunk VAs and slot numbers verbatim, and
`stateslots [summary|class <name>|state <name>|incomplete]` queries them without
turning a slot into a transition.

`tools/analyze_stage_select_grid.py` recovers the 3×3 stage-select grid from
`FUN_004ffc80`, the raw `this+0xba8` toggle disassembly at `FUN_004ff460`, and
the raw cursor-key mapping at `FUN_004ff070` into `docs/stage_select_grid.txt`
and `StageSelectGrid.h`. The eight outer cells map `(x,y)` to the raw
`this+0xbb0` stage numbers `8,1,3 / 7,?,6 / 4,2,5`. The center cell is resolved,
not guessed: `this+0xba8 != 0` gives stage `9`, and `this+0xba8 == 0` gives
`9 + GetSubItemsCount(DAT_006c55a4)` for counts `1..5` (stages `10..14`); other
counts assign no stage. The cursor keys are `0x1c/0x1b` for `this+0xba4` `+1/-1`
and `0x19/0x1a` for `this+0xba0` `+1/-1`, clamped to `0..2`. Both raw inputs are
supplied through `FlowMenuFlags` (`flowrun flags ba8=0|1 sub=N`), and the
stage-select overlay shows the exact observed value without a gameplay
interpretation of either field. `flowgrid [summary|cells|keys|center <0|1> <count>|stage <x> <y> <0|1> <count>]`
prints the raw cells, center rule and cursor-key rows directly.

`tools/analyze_menu_items.py` and `tools/analyze_menu_actions.py` recovered 254
static menu items across 26 suppliers and 32 action codes from `MMU.exe`, writing
`docs/menu_items.txt`, `docs/menu_actions.txt`,
`core/include/mmu/core/MenuItemTables.h` and `MenuActionTable.h`. Every recovered
menu class now selects a default supplier from its constructor: those fixed
arrays drive `flowrun` selection, blank rows are skipped, and confirm emits a
`MenuAction` event with the exact label, action code, source item table and
recovered screen destination. The manager bytes read by `0x4032e0` (`+0x19`),
`0x403300` (`+0x1a`) and `0x403cc0` (`+0x59`) are modeled as raw flags;
`flowrun flags 19=1 1a=0 59=0` selects the matching constructor variants;
`flowrun flags sub=1 boss=4 four=0 eight=0 prev9=0` exercises action `0x06`'s
cutscene gate, and `flowrun complete <loader id>` emits the recovered
`StageCompleted` routing. `flowrun supplier <0xVA>` can force a specific table.
`flowrun boot` emits the constructor's mask/path/write evidence, and
`flowrun complete <loader id>` drains the completion prelude.
`flowcomplete [summary|rows|flag <id>|state <hex> [0|1]|pending]` lists the 12
raw completion-cutscene branches, their string VAs, selector bytes and literal
manager writes directly from `CompletionCutsceneTable.h`.
Action `0x47` follows the recovered dispatcher condition: with neither `+0x19`
nor `+0x1a` set it
constructs `NewGameMenu`, otherwise `CharacterSelectMenu`. Query both tables with
`flowmenu [items [class]|actions [code|kind]]`. The second 18-item vector in
`MainMenu` (`+0xaa0`) is constructed and destroyed but has no observed reader in
the recovered callbacks, so it is not interpreted.

The recovered action cases in `FUN_0050f7d0` and the concrete menu slots now emit
their literal side effects. Action `0x06` (`STAGE SELECT`) only plays
`CSN02FourBosses`/`CSN03EightBosses` when the sub-item count is below 2,
manager `+0x3c` is not 9 and `FUN_00403d40()` returns zero (the byte at
manager `+0x59`); those branches set phase 8 and return without building
`MenuStageSelect`. Otherwise the fallback clears `+0x3c` and constructs
`MenuStageSelect`. Action `0x10` writes manager `+0x5d` (quit), `0x41` forwards
the opaque manager `+0x34` except on `CharacterSelectMenu`, where slot 2 uses
literal phase 2, and `0x48`/`0x49` select MEGA MAN/Z-PROTOTYPE and write phase
5 via `0x61dc80`/`0x402790`. `completeStage(loaderId)` mirrors the literal
loader-id routing at the end of `FUN_00511350`; it is dispatch evidence, not a
gameplay completion flag. The `0x100` branch now also carries its
`FUN_004308c0` pending-string write (global string into manager `+0x40`, byte
`+0x60 = 1`) and the `FUN_004308a0` clear writes on the object at global
`0x6c55f8` (`+0x178:1=0`, `+0x17c:4=0`); these remain raw helper-call evidence.

`tools/analyze_save_flags.py` recovered 138 `FUN_004412d0` writer records, 33
`FUN_00441d90` mask-8 keys, 9 `FUN_00441db0` mask-4 keys and 3 direct
`FUN_00441c80` call sites (keys `1`/`30` with mask `8`, key `30` with mask `2`).
Every reader key has a writer record; no semantic name is assigned. The raw
condition id is a keyboard index below `0x143` or manager byte `id-0xf9`, and
`KeyedFlagRuntime` models the four-bit state machine at record `+0x24`: bit 2
is current true, bit 3 a one-frame rising edge, and bit 1 a falling edge.
`FUN_00511350`'s mask-8 completion checks now consume rising edges from that
runtime, while `flowrun flags cut=<mask>` remains a deterministic override.

`FUN_00507e90` decompiles to `manager[1] = manager[0]; manager[0] = mask`, so
`KeyedFlagRuntime` also tracks the previous mask (`+4`); boot only writes `+0`.
`tools/analyze_activity_masks.py` found 24 direct `FUN_00507e90(mask)` call sites
with six literal values (`0x1`, `0x2`, `0x10`, `0x40`, `0x100`, `0x200`). The raw
site, owning vtable slot and nearby state/cue evidence are written to
`docs/activity_masks.txt` and `ActivityMaskTable.h`. The runtime can apply a
site with `GameFlowRuntime::applyActivityMaskCall()` or `flowrun mask call <va>`;
`flowrun masks` lists the literal values, `flowrun masks sites` enumerates all 24
raw call sites, and `flowrun mask <hex>` sets one directly. `flowrun boot` applies
the constructor's `0x80` mask. These calls gate
the keyed-flag records above, but no gameplay state or category meaning is
inferred from a mask value.

`tools/analyze_activity_mask_readers.py` found 36 direct `FUN_00507ec0()` calls:
26 literal equality tests, 3 literal bit tests and 7 intersections with a
caller-held mask. The raw test form, containing function, vtable owner and
state/cue strings are written to `docs/activity_mask_readers.txt` and
`ActivityMaskReaderTable.h`. `flowrun maskreads` summarizes them and
`flowrun maskreads all` enumerates all 36 raw reader sites;
`flowrun maskread <site va> [dynamic mask]` evaluates one test against the
current mask, with the second operand supplied explicitly for intersections.
Their gameplay purpose remains unresolved.

`tools/analyze_continue_stages.py` decompiled `FUN_0050e610` to recover 38 saved
stage ids (all ids 1-39 except 15) and the exact `Level...` constructor each one
uses. Action `0x0a` now tags its `MenuAction` event with the raw manager stage
id plus the recovered class name and constructor VA; `flowrun continue 13`
models `LevelWilyFour`. The stage id itself is still raw save data, so no save
file layout is inferred.

`tools/analyze_level_setup.py` mines those same 38 constructors for their shared
literal setup: two cue arguments to `FUN_00539770` (including two-way alternates
for Comet Woman, Rainbow Man and Wily Stage One), the stage id written at object
`+0x08`, an `Environment` object from `FUN_00443e30` stored at `+0x14`, the exact
registrar/environment call sites, every literal zero write, and the copy of
`0x69899c` or `0x6989b4` into global `0x6c5598` (36 and 2 constructors). The 33
unique cue strings and all 38 rows are written to `docs/level_setup.txt` and
`core/include/mmu/core/LevelSetupTable.h`. `GameFlowRuntime::applyLevelSetup()`
and `flowlevel [list|cues|stage|ctor|loop|intro|apply]` expose the row as a
`LevelSetupApplied` event only; no cue is played, no transition is selected and
no field or global is given a gameplay name.

`tools/analyze_manager_fields.py` builds a raw offset map for the global at
`0x6c55a0` and the completion object at `0x6c55f8` from their constructor and
decompiled accessors. It records 29 literal writes, 11 accessor rows (including
the `0x4033e0`/`0x403400`/`0x403d40` forwarders and the `0x507e90`/`0x507ec0`
activity-mask helpers), and 44 observed field accesses in `FUN_0050f7d0`,
`FUN_00511350`, `FUN_00507fd0`, `FUN_004308a0` and `FUN_004308c0`. Exact byte
offset, access size, read/write direction, call counts and source lines are
preserved in `docs/manager_fields.txt` and
`core/include/mmu/core/ManagerFieldTable.h`. `flowmgr` queries the map by
summary, offset, accessor VA or consumer function. No offset is renamed or given
gameplay semantics; the existing raw flags at `+0x18`, `+0x19`, `+0x1a` and
`+0x59` stay raw.

`tools/analyze_outro_handoff.py` verifies 19 direct call sites in `MMU.exe`:
cue/set-state/state-name calls, the Wily 5 outro cutscene loader, and the
`FUN_005a8870` handoff calls at `0x5b0a57`, `0x5b0b69` and `0x5a73ea`. Each row
keeps the raw function VA, call VA, target VA, pushed string VA, recomputed
hash, target kind and owner label; class owners are checked against RTTI
vftables and the other owners stay raw `FUN_...` names. The 16 recovered string
VAs and hashes are written to `docs/outro_handoff.txt` and
`core/include/mmu/core/OutroHandoffTable.h`. A second section keeps 43 verbatim
decompiled expressions with their source lines, raw object offsets (`+0x150`,
`+0x17c`, `+0x18c`, `+0x190`, `+0x194`, `+0x3dc`, `+0x140`, `+0x148`, `+0x144`,
`+0x14c` and others), global operands and literal constants uninterpreted. This
is call-site evidence only: no gameplay timing, transition or destination is
inferred. `outro [summary|calls|call <va>|owner <name>|strings|facts|func <va>|offset <hex>]`
lists those raw rows.

`tools/analyze_menu_actions.py` also extracts the literal `.xcut` paths that
`FUN_0050f7d0` builds for actions `0x1e`/`0x1f` (the SpecialMenu credits rows).
Those paths are attached to the emitted `MenuAction` events so a front end can
start the matching cutscene without guessing from the row label.

`tools/analyze_cutscene_keys.py` matched all 80 `cutscenedefs.mmu` database keys
to their original `.xcut` paths using the recovered non-reflected CRC-32, writing
`docs/cutscene_keys.txt` and `core/include/mmu/core/RegisteredCutscenes.h`.
MMUPAK v5 now bakes the decoded 3,379 timestamps and 7,513 commands, so
`core/include/mmu/core/CutsceneRuntime.h` can replay command order, elements,
movement, text and `EndScene` without the original files. There is no numeric
opcode table: both original dispatchers switch on `Command.name`. Use
`cutscene [list|start <key|#index|suffix>|status|step <sec>|events|stop]` or the
**View → Cutscene Runtime** debug panel. `PlaySound`, `PlayMusicLoop`,
`PlayMusicOnce` and `StopSound` events are routed to baked SFX/music by resource
base name. MMUPAK v5 now also bakes the `guns.mmu` object/animation table: a
`DisplayObject` resource is rebuilt as `..\Cutscenes\<scene>\<resource>`, hashed
with the same CRC-32, and its `PlayAnim` name is matched against the object's
parallel `spriteKeys`/`sprites` arrays to reach the packed frame image.
`GetInGameObject` ids match the editor-side first-sprite path baked for each
object. The **View → Cutscene Runtime** panel previews the resolved image and
shows the object/sprite/image keys.

The general 32-bit asset key hash is no longer open: `tools/crack_hash.py`
resolves 137 of the 490 `guns.mmu` keys with the non-reflected CRC-32 at
`0x43c010` and writes `docs/key_names.txt`.
`tools/generate_verified_key_names.py` copies only those exact matches into
`core/include/mmu/core/VerifiedKeyNames.h`; `keynames [summary|list|key <hex>|name <text>]`
queries them without touching candidate lists. The unrestricted sweep in
`docs/key_names_discovered.txt` produced 84651 candidates and `new_matches=0`,
while `docs/key_names_mitm.txt` has 5 exact-hash candidates still awaiting
verification. `keycandidates [summary|list|key <hex>]` prints those five from a
separate header with an explicit `UNVERIFIED` label; they are never returned by
`keynames` or merged into `KNOWN_KEY_NAMES`. `generate_key_sweep_summary.py`
copies only the four sweep counters from the 84,651-candidate report into
`KeySweepSummary.h`; `keysweep [summary]` prints those counters, and candidate
strings are never imported. An unrestricted binary-string dictionary produces
CRC collisions rather than evidence.
The `GotoAttack1/3/4` selection rule is now raw-evidence backed (phase jump
table, counter clears and caller); next are the remaining enemy archetypes
beyond those already recovered and the still-unresolved key names with
domain-specific dictionary sweeps. The debug harness must not be mistaken for
the reconstructed game.
