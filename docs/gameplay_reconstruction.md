# Gameplay reconstruction notes

All addresses below are VAs in the original 32-bit `MMU.exe` (image base
`0x400000`). They were recovered with two independent tools:

- `tools/analyze_rtti.py` — parses MSVC RTTI directly from the PE:
  `TypeDescriptor` (inline `.?AV…@@` name) → `CompleteObjectLocator`
  (signature `0`, TypeDescriptor pointer at `+0xc`) → `vftable[-4]` →
  virtual function pointers. Result: **348 classes, 347 vtables**, dumped to
  `docs/rtti_classes.txt` and `docs/vtables.txt` with `--all` (the default interactive report is capped at
  60 classes). Up to 64 slots are read per table so enemy state triples past
  slot 24 are visible. `tools/generate_rtti_vtable_header.py` copies those 347
  tables and 9,127 function pointers into `core/include/mmu/core/RttiVtables.h`;
  `rtti [summary|classes [filter]|class <name>|vftable <hex>|slot <hex>]`
  lists them without interpreting a slot as a named virtual method.
- `tools/DecompileTargets.java` — a Ghidra headless post-script that decompiles
  selected functions into `docs/decompiled/`. Re-run with:

```sh
export JAVA_HOME=/opt/homebrew/opt/openjdk@25/libexec/openjdk.jdk/Contents/Home
export PATH="$JAVA_HOME/bin:$PATH"
analyzeHeadless ghidra_project MMU -process -noanalysis \
  -scriptPath tools -postScript DecompileTargets.java
```

`tools/generate_decomp_index.py` copies the 404 VA-named function filenames,
12 class-specific string xrefs and the 293/100/19 target lists (384 unique VAs)
into `core/include/mmu/core/DecompIndex.h`.
`decomp [summary|list [filter]|func <va>|targets [kind]|xrefs|xref <class>]`
queries those
identifiers; no function is renamed or classified. The six recovered proto2
schemas in `proto/` and `docs/protos/` are copied verbatim by
`tools/generate_proto_index.py` into `core/include/mmu/core/ProtoIndex.h`;
`protos [summary|list|schema <name>|messages <name>|message <schema> <name>|field <name>]`
prints exact source lines, message names and field declarations without gameplay
labels.

## Top-level flow (level/menu evidence)

`tools/analyze_game_flow.py` scans the RTTI vtables of every `Level*` and
`Menu*` class, reads printable `.rdata` strings from the first part of each
callback, and records calls to the real state setter `0x4405c0` and the global
cue setter `0x4026b0`. The generated report is `docs/game_flow.txt`:

```sh
python3 tools/analyze_game_flow.py /Users/mysterypaint/Downloads/MegaManUnlimited/MMU.exe
```

The distinction between the two setters matters. Menu classes call
`setState("<Name>")`, so those strings are genuine menu states. `BaseLevel` and
its concrete subclasses pass `"Victory"`, `"VictoryZ"`, `"Death"`, etc. to the
global cue setter instead; they are cues, not proof of a `BaseLevel` state name.

Recovered anchors:

- `BaseLevel` vftable RVA `0x296614` (VA `0x696614`): 10 slots. Slot 0 is the destructor, slot 1
  (`0x53b2f0`) is the per-frame driver, slot 2 (`0x539c70`) handles the death
  path, slot 3 (`0x539010`) and slot 4 (`0x53aed0`) are additional update/
  environment hooks; slots 5-9 default to `0x4d73fa`. The driver checks a real
  integer phase at `+0xb0`: `0` is active gameplay, `2` is the victory sequence
  (`0x53b49f`), `3` is the death/fail sequence (`0x53b53f`/`0x53b58e`, cue
  `"Death"` at `0x539cc1`), and `5` is exiting/teleport-out (`0x53b2c5`, early-out
  at `0x53b308`). Values `1` and `4` have not been evidenced yet. These are
  phase values, not `setState` names.
- Concrete levels exist for all eight Robot Masters, their `Super` and `Perfect`
  variants, Wily 1-5, Endless, EndlessBoss, and Gauntlet. They share several
  `BaseLevel` slots and usually override only slot 0 plus one or two hooks.
- Menus: `TitleMenu` (`IntroFadeIn`, `Wait`, `Flash`, `IntroFadeOut`; cues
  `TitleScreenA/B`), `MainMenu` (`IntroFadeIn`, `Select`, `Confirm`, `Flash`,
  `IntroFadeOut`; cues `GameOverLoop`, `Select`, `Confirm`), `NewGameMenu`
  (`Select`, `SelectDone`; teleport-out cues), `MenuStageSelect`
  (`IntroSlideIn`, `SelectLevel`, `Flash`, `IntroSlideOut`; cues `StageSelect`,
  `StageSelectBIntro`, `StageSelectBLoop`, `MenuThemeB`), `OptionsMenu`
  (`IntroFadeIn`, `Select`, `Flash`, `IntroFadeOut`, `MapKeyboardInputs`,
  `MapJoystickInputs`, `SelectDone`), `PauseMenuStatus` (`SlideIn`,
  `SelectItem`, `SlideOut`, `EnergyFill`, `WeaponEnergyFill`; cues `PauseMenu`,
  `Wrong`, `EnergyMeter3s`), `ShopMenu` (`SlideIn`, `SelectItem`, `Confirm`,
  `SlideOut`, `NotEnoughBolts`, `TooManyItems`; cues `Confirm`, `Purchase`,
  `Wrong`, `MenuThemeA/B`), plus `BusterMenu`, `CharacterSelectMenu`, and
  `MenuInGame`. The leading intro states come from constructor graphs (below),
  not from setter call sites.

This is evidence for the **real** MMU flow. The SDL harness's title card and
pause behaviour are not part of it and must be replaced by these classes once
their state machines are reconstructed in `core/`.

`core/include/mmu/core/GameFlow.h` remains the data-only catalogue: 20 top-level
menu classes, 39 level classes, six level cues, and the four evidenced
`BaseLevel` phase values. The console command `flow [screens|levels|name|#index]`
prints it, and `--selftest` verifies its counts and key lookups.

The first executable reconstruction layer is
`core/include/mmu/core/GameFlowRuntime.h`. It models the menu states and cues
that are directly observable in the decompiled callbacks (TitleMenu
`IntroFadeIn` → `Wait` → `Flash`/`IntroFadeOut`, MainMenu selection wrap and
confirm cue, stage-select `IntroSlideIn` → `SelectLevel`/`Flash`/`IntroSlideOut`,
pause energy fills, shop failures, etc.). Initial states come from the
constructor graphs below. Timers are explicitly reconstruction knobs, not
frame-accurate constants.
Terminal states emit events instead of guessing a destination screen or level.
The console command `flowrun` exposes its state, selection, cues, level phase,
and pending events; `tools/gameflow_runtime_selftest.cpp` verifies it
independently of assets.

The original dispatcher itself is a plain named-state machine at global
`0x6c55cc`, not a polymorphic screen manager. `0x4405c0` calls `0x4404c0`, which
hashes the state name with `0x4012f0`/`0x43c010`, calls the old state's exit
callback at `+0x40`, writes hashes at `+0x24`/`+0x28` and the transient requested
hash at `+0x2c`, then calls the new state's enter callback at `+0x20`. `0x446ca0` registers states, `0x461380` is a
reentrancy guard, and `0x4464a0` is the per-frame update. The hash is CRC-style
shift-left with polynomial `0x4c11db7` and seed `0`; for short ASCII strings it
degenerates to packed bytes (`hash("Wait") == 0x57616974`).
`core/include/mmu/core/StateDispatcher.h` reproduces those semantics portably,
with string registrations instead of original pointers. The portable `update()`
calls the active state's hook, then runs a portable per-registration timer pass
with deferred callbacks and mask gating. The original `0x4464a0` likewise
iterates every registered state and decrements timers through `0x446760`, firing
a callback when a timer expires. The mask is the first dword of the global manager at `0x6c55a0`:
`0x00507ec0` is a one-line getter for that field, and `0x00507e90(mask)` writes
it (with the old value saved at `+0x4`) from state callbacks. The observed
literal masks are `1`, `2`, `0x10`, `0x40`, `0x100` and `0x200`; the manager
constructor `0x0050bbb0` starts with `0x80`. The portable runtime still ticks
every registered timer with all masks active because the per-state mask rules
are not reconstructed; `tools/analyze_boot_cutscenes.py` records the recovered
initial value and boot writes in `docs/boot_cutscenes.txt`.
`flowboot [summary|writes|play|pending]` prints the same raw manager writes and
both exact `.xcut` paths.

`tools/analyze_state_names.py` scans the 671 calls to the registration helper
`0x401370` and recovers 266 unique state names, including every menu state used
by the runtime (`Wait`, `Flash`, `IntroFadeOut`, `SelectLevel`, `InGame`,
`WeaponEnergyFill`, `TooManyItems`, …). It writes `docs/state_names.txt` and the
generated `core/include/mmu/core/RegisteredStateNames.h`. The name heuristic is
the last printable `.rdata` immediate in the 0x90 bytes before a call, so it is
evidence for identifiers and exact hashes, not for behaviour. `GameFlowRuntime`
registers each `FlowState` name in its own `NamedStateDispatcher` and mirrors
every `setScreen`/`setState` transition through it; the debug panel and
`flowrun status` show the original state name and hash. Terminal destinations
were initially left as events; the menu-item and action tables below now supply
evidence for them, but the runtime still emits them as events rather than
guessing when the front-end must transition.

`tools/analyze_state_transitions.py` goes one step further: it scans every
`Menu*`/`Level*` vtable callback for calls to `setState` (`0x4405c0`) and the
global cue setter (`0x4026b0`) and records the class, slot, callback address,
call address, target string and hash. Its 222 rows (67 `setState`, 155 cues) are
written to `docs/state_transitions.txt` and
`core/include/mmu/core/RegisteredStateTransitions.h`. Every observed `setState`
target is checked against the 266-name registry. `core/include/mmu/core/FlowEvidence.h`
provides query helpers, and the console command `flowtrans [all|to <name>|class <name>]`
prints the same rows with class, slot, callback address, call address, and
setter kind; the ImGui **Flow Evidence** panel exposes the same table with a
text filter. This is the evidence base for the ordered state graph below. It
still does not prove timing, item labels, or which screen a terminal exit
selects.

## Ordered state graphs

`tools/analyze_state_graph.py` combines three observations to reconstruct each
class's state list in the order the game itself registered it:

```sh
python3 tools/analyze_state_graph.py /Users/mysterypaint/Downloads/MegaManUnlimited/MMU.exe
```

1. The registrar `0x401370` is called once per state inside a constructor. The
   last printable `.rdata` immediate before each call gives the state name.
2. A constructor is identified by writing a recovered vftable pointer and
   containing at least one of those registration calls in the same `int3`-
   delimited function region. Grouping registrations by that region preserves
   the binary's own order. A single registration is kept: the name, the
   constructor's initial `setState` and any vtable edges remain direct evidence
   (previously the two-call minimum discarded classes such as `EZBlock`,
   `SawPole`, `CopterCannon` and the one-state projectile classes).
3. The constructor's final `setState` (`0x4405c0`) call selects the initial
   state. The class's virtual callbacks are then scanned for setter and cue
   targets using the same logic as `analyze_state_transitions.py`.

The generated `docs/state_graph.txt` contains **238 constructor regions across
237 classes, 671 registered states, and 1,446 observed setter calls**. `core/include/mmu/core/RegisteredStateGraph.h`
is the generated C++ table, with `stateGraphClassCount()`, `graphStateCount()`,
`graphEdgeCount()`, `graphOrdersState()`, and `graphInitialStateForClass()`
helpers exposed through `FlowEvidence.h`. The console command
`flowgraph [initial|to <name>|<class>]` prints summaries, initial states, target
xrefs, and a class's full ordered graph.

Key recovered initial states include `TitleMenu` → `IntroFadeIn`, `MainMenu` →
`IntroFadeIn`, `MenuStageSelect` → `IntroSlideIn`, `ShopMenu` → `SlideIn`,
`MegaMan` → `Wait`, `Telly` → `Idle`, and `SuperCannon` → `Stand`. The menu
values now drive `flowInitialState()` and the runtime's intro phases; the older
setter-only guesses remain as a fallback for classes whose constructor was not
recovered.

This closes the ordered-state-graph milestone, but the table stays deliberately
conservative: an edge records only a target observed in a specific virtual
callback. It does not claim which current state enabled that callback, frame
timing, animation lengths, menu item labels, or terminal destinations.

## Menu item and action tables

`tools/analyze_menu_items.py` reads the static string objects built by
`0x402b30`/`0x402850` and recovers **254 items across 26 suppliers and 15
classes** into `docs/menu_items.txt`. Each item records its exact label, index,
action code, supplier VA and menu behavior (`spacer`, `confirm` or `flash`) from
the branch at `0x503550`. `tools/analyze_menu_actions.py` reads the action switch
in `0x50f7d0` and recovers **32 action codes**, of which 23 name a concrete
screen via an RTTI constructor; the raw report is `docs/menu_actions.txt`.
Labels are not used to infer destinations.

The generated `MenuItemTables.h` and `MenuActionTable.h` expose these tables to
the portable runtime. The initial `MainMenu` supplier is `0x508EA0`: slot 0 is
`NEW GAME` with action `0x47`, slots 3, 5, 6 and 9 are blanks, slot 4 is `SPECIAL`
(`0x1b`), slot 7 is `TITLE SCREEN` (`0x01`) and slot 8 is `EXIT GAME` (`0x10`).
`GameFlowRuntime` skips blank slots exactly as the `empty()` loop around
`0x5035a2`/`0x503622` does; confirm then emits a `MenuAction` event with the
action code, label, source item table and destination. The same fixed-array
selection now applies to every recovered menu class, with each default supplier
chosen from its constructor. Manager bytes `+0x19`, `+0x1a` and `+0x59` are
modeled as raw `FlowMenuFlags` because `FUN_004032e0`, `FUN_00403300` and
`FUN_00403cc0` only establish the byte offsets. Action `0x47` follows
`FUN_0050f7d0`: when `FUN_004033e0`/`FUN_00403400` both return false it
constructs `NewGameMenu` via `0x4feb80`; otherwise it constructs
`CharacterSelectMenu` via `0x6182b0`. The table records both destinations and
the runtime selects between them with `+0x19`/`+0x1a`.

The dispatcher also associates actions `0x02`/`0x03`/`0x04`/`0x09` with seven
`MainMenu` supplier variants: `0x508ea0` (`GAME MENU`), `0x508d00`/`0x508820`
(`MAIN MENU`), and `0x508b60`/`0x508680`/`0x5089c0`/`0x5084e0` (`GAME OVER`).
The conditional pairs are selected by manager byte `+0x59`, read by
`FUN_00403d40` → `FUN_00403cc0`. Constructor variant pairs for `NewGameMenu`,
`EndlessAttackMenu`, `EndlessBossAttackMenu` and `GauntletMenu` are selected from
raw flags `+0x59`, `+0x19` and `+0x1a`; `setMenuFlags`/`flowrun flags` models
those reads. `setMenuSupplier`/`flowrun supplier` can also force a specific
recovered table. Action `0x0d` is now labelled save/data mode: `0x504ce0` is
called with `param3=1`, which selects `SaveMenu-BG.xspr` in the `LoadMenu`
constructor. `flowmenu items [class]` and `flowmenu actions [code|kind]`
query the evidence.

`tools/analyze_flow_edge_facts.py` extends the verbatim branch collector to the
twenty menu classes in `docs/state_graph.txt`. The generated
`docs/flow_edge_facts.txt` / `FlowEdgeFacts.h` records 181 setter/cue facts with
1,390 comparison/test operands, register/stack memory operands, 144 qword
constant addresses with decoded double values, and printable `.rdata` keys.
These are raw callback operands only. For example, `BusterMenu` slot 2 cues
`0x618e28` with key `0x68da0c` (`"Select"`) after `fcomp qword ptr [0x6987b8]`
(`-1.0`) and `fcomp qword ptr [0x697ce0]` (`0.0`), and `MenuStageSelect` slot 2
calls `setState` at `0x5000f4` with key `0x65f984` (`"SelectLevel"`) after
`fcomp qword ptr [0x697ce0]` with constants `0.0` and `400.0` at `0x698aa0`.
`flowfacts [summary|constants <class>|call <va>|<class>]` prints these rows;
no branch is promoted to a runtime transition without direct evidence.

The inline `PauseMenuStatus` constructor (`0x4f9160`) is now recorded separately
by `tools/analyze_pause_weapon_resources.py`. It pushes 44 exact
`..\\Menus\\Pause\\*.xspr` strings in sequence before its enclosing `ret` at
`0x4faf96`: 31 `Icon` paths and 13 `Colors` paths, with each push VA, string VA,
path, and non-reflected CRC-32 key preserved in
`docs/pause_weapon_resources.txt` / `PauseWeaponResources.h`. Examples include
sequence 13 at push `0x4f9d16` / string `0x65e564`, key `0x73a4ca16` for
`..\\Menus\\Pause\\MegaBusterIcon.xspr`, and sequence 37 at push `0x4fa1c3` /
string `0x65e904`, key `0x424b7cbc` for
`..\\Menus\\Pause\\WaterCannonColors.xspr`. Duplicate keys are retained;
`0x7c728d9f` appears as both `RushCoilColors.xspr` pushes at sequences 35 and
36. `pauseres [summary|list|key <hex>|path <text>|kind <Icon|Colors>]` queries
this order without assigning weapon meanings or slot semantics.

`tools/analyze_inline_menu_resources.py` records the wider constructor-level
evidence for the same twenty recovered menu classes. Inside each int3-delimited
constructor region it finds every push of a printable `.rdata` path ending in
`.xspr`, yielding 231 sites and 153 unique paths in
`docs/inline_menu_resources.txt` / `InlineMenuResources.h`. For example,
`PauseMenuStatus` region `0x4f9160..0x4fafa0` has 89 sites beginning at push
`0x4f97d0` / string `0x65e180` for `..\Menus\Pause\PauseMenuBGZERO.xspr`, while
`MenuStageSelect` region `0x5002b0..0x500d60` has 18 sites beginning at push
`0x5004ea` / string `0x65f588`. `menures [summary|list|class <name>|key <hex>|path <text>]`
prints these rows verbatim; all resource roles remain unassigned.

`tools/analyze_state_slot_blocks.py` applies the enemy thunk-block read to every
constructor region in `docs/state_graph.txt`. It emits 671 raw state rows across
238 constructor regions (237 classes) in `docs/state_slot_blocks.txt` /
`StateSlotBlocks.h`: all 671 rows have three `jmp dword ptr [eax + NN]` thunks. The reader accepts both `0xNN` and
short-form `NN` displacement operands (e.g. `[eax + 8]`), which recovers the
previously missed menu thunks `0x431930` and `0x433070`. For example,
`MetTrois.Hidden` registers at `0x58b9d5` with slots `25,24,23` and thunks
`0x4242f0,0x41f710,0x426380`, while `BusterMenu.Select` registers at
`0x6193bb` with slots `3,2,1` and thunks
`0x403290,0x431930,0x433070`. `stateslots [summary|class <name>|state <name>|incomplete]`
prints these blocks; no slot is interpreted as a runtime transition.

`LoadMenu` (`0x504ce0`), `ShopMenu` (`0x4f5d50`), `PauseMenuStatus`
(`0x4f9160`) and `MenuStageSelect` (`0x5002b0`) do not build their rows through
the recovered `FUN_00402b10`/`FUN_00402b30`/`FUN_00402850` supplier pattern.
Their constructors assemble inline sprite paths, icons and numeric descriptors
with inlined helpers that Ghidra mis-decompiles as code addresses (for example
`0x4f64dc` in `ShopMenu`). No static label/action array is present for those
screens, so `analyze_menu_items.py` reports no supplier for them and their
runtime row counts and slot order remain reconstruction knobs.

The harness title and pause overlays now run `GameFlowRuntime` directly. The
title `IntroFadeIn → Wait → Flash → IntroFadeOut` chain uses `TitleMenu`'s
constructor states, and its exit emits `ScreenChangeRequested`. Pause enters
`MenuInGame`; the harness sets `FlowMenuFlags::pauseExitStageVariant` so it uses
the recovered `0x5081A0` supplier, whose four visible rows are `RESUME GAME`
(`0x0e`), `EXIT STAGE` (`0x0f`), `TITLE SCREEN` (`0x01`) and `EXIT GAME` (`0x10`).
`0x508340` is the other recovered table: it omits `EXIT STAGE`, leaving three
visible rows. The branch lives in `0x511350`: for manager stage id `+0x3c` in
`1..9` it evaluates `FUN_0061daf0(DAT_006c20f4[stageId])` (a byte read from a
progress array through `FUN_00435b10`/`FUN_00435c70`); false builds `0x508340`,
true builds `0x5081A0`. `DAT_006c20f4` is the fixed int table
`[2,8,3,7,9,6,4,1,5]` for ids 1-9. The save-data source of the progress byte is
not recovered, so `flowrun flags pause=0|1` supplies the predicate result
directly. `PauseMenuStatus` remains a separate energy-detail screen with no item
supplier. This replaces the previous hardcoded title/pause cards without turning
the harness into the game UI.

The harness now completes the title → stage-select → gameplay path. On entry
to `MainMenu` it selects the recovered unflagged action `0x03` supplier
`0x508820`, whose slot 0 is `STAGE SELECT` (`0x06`). Confirming that row emits
the destination `MenuStageSelect`; confirming a grid cell emits `StageSelected`
with the raw stage id from `FUN_004ffc80`, and the harness converts it to a
`LevelSetupApplied` event with `applyLevelSetup()`. A recovered level class is
then mapped to an exact `PackedScene::name` already present in `assets.pak`
(`WilyOne`…`WilyFive` are normalized to the digit spellings used by the editor
paths), and `GameRuntime::startScene()` loads that scene. The `_Section_1` /
`_Section_01` name preference is by filename; when that scene has no decoded
spawns, the first matching scene with spawns is used so the harness enters a
playable section. This is harness policy over recovered names and decoded
`PackedScene::spawnCount`, not an assigned stage order. `GameFlowRuntime` still
only emits events and never chooses gameplay destinations. `MenuAction 0x0a`
uses the same
handoff when `flowrun continue <id>` supplies a raw saved-stage id.

`MenuStageSelect` no longer uses a linear 12-row placeholder. Callback
`0x4ffc80` maps the 3×3 cursor grid (`this+0xba0` x, `this+0xba4` y) to the
raw `this+0xbb0` stage number `8,1,3 / 7,center,6 / 4,2,5`, and `0x4ff070`
clamps row/column to `0..2` instead of wrapping. Its raw key mapping is
`0x1c/0x1b` → `this+0xba4` `+1/-1` and `0x19/0x1a` → `this+0xba0` `+1/-1`.
`tools/analyze_stage_select_grid.py` recovers the previously unresolved center cell directly into `docs/stage_select_grid.txt`: when the raw byte
`this+0xba8` is nonzero the stage is `9`; when it is zero the stage is
`9 + GetSubItemsCount(DAT_006c55a4)` for counts `1..5`, i.e. `10..14`. No stage
is assigned outside those ranges. Disassembly of `FUN_004ff460` also shows
`this+0xba8` being toggled after the `this+0xbac` timer when the
`DAT_006c55a4` subobject flags test true with raw mask `0xf`
(`FUN_00404b00`), but no gameplay label is assigned to that predicate. The
runtime supplies both raw inputs through `FlowMenuFlags` and emits the exact
`StageSelected` value. `flowgrid [summary|cells|keys|center <0|1> <count>|stage <x> <y> <0|1> <count>]`
prints those raw cells, key mappings, and center-rule results; no field is
renamed or given gameplay semantics.
Actions `0x48`/`0x49` (the `MEGA MAN`/`Z-PROTOTYPE` rows) are handled locally by
`CharacterSelectMenu` slot 2 (`0x617f00`), not by the global action dispatcher:
it reads the row action from `FUN_00402b30` and, for `0x41` or `0x47 < action <
0x4a`, calls `FUN_004405c0("SelectDone", ...)`. The runtime therefore emits a
`MenuAction` event with no screen destination, which is the observed behaviour.
The separate `0x45` (`BUSTER:`) row calls `FUN_0061ef80()` there; the global
`0x46` case builds `BusterMenu` for other menus. The character-select timer slot
`0x617e10` additionally handles `0x48`/`0x49` by calling `0x61f5e0`, writing
character `0`/`1` through `0x61dc80`, and setting phase 5 through `0x402790`;
its local `0x41` writes character `0` and phase 2 instead. The runtime now
mirrors those manager-phase writes and reports the selected character in the
`MenuAction` event.

Action `0x0a` passes the raw manager field `+0x3c` (`param_1[0xf]`) to
`FUN_0050e610`, which switches over 38 saved stage ids and constructs a
`Level...` object. `tools/analyze_continue_stages.py` reads that switch and each
decompiled constructor's `vftable` assignment into
`docs/continue_stages.txt` and `ContinueStageTable.h`.
`flowcontinue [summary|list|stage <id>|class <name>|ctor <va>]` enumerates those
raw rows. The ids cover 1-39 except
15: `1` is `LevelTankMan`, `14` is `LevelWilyFive`, `16` is `LevelEndless`, and
`39` is `LevelPerfectWhirlpoolMan`. `flowrun continue 13` models the saved id,
and the `CONTINUE` `MenuAction` event now carries the id, class name and
constructor VA. The save-data source of the id is still unmodelled.

Actions `0x1e`/`0x1f` (the SpecialMenu `CREDITS (MAIN GAME)` and
`CREDITS (SPECIAL)` rows) build literal `.xcut` paths in `FUN_0050f7d0`
(`CSNMainCredits.xcut` and `CSNSpecialCredits.xcut`). `analyze_menu_actions.py`
extracts those into `MenuActionTable.h`, and the emitted `MenuAction` event
carries the path so the cutscene can be started without reading the label text.

Action `0x06` follows `FUN_0050f7d0:214-262`. After clearing the manager string
`+0x10` and setting byte `+0x18`, it requires `GetSubItemsCount(0x6c55a4) < 2`,
`manager +0x3c != 9`, and `FUN_00403d40() == 0`. That helper is a one-line
wrapper over `FUN_00403cc0`, which returns byte `+0x59` of the manager, so the
runtime predicate is `!menuFlags.flag0x59`. With boss count 4 and
`FUN_004309f0() == 0` it plays `CSN02FourBosses`; with count 8 and
`FUN_00430a30() == 0` it plays `CSN03EightBosses`. Both branches set
`manager +0x08` (stage phase 8) and return before constructing
`MenuStageSelect`, so the emitted event has no screen destination. The fallback
sets `+0x3c = 0` and constructs `MenuStageSelect`. Action `0x10` writes manager
byte `+0x5d` (`0x5100a1`, read as quit at `0x5e0169`), and generic action `0x41`
forwards the opaque `manager +0x34`, whose writer is still unidentified.

`completeStage(loaderId)` mirrors the raw loader-id dispatch at the end of
`FUN_00511350`: `0x10/0x11/0x12/0x13` write actions `0x12/0x15/0x18/0x1b`,
`0x1e-0x24` writes `0x20`, `0x14-0x17` and `0x25-0x27` write `0x21`,
`0x18-0x1d` writes `0x22`, and the catch-all plays `GameOverIntro` with phase
`5` for saved ids `0xa-0xe` or `9` otherwise. This is evidence for that
dispatch only; no gameplay completion condition is inferred from the name.

Before that loader-id routing, `FUN_00511350` checks a chain of raw flag ids
through `FUN_00441d90` (`7`, `8`, `9`, `0xa`, `0xb`, `0xc`, `0xd`, `0xe`). The
first true result calls `FUN_00507fd0(1)` to clear the manager screens and then
starts a hard-coded `.xcut` path with `FUN_0050ba40(path, 1)`, returning before
the loader-id switch. Three manager-state cases are checked after the flags:
manager value `0x100` chooses between the ending and credits paths using
manager byte `+0x59` and writes `+0x08 = 0x100`; `0x200` selects the intro path
and writes `+0x08 = 4`, `+0x30 = 1`; `0x40` calls `FUN_00510c80()` without a
cutscene. The global
`std::string` objects used by those branches (`0x6c6964`, `0x6c6a00`,
`0x6c6a3c`, `0x6c6a98`) are reconstructed from their static initializers in
`.text`, not from filenames. `tools/analyze_completion_cutscenes.py` emits all
12 branches into `docs/completion_cutscenes.txt` and
`core/include/mmu/core/CompletionCutsceneTable.h`; the runtime models the
unresolved save-data source with `FlowMenuFlags.completionFlags` and carries the
literal path plus the raw flag/state in `FlowEvent`. The SDL2 harness starts the
matching decoded cutscene from that path.
`flowcomplete [summary|rows|flag <id>|state <hex> [0|1]|pending]` prints all 12
rows, including the `0x100` pending-string selector rows and the `0x200` literal
writes, without assigning gameplay outcomes to the flag IDs or manager states.

For manager state `0x100`, the same branch also calls `FUN_004308c0` a second
time with the chosen global string. Decompiling that helper shows it copies the
string into manager `+0x40` and writes byte `+0x60 = 1`; this is carried as
`completionPendingPath`, `completionPendingStringVa` and the raw helper offsets
in `FlowEvent`, without assigning a gameplay meaning to those fields. Immediately
before the state checks, `FUN_004308a0` clears byte `+0x178 = 0` and dword
`+0x17c = 0` on the object held at global `0x6c55f8`; the analyzer verifies that
call site and emits the offset/size/value triples into
`core/include/mmu/core/CompletionCutsceneTable.h`. This is raw call evidence; no
semantic name is inferred for the cleared object or the pending slot.

The `FUN_00441d90` predicate is now tied to the recovered keyed flag store.
`tools/analyze_save_flags.py` decodes 138 `FUN_004412d0` registrations
`(category, key, aux, enable, min, max)` and all reader sites: 33 mask-8 keys,
9 mask-4 keys and 3 direct `FUN_00441c80` calls (`key=1/mask=8`,
`key=30/mask=8`, `key=30/mask=2`). Every reader key has a matching writer
record; the report writes that cross-check to `docs/save_flags.txt` and the
raw tables to `core/include/mmu/core/SaveFlagTable.h`.
`saveflags [summary|writers|writer <va>|mask8|mask4|readers|key <n>]` lists
those raw records; no key, category or mask is given a gameplay name.
`FUN_00441820` treats each aux id below `0x143` as a
keyboard index and every higher id as manager byte `id-0xf9`, ORs the active
conditions, and advances a four-bit state at record `+0x24`: bit 0 false,
bit 1 falling edge, bit 2 true, bit 3 rising edge. It also skips a record when
`category & FUN_00507ec0()` is zero, so `KeyedFlagRuntime` exposes the recorded
category and global activity mask; the harness defaults that mask to all
categories because the per-state mask rules remain unresolved. New records
start at bits `0`, while `FUN_00441b70(1)` seeds them with bit 0.
`GameFlowRuntime::completeStage()` now tests `FUN_00441d90` keys 7-0xe as
rising edges from this runtime, with `FlowMenuFlags.completionFlags` retained
only as a deterministic harness override.

`tools/analyze_activity_masks.py` recovered 24 direct calls to
`FUN_00507e90(mask)`. Decompiling the helper confirms the exact operation
`manager[1] = manager[0]; manager[0] = mask`: the previous activity mask is
copied to manager `+4` before the new mask is written to `+0`, and the helper
returns `1`. `KeyedFlagRuntime::setActivityMask()` mirrors both writes and
`ActivityMaskChanged` events carry the previous value; the boot path writes only
`+0` and leaves `+4` untouched. The literal masks are
`0x1`, `0x2`, `0x10`, `0x40`, `0x100` and `0x200`; `docs/activity_masks.txt`
records each call site, containing function, vtable owner, and state/cue strings
from the existing state graph and decompilation. The generated
`core/include/mmu/core/ActivityMaskTable.h` lets the harness select a raw site
through `GameFlowRuntime::applyActivityMaskCall()` (`flowrun mask call <va>`) or
set a literal value directly (`flowrun mask <hex>`); `flowrun masks sites`
enumerates every raw site. Boot uses the constructor's `0x80` mask. These values
are still treated as raw bitmasks: their gameplay
states and the reason each call chooses its mask remain unresolved.

`tools/analyze_activity_mask_readers.py` recovered the other half of the mask
interface: 36 direct `FUN_00507ec0()` calls. The first test after each call is
recorded as a literal equality (26 sites: `eax == 1/2/4`), a literal bit test
(3 sites: `eax & 0x80`), or an intersection with a caller-held value (7 sites
such as `test dword ptr [esi + 0x9c],eax`). The report
`docs/activity_mask_readers.txt` and generated
`core/include/mmu/core/ActivityMaskReaderTable.h` keep the raw operand text,
containing function, vtable owner and state/cue evidence.
`GameFlowRuntime::evaluateActivityMaskRead()` mirrors only that test form;
callers must supply the dynamic operand for intersections. The console exposes
the same table through `flowrun maskreads`, `flowrun maskreads all`, and
`flowrun maskread <site va> [dynamic mask]`. No state name or gameplay rule is
assigned to a test or its mask.

`tools/analyze_level_setup.py` extends the 38-level cross-check into the
constructor prefix itself. Every recovered `Level...` constructor passes two
string arguments to the cue registrar `FUN_00539770` (38 exact call sites at
`0x539770`), writes the saved stage id at object `+0x08`, stores the result of
`FUN_00443e30("Environment")` at `+0x14`, and copies `0x69899c` (36 sites) or
`0x6989b4` (2 sites) into global `0x6c5598`. The first two cue arguments are
recorded with their branch alternatives: Comet Woman and Wily Stage One select
between two loop and two intro strings, Rainbow Man has two loop strings and an
empty intro, and many levels use an empty intro literal. In total there are 33
unique nonempty cue strings. Exact registrar and environment call sites and all
literal zero writes to the level object are preserved in
`docs/level_setup.txt` and `core/include/mmu/core/LevelSetupTable.h`. The
runtime's `applyLevelSetup()` emits a `LevelSetupApplied` event with a pointer to
the static row; `flowlevel [list|cues|stage|ctor|loop|intro|apply]` exposes the
same data without starting a level, playing a cue or assigning semantics to the
global. `FUN_0043c840` reads `0x6c5598` through `FUN_0043c890`, and `FUN_005b15d0`
negates the global and emits the literal cue `"GravityChange"`; neither the
global's name nor the purpose of `+0xf4`/`+0xf8` is inferred here.

`tools/analyze_manager_fields.py` turns the same constructor and its known
consumers into a raw field map. `FUN_0050bbb0` zeroes 25 fields and then writes
`+0x00 = 0x80`, `+0x08 = 0x80`, `+0x0c = 4` and `+0x30 = 1`. The unrelated
`FUN_0050ba40` helper also writes `0x80` at `+0x00`, but its callers pass local
objects rather than the global manager, so it is deliberately excluded. The
analyzer records those 29 manager literal writes, plus 11 accessor rows and 44
consumer accesses with exact byte offset,
size, direction, direct call count and source line. The tiny helpers expose
`+0x18` as a one-byte write (`FUN_00403220`, 157 sites), `+0x19`/`+0x1a` and
`+0x59` as one-byte reads (`FUN_004032e0`, `FUN_00403300`, `FUN_00403cc0` with
forwarders `FUN_004033e0`, `FUN_00403400`, `FUN_00403d40`), and the
activity-mask helpers as `+0x00`/`+0x04` writes and `+0x00` reads
(`FUN_00507e90`, `FUN_00507ec0`). Consumer evidence includes the repeated
four-byte pointer writes at `+0x18`/`+0x1c` in `FUN_0050f7d0` and
`FUN_00507fd0`, the saved-stage reads/writes at `+0x3c`, and the completion
object's `+0x60`, `+0x178` and `+0x17c` writes in `FUN_004308c0`/
`FUN_004308a0`. All of this is emitted to `docs/manager_fields.txt` and
`core/include/mmu/core/ManagerFieldTable.h`, and `flowmgr` reads those static
rows. None of the offsets are renamed, no object lifetime is invented, and the
previously recovered flags remain raw manager bytes.

The constructor also creates a second 18-item vector at `MainMenu +0xaa0`
(`param_1 + 0x2a8` in `FUN_00503890`) and the destructor releases it, but no
recovered `MainMenu` callback reads it. It is left as an unexplained allocation
rather than interpreted as another menu page.

## Base virtual layout

The engine `GameObject` vftable (`0x697664`) has 18 slots. Shared across most
subclasses:

| Slot | Function | Meaning |
|------|----------|---------|
| 0 | `0x43c190` (base) | scalar deleting destructor (overridden per class) |
| 1 | `0x401580` | vector deleting destructor |
| 2 | `0x43c910` | lookup in an array at `+0x300`, dispatch a virtual at `+4+12*i` |
| 3 | — | **message handler** (overridden per class) |
| 4 | `0x43d340` | attach object, copy sprite/animation fields |
| 5 | `0x4015b0` | set byte at `+0x68` |
| 6 | `0x4015f0` | calls `0x4015d0` |
| 7 | `0x404f30` | returns 0 |
| 8 | `0x43d8a0` | empty default |
| 9 | `0x431140` | returns `this+0x34` |
| 11 | `0x43cf60` | state/animation update (overridden) |
| 12 | `0x434cf0` | empty default |
| 14 | — | **per-frame logic** (overridden) |
| 16 | `0x425280` | returns 0 |
| 17 | `0x43d900` | default handler |

`Enemy` = `0x68fe5c`, `Boss` = `0x6900cc`, `MegaMan` = `0x68fccc`,
`Camera` = `0x69775c`.

## MegaMan message handler (slot 3, `0x5b02f0`)

The handler switches on a message ID and changes animation state with
`FUN_004405c0("<Name>", this+1)`. String literals at `0x67d544` next to
`"Units\MegaMan\megaman.xgun"` give the canonical state list:

```
Stand Step Run Fall Hurt HurtKnockback HurtSlide Ride Slide Ladder LadderOut
Transition Death Fly Wait Spawn Glued Stun CometDash WeaponAbsorb Carried
Teleport ZeroShoryuken ZeroMessenko ZeroMessenkoDrop ZeroSaberSlash ZeroPunch
```

Message arms identified from the decompilation:

| ID | Effect |
|----|--------|
| `0x00` | direction/dash input, shot deflection ("Deflect") |
| `0x01` | Death |
| `0x05` | Transition (direction = 1..6 picks destination) |
| `0x07` | add a world-space offset to position |
| `0x08` | damage: invincibility flag `+0x1bd`, attacker at `+0x70`, knockback from attacker x, then Hurt/Fall/Stand/Run |
| `0x09` | reset temporary flags, return to Fall unless in a locked state |
| `0x0d` | health += value (`param_1[99]`) |
| `0x0e` | weapon energy += value, weapon selection over 13 weapons |
| `0x0f` | Fly |
| `0x11` | Glued (value stored at `+0x72`) |
| `0x13` | clear Glued |
| `0x14` | Stun (guarded by Death/Fall/ZeroShoryuken/Hurt) |
| `0x1a` | dash/timer scalar |
| `0x21` | per-frame input; Stand/Run/Fall/Ladder, "HurtSlide"/"HurtKnockback" |
| `0x22` | unpause |
| `0x23` | Carried → Stand |
| `0x24` | Teleport |
| `0x26` | GravityChange |

State guards in the update virtuals (`0x5aa470`, `0x5b15d0`) check `Wait`,
`Death`, `Transition`, `Fly`, `Fall`, `ZeroShoryuken` before applying input.

## Outro/handoff raw call-site evidence

`tools/analyze_outro_handoff.py` verifies 19 direct call sites, 16 pushed
string VAs and 43 verbatim decompiled offset/constant facts from `MMU.exe`;
the generated rows are in `docs/outro_handoff.txt` and
`core/include/mmu/core/OutroHandoffTable.h`. The priority sites are
`0x5a8a97` (target `0x420a90`, pushed `0x67d744` = `EnergyMeter3s`, hash
`0xa5964be6`), `0x5b0a57`, `0x5b0b69` and `0x5a73ea` (all target `0x5a8870`),
`0x526410` (target `0x621890`, pushed `0x66f044` =
`..\Cutscenes\CSNZero\csnWily5Outro.xcut`, CRC-32 key `0x60bb8d06`), `0x526632`
(target `0x4026b0`, `BigExplosion`, hash `0xd3f76c3c`), `0x5d3955`, `0x5c8625`
and `0x5c682f` (target `0x4405c0`, `ConcludeBossFight`, hash `0x3bdfa46f`).
Owner labels for virtual functions are checked against RTTI vftables; internal
helpers keep their raw `FUN_...` names. The raw facts preserve `FUN_005a8870`
object offsets `+0x150`, `+0x17c`, `+0x18c` and `+0x194`, its `FUN_00507e90(1)`
and `FUN_00507e90(2)` mask arguments, MegaMan message `0xd`/`0xe` array offsets
`0x18c`, `0x190`, `0x3dc` and `0x194`, and the Wily 5 fields at `+0x4`, `+0x10`,
`+0xa0`, `+0x140`, `+0x144`, `+0x148` and `+0x14c` with their literal constants
and global operands. The table records calls, targets, literals, hashes,
offsets and constants only; it does not assign timing, transitions,
destinations or gameplay outcomes. `outro [summary|calls|call <va>|owner <name>|strings|facts|func <va>|offset <hex>]`
lists the generated rows verbatim.

## Runtime implementation

`core/PlayerState.{h,cpp}` implements `PlayerController` with these exact state
names and message IDs. The transition guards mirror the decompiled switch arms.
Numeric physics constants come from `guns.mmu`: 316 game objects and 490 unique
hashed `GameParam` keys. The hash is **resolved**: `MMU.exe 0x43c010` is
non-reflected CRC-32 (poly `0x04c11db7`, seed `0`, no final XOR), the same
function used for state and cutscene names. Case is significant and no path
normalization is applied. `tools/crack_hash.py` verifies 137 key/name pairs and
writes `docs/key_names.txt`; the other candidate hashes in that tool are kept as
negative controls. The earlier “hash remains open” conclusion came from testing
only reflected zlib CRC-32.

`World::step(dt, PlayerInput)` advances the controller and syncs the resulting
position/facing into the player `WorldObject`. `state`, `states`, `event`,
`hurt`, and `heal` console commands expose it; `Hurt`/`Stand`/`Run` transitions
are covered by `--selftest`.

## Collision

`levelart.mmu` contains 557 full-room collision images. Most are 8-bit indexed
surfaces whose palette encodes tile semantics: white is solid terrain, red is a
solid hazard, black is open space, and the remaining colors mark ladders/water.
Six are 24/32-bpp direct-color images and are decoded through their stored SDL
channel masks/shifts/losses. `core/LevelArt.cpp` resolves them with the same
scene-name heuristics as the background art (`\Levels\YoyoMan_Section_1.xscn` →
`Collisions/YoyoMan_Section1.png`), builds a row-major solid mask, and attaches
it to the controller. The player uses a ~7x12 pixel box with axis-separated
resolution. The exact 791 editor-side `.xscn` literals are copied from
`tools/scene_names.inc` into `core/include/mmu/core/SceneNames.h` by
`tools/generate_scene_names_header.py`;
`scenenames [summary|list [filter]|path <text>|index <n>]` lists them without
mapping them to runtime keys.

Name matching now covers aliases for the Wily 4 abbreviated boss names
(`CWoman` → `CometWoman`, `GMan` → `GlueMan`, ...), NitroMan/TrinitroMan, Gauntlet
rooms that reuse Endless surfaces, the Super Robot Master rooms, and the
`3H3H_001A/B` → `001/002` variants. Coverage is 791/791 scenes, all loadable;
lookup (`findSceneCollisionImage`) is split from bitmap construction so the
coverage query does not decode hundreds of images. `--selftest` reports lookup
and loadable coverage and confirms that the player lands on the packed floor
(`y=170`, `grounded=yes`) instead of falling forever. The `collision` console
command reports the active image index, dimensions, and the player's
solid/grounded state.

## Coverage accounting

`tools/generate_coverage_report.py` derives `docs/reconstruction_coverage.txt`
and `core/include/mmu/core/CoverageSummary.h` from the generated artifacts, and
`coverage [summary|keys|states|reports]` prints the same counts. The verified
key-name count is **137/490 (28.0%)**, leaving 353 unresolved keys; five MITM
candidates stay `UNVERIFIED`, and the 84,651-candidate sweep produced
`new_matches=0`. State slot blocks are 671/671 (100%) with 0 incomplete; the
second `MMWeapon` constructor at `0x549dc0` is retained alongside the primary
one at `0x549a80`.
All 36 raw reports now have a direct portable-header counterpart. All 39
generated headers are reachable from the SDL harness (37 direct; two through
`FlowEvidence.h`), and all 64 registered commands are documented. Raw evidence
queries `flowraw`, `rtticlasses`, `camera`, `playerspawn` and `playerctl` expose
the mapped reports verbatim.

These are bounded artifact ratios, not gameplay completeness. The total number
of functions, rules, or behaviours in `MMU.exe` is unknown, so no global
percentage is claimed. Counts without a denominator are listed separately:
404 decompiled functions, 347 vtables / 9,127 slots, six proto schemas /
24 messages / 166 fields, and 791 scene paths.

## Enemy state machines

`tools/FindStringXrefs.java` locates the functions that reference each
`Units\...\Foo.xgun` path in `MMU.exe`. Following those constructors to their
vftables gives Telly's state callbacks directly:

| State | Callback | Address |
|-------|----------|---------|
| `MoveH` enter | slot `0x5c` | `0x586090` |
| `MoveH` update | slot `0x60` | `0x586310` |
| `MoveV` enter | slot `0x68` | `0x586020` |
| `MoveV` update | slot `0x6c` | `0x5861d0` |
| `Idle` update | slot `0x78` | `0x586100` |

The logic is now unambiguous. `Idle` compares the player position against a
60-pixel radius and picks an axis; the enter callbacks store the direction to
the player in `+0x18c`; each move callback keeps that direction until it has
travelled the radius from its spawn point (`+0x198`/`+0x19c`). At that moment the
update callback re-reads the player and selects `MoveV` when the vertical
distance exceeds the radius, `MoveH` when the horizontal distance does, or
`"Idle"` (`DAT_0067b1bc`/`0x67b1d4`) when neither does — so Telly can hand off
between axes. The three empty callbacks are state exit hooks.

The constructor at `0x586470` also identifies three Telly gun parameters by
matching the hashed keys it loads into `edi` against the decoded `guns.mmu`
objects:

| Key | Name | Value | Meaning |
|-----|------|-------|---------|
| `0x57469074` | `MoveSpeed` | 30 | movement speed |
| `0xda1f1441` | `MoveDistance` | 60 | detection/home radius |
| `0xa21be30b` | `HitPoints` | 20 | object hit points, not a sprite index |
| `0x42714481` / `0x42714480` | `TargetHotSpotX` / `TargetHotSpotY` | 26 / -53 | contact box from spawn handle |

The owning object is #104 (`key=0xaabc064d`). Resolving the key names removes
the last approximation from Telly's two physics parameters and corrects the
earlier interpretation of `0xa21be30b`.

`core/EnemyState.{h,cpp}` implements this as `EnemyController`. Telly uses
`Idle`/`MoveH`/`MoveV`; EZBlock (`0x68dcbc`) toggles `Wait`/`Idle`; MineE
(`0x690d0c`) runs `Scan` → `Explode` → `Death`; SawPole (`0x68ff4c`) uses a
bounded `Patrol`; and SuperCannon (`0x690044`) cycles `Stand` → `Turn` →
`Attack`, counting a projectile volley on `Attack` entry. Flasher (`0x692024`)
cycles `Move` → `Attack` → `Move`, counting an attack on `Attack` entry; the
callback at `0x57c380` plays the `FlasherAttack` cue. Electrode (`0x691554`)
cycles `Init` → `Move` → `Shoot` → `Move`, counting a shot on `Shoot` entry;
slot 29 plays the `Electrode` cue. Their timing and contact boxes are still
approximations. `World::loadScene` attaches controllers to matching spawns,
`World::step(dt, PlayerInput)` drives them, and body contact with the player's
~7x12 box raises the existing `Damage` event. `enemies`,
`enemy <index> [state|hurt N]`, `enemy states` and `projectiles` expose the
machines in the console; `--selftest` now reports 127 PASS/FAIL checks including
those transitions, projectile spawning/homing, contact damage, raw camera,
player-spawn and player-control evidence tables, and cutscene sprite resolution.

Six further archetypes follow constructor vftables recovered from `MMU.exe`:

| Class | vftable | Machine | Evidence |
|-------|---------|---------|----------|
| ExplosivePlatform | `0x69500c` | `Idle` → `Flash` → explosion | `0x54f710` fuse uses `ExplosionDelay` (0.8 s normal / 0.5 s alternate); helper `0x54f8a0` |
| YokuBlock / YokuSpike | `0x6950dc` / `0x69514c` | `Wait` → `Appear` → `Idle` | `0x54e780` handler, `0x54e860` cues `YokuBlock`; `IdleDelay` from the owning object |
| MetStand / MetSwim | `0x68e2bc` / `0x68e35c` | `Stand` → `Pop`/`SwimUp` → `Fire`/`FallDown` → `Hidden` | `0x5e17xx` / `0x5e10xx` callbacks; Met base `0x58da40` toggles hitbox with `Hidden` |
| MetLaser / MetTank | `0x69101c` / `0x6910ac` | `Hidden`/`Stand` → `Pop` → `Fire`/`Exposed` → `Hiding`/`Charge` → `Hidden` | `0x58cxxx` callbacks; MetLaser starts `Hidden` (`0x58cc00`) |
| CrossShooter / SpikePlatform | `0x691e4c` / `0x69522c` | `Wait` → `Shoot` → `Wait` | `0x57eac2` / `0x54d33c` cue shots; `IdleDelay` from the gun object |
| ShieldAttacker | `0x690c84` | `Init` → `Move` ↔ `Turn` | `Move` enter (`0x58f8d0`) emits two `EnemyShot` cues; `0x58fdbc` / `0x58fe10` enter `Turn` |
| CannonBall | `0x6914ac` | `Up`/`ShootUp`/`TurnUp` ↔ `Down`/`ShootDown`/`TurnDown` | All six callbacks (`0x587710` … `0x587580`) are reachable; player position selects the turn state; shots at `0x587300` / `0x587150` |

The portable machines reproduce these cycles and emit aimed `EnemyShot`
projectiles (180 px/s) at the recovered firing states. ExplosivePlatform's
explosion is a short-lived `EnemyExplosion` marker. The shot/projectile speed
and the exact contact boxes remain reconstruction knobs where the original
gun object is not yet tied to the owning spawn.

## Boss state machines and hashed gun parameters

The shared damage routine `0x420380` treats `+0x150` as a small integer HP
value: weapon damage is subtracted, and reaching zero takes the death path
(`0x5b51f0`). WilyBoss has the same comparison inlined at `0x575ba0`. The base
Boss `Pose` update `0x5a3690` fills that meter from 0 to **28** in `0.08 s`
steps (2.24 s total). The earlier portable HP values of 800–1500 were
invented; controllers now use a full meter of 28.
`tools/analyze_boss_damage.py` resolves the previously unrecorded key use: when
the meter reaches zero, `FUN_00420380` (and the `+0x160` variant
`FUN_00420690`) reads GameParam `0xa96bff8d` through `FUN_0043c800` and compares
the result with the double constant `1.0` at `0x697ce8` (`fcomp` / `fnstsw` /
`test ah,0x44` / `jp`, an equality test). The single decoded `guns.mmu` value
for that key is `1.0` (object `0xebecc443`), so the depleted branch returns raw
code `4`; `FUN_00420450` dispatches `4` to a spawn plus `FUN_004047a0(1)`. The
subtracted amount itself comes from `FUN_005b4e80`/`FUN_005b5160`, so this key
is not a per-weapon damage value. Raw helpers, meters, result codes and callers
are in `docs/boss_damage.txt`: 11 of the 15 call sites resolve through RTTI
vftable stores to LuggageMan, Skoripion, ChickenBomb, BulletTrap, SniperJoe,
TankJoe, GrenadeSoldier, MineP, MineE, Leprechaun and LaserSpreader. Two more
resolve by unique RTTI vftable membership: `0x58b710` belongs only to MetTrois
(`0x69115c`, slot 3) and `0x5da240` only to SpreadMine (`0x68ef6c`, slot 3).
The two remaining sites are the generic helper wrappers at `0x420450` and
`0x420570`, which no vftable references. The compared result codes (`0/2/3/4`)
are recorded per call without assigning them meanings.
`bossdamage [summary|calls|owner <name>|call <va>|bits <hex>]` lists the raw
caller rows, including the two unresolved generic-helper wrappers, without
turning a result-code bit into a gameplay outcome.
`tools/analyze_damage_wrappers.py` extends that join one call outward:
`docs/damage_wrappers.txt` and `DamageWrapperTable.h` record four call sites
into the wrappers themselves (`0x5858e5`, `0x590675`, `0x5a6f5f`, `0x56438f`),
their enclosing handlers and the same owner resolution rules. Sweeper
(`0x5858d0`, unique vftable member `0x691664` slot 3), Leprechaun (`0x590660`,
vftable store `0x690c0c`) and Bullet (`0x564320`, vftable store `0x693414`)
resolve; the handler `0x5a6ef0` at `0x5a6f5f` does not, so its owner stays
empty. The wrapper VAs, helper VAs, meter offsets and result-code bits are
preserved as raw constants; no shared base handler is inferred as an owner.
`tools/analyze_enemy_state_machines.py` joins those thirteen owners to their
constructor state registrations by exact RTTI class name and vftable VA. The
result in `docs/enemy_state_machines.txt` / `EnemyStateGraph.h` records 28 raw
states and 35 setter/cue edges: Skoripion starts in `Walk`, GrenadeSoldier and
Leprechaun in `Hidden`, MineP in `Scan`, SniperJoe in `Stand`, TankJoe and
LuggageMan in `Move`, BulletTrap/LaserSpreader in `Stand`, MetTrois in `Hidden`
and SpreadMine in `Idle`. These replace the former `Patrol`/`Generic`
classification for those names; no update timing or transition trigger is
asserted, so their controllers hold the initial state until direct evidence
adds more. The tool also disassembles each constructor and reads the three
`jmp dword ptr [eax + 0xNN]` thunks materialised before each `0x401370`
registration. This gives 28 state/slot blocks: for example
SniperJoe owns `Fire` 23–25, `Jump` 26–28 and `Stand` 29–31, so its slot-24
edges are attributed to `Fire`, slot-27 to `Jump` and slot-30 to `Stand`;
MineP owns `Flash` 26–28 and `Burst` 29–31. Edges at slot 3 are outside all
state blocks and stay unattributed.
`tools/analyze_enemy_edge_facts.py` disassembles each of those 35 callbacks
from entry through the setter/cue call and records the raw branch operands in
`docs/enemy_edge_facts.txt` / `EnemyEdgeFacts.h`: 126 comparison/test
instructions, 15 qword constant addresses with their decoded double values,
register/stack memory operands, and the printable `.rdata` key pushed before
each `setState`. For example, LuggageMan slot 3 calls `0x5a6f4b` with key
`0x67d508` (`"Glued"`) after `cmp dword ptr [ebp - 8], 0x11` and `0x13`;
Skoripion slot 24 cues `0x583144` with key `0x67ae1c` (`"SkoripionShot"`) after
`fcomp qword ptr [0x697ce0]` (`0.0`) and compares `[edx + 0x1a4]` with `0`
and `1` and `[ecx + 0x1a4]` with `2`; MetTrois slot 33 calls `0x58b49e` with
key `0x67b7a8` (`"Hiding"`) after comparisons against `0x698a20`
(`0.800000012120929`); and SpreadMine slot 3 calls `0x5da294` with key
`0x681ea4` (`"Explode"`) after `cmp dword ptr [ebp - 0x10], 0` and
`cmp dword ptr [ebp - 4], 3/0`. These are verbatim operands; no trigger,
timing, field name, or gameplay meaning is inferred, and `enemyfacts` exposes
the table for inspection.

The base `Boss` vtable (`0x6900cc`) has its state callback block at slots
23–30 (`0x5a3790`, `0x5a3ad0`, `0x5a3640`, `0x5a3a60`, `0x5a3690`,
`0x5a3630`, `0x4d73fa`, `0x5a3e20`). Its initial state is **`Spawn`**
(`0x5a42ca`), not `Intro`: the enter callback sets a 400 px/s descent and a
start point of `player.x + 216`, `player.y - 300`; `0x5a3ad0` descends to
`player.y + 112` before entering `Pose`. Slot 29 is a pure virtual attack hook
that concrete robot masters override.

| Class | Vtable | Recovered entry state and states |
|-------|--------|----------------------------------|
| `MidBoss` | `0x691b94` | Only `MidBossExplosion` (`0x580290`/`0x5804d0`/`0x5801d0`); the base class has no attack cycle. The portable `Intro` cycle stands in for concrete robot-master subclasses. |
| `WilyBoss` | `0x692794` | Initial `Idle` (`0x5767a5`); `EnergyFill` (`0x575490` triple) and `WilyBossExplosion` (`0x575780` triple). |
| `WilyMachine` | `0x692cc4` | Initial `Wait` (`0x56eb95`); `Hover`, `GotoAttack1`..`4`, `Attack3`, `PillarDrop`, `Lightning`, `Ram`, `RamReturn`, `Crush`. |
| `WilyCapsule` | `0x692e94` | Initial `FlyUp` (`0x56bbb1`); `Wait` and `FlyDown`. Transitions are position-conditional: `-17 px` above the player, a `5.0 s` wait, then `+170 px` below. |

`EnemyController::update` mirrors the high-confidence topology. The base
`MidBoss` death chain `Death` → `BigExplosion` → `MidBossExplosion` was not
present in the binary and is retained only as a generic-subclass fallback;
WilyBoss death now enters `WilyBossExplosion`.

### WilyMachine attack timing and projectiles

The constructor at `0x56dd70` registers twelve states and ends with `setState`
at `0x56eb95`, so the machine starts in `Wait`. The state triple table is:

| State | [exit / update / enter] |
|-------|-------------------------|
| `Crush` | `0x56c8a0` / `0x56d940` / `0x56ccf0` |
| `Hover` | `0x56c8c0` / `0x56da10` / `0x56ce60` |
| `GotoAttack1` | `0x56c8b0` / `0x56d510` / `0x56cdb0` |
| `GotoAttack2` | `0x56c870` / `0x56d370` / `0x56cbf0` |
| `GotoAttack3` | `0x56c860` / `0x56d2b0` / `0x56cb20` |
| `GotoAttack4` | `0x56c7e0` / `0x56d240` / `0x56ca70` |
| `PillarDrop` | `0x56c880` / `0x56c890` (empty) / `0x56ccb0` |
| `Lightning` | `0x56c7f0` / `0x56d7a0` / `0x56c800` |
| `Attack3` | `0x56c830` / `0x56d8e0` / `0x56c840` |
| `Ram` | `0x56c750` / `0x56d180` / `0x56c760` |
| `RamReturn` | `0x56c6f0` / `0x56d730` / `0x56c700` |
| `Wait` | `0x56c8d0` / `0x56cec0` / `0x56c8e0` |

Selector `0x56d580` reads the `+0x368` phase (0–3) and enters `GotoAttack1`,
`GotoAttack2`, `GotoAttack3` or `GotoAttack4`, which are setup/movement states
for `PillarDrop`, `Lightning`, `Attack3` and `Ram` in that order. This
corrects the earlier registration-order guess.
`tools/analyze_wily_attack_selector.py` now recovers the exact jump table into
`docs/wily_attack_selector.txt` at
`0x56d644` and the literal state-name strings at `0x678fa8`–`0x678fcc`: phases
`0/1/2/3` map to `GotoAttack1/2/3/4`. Phases 1 and 2 also clear the raw counter
`+0x374`; phase 3 writes `0` back to `+0x368`, while the other branches fall
through to the `+1` increment at `0x56d628`. The only caller is the Hover update
at `0x56da4a`, reached when the `+0x364` timer expires. The raw `0xf` event
handler `0x56dca0` selects `EnergyFill` for phase 0 and `Crush` for phase 1.
`wily [summary|phases|phase <n>|raw]` lists the four phase targets, counter-clear
bytes, raw field offsets, selector/jump-table/caller VAs and wrap constant
verbatim; phase numbers are not given gameplay labels.
The portable model keeps its monotonic debug counter but applies the recovered
counter-clear rule. The hashed GameParams read by
these callbacks resolve against `guns.mmu` as `0xc4cfff3e` = 2.0 s,
`0xefdf17dd` = 1.0 s, `0x7ffa2ceb` = 0.75 s, `0x6e2ae279` = 40.0,
`0x4c2d9a15` = 50.0, `0x45093409`/`0x4509340a` = 63, `0xb7099e6c` = 79 and
`0xb7099e6f` = 128. Ram travel uses 186 px/s out and 62 px/s back.

Enemy controllers emit `SpawnedObjectEvent` records; `World` instantiates them
as real `WorldObject`s with the exact type strings `WilyMachinePillar`,
`WilyMachineLightning`, `WilyMachineShot` and `WilyMachineBlock`. The decoded
`guns.mmu` objects resolve the previously unknown constants:

| Object key | Role | Parameters |
|------------|------|------------|
| `0xb4cb8fba` | `WilyMachineShot` | speed `0x57469074` = `250`, damage `0xc167f1a0` = `4` |
| `0xec182c21` | `WilyMachineBlock` | speed `0x57469074` = `230`, damage `0xc167f1a0` = `4` |
| `0x45b4d5d7` | `WilyMachineLightning` | lifetime `0xfc3e6048` = `1.6 s`, damage `6` |
| `0xf47b10d7` | `WilyMachinePillar` | drop `80`, lift `50`, damage `6`, first block `0.35 s`, interval `0.6 s` |
| `0x7af30f7` | `WilyMachine` | setup `2.0 s`, post-attack `1.0 s`, volley `0.75 s`, offsets `79/63` and `128/63` |

`0xfd50df10` is `1` on the pillar and is not its damage key; `0xc167f1a0` is
the shared damage key and resolves to `6`. This corrects the earlier
`135`/`6` shot and `1`-damage pillar notes.

The pillar is a persistent object: `Wait`'s update `0x56cec0` spawns it once and
stores it at `WilyMachine+0x36c`. `PillarDrop`'s enter `0x56ccb0` does **not**
spawn another pillar; it sends event `0xf` to that stored object. The pillar
event handler `0x56bdd0` maps `0xf` to `Drop` and `0x20` back to `Follow`;
`Crush`'s update `0x56d940` sends the `0x20` release after its timer expires.
`World::routeProjectileCue` reproduces that routing and resets the pillar's
block counter when it returns to `Follow`.

`GotoAttack3`'s enter `0x56cb20` calls `0x56cf70` once, so the three homing shots
with `1/2/3 s` activation delays are emitted on entering `GotoAttack3`, not on
entering `Attack3`. `Attack3` then lasts only `0.75 s` (`0x7ffa2ceb`) before
returning to `Hover`. `GotoAttack1` steers toward `player + (90, 80)`, while
`Hover` steers toward `player + (128, 63)`. Lightning still fires exactly two
strikes `2.0 s` apart, then waits `1.0 s` before returning to `Hover`.

The dropped pillar fires three `WilyMachineBlock` objects at offsets
`(-10, -10)`, `(-10, -26)` and `(-10, -42)`. The first fires after `0.35 s`,
then cadence is `0.6 s`. Blocks fly outward at `230 px/s`, pause, and return at
the same speed (the registered `Fly`/`Wait`/`Return` states). All projectiles
are integrated, can damage the player, and are listed by the `projectiles`
console command. Their art is resolved from packed image paths, with a
primitive debug marker where no path exists.

The numeric parameters are stored under 32-bit `GameParam` keys.
`tools/extract_gun_params.py` scans `MMU.exe` for `mov edi,<hash>;
call 0x43c800` sequences and groups them by the enclosing constructor's
vftable in `docs/gun_params.txt`.
`gunparams [summary|objects|object <name>|key <hex>|lookup <object> <key>]`
lists the decoded object keys and raw values; object ownership remains the
generator's key-overlap inference, not a gameplay classification.
`tools/resolve_gun_objects.py` maps each constructor to a decoded
`guns.mmu` object using sound-path directory hints, key overlap, and the known
Telly override (`0xaabc064d`), then emits `core/include/mmu/core/GunParams.h`.
`tools/crack_hash.py` resolves the keys through the non-reflected CRC-32 at
`0x43c010`. Important names include `MoveSpeed` (`0x57469074`), `MoveDistance`
(`0xda1f1441`), `HitPoints` (`0xa21be30b`), `TargetHotSpotY/X`
(`0x42714480`/`0x42714481`), `ShootDelay` (`0xe1e66eb0`), `JumpHeight`
(`0xfb2481ab`), `ExplosionDelay` (`0xc3ae9999`) and `IdleDelay` (`0xb98e43bd`).
`EnemyState.cpp` reads Telly speed/distance/hotspots, SawPole speed/height and
SuperCannon fire interval through this table instead of hardcoding them. The
full verified dictionary is `docs/key_names.txt`;
`tools/generate_verified_key_names.py` copies those 137 exact matches into
`core/include/mmu/core/VerifiedKeyNames.h`, and
`keynames [summary|list|key <hex>|name <text>]` queries them. No candidate report
is imported into that table. The five MITM hits are kept separately by
`tools/generate_key_name_candidates.py` and printed by
`keycandidates [summary|list|key <hex>]` with an `UNVERIFIED` label;
`tools/generate_key_sweep_summary.py` copies only the four counters from the
84,651-candidate sweep report into `KeySweepSummary.h`; `keysweep [summary]`
prints them without importing candidate strings.
`tools/sweep_key_names.py`
adds names from curated token combinations, and `tools/mitm_key_names.py` uses
CRC linearity with a curated vocabulary; both write candidates for review
(`docs/key_names_discovered.txt`, `docs/key_names_mitm.txt`). Unrestricted
binary-string dictionaries were tried and rejected: they generate CRC
collisions, not evidence.

## Cutscene command runtime

`CutSceneVault` is a thin wrapper around an embedded `mmu::Database` (ctor
`0x405480`, global `0x6c55d0`); `0x405380` is `SpritesVault`, not the cutscene
vault. `Assets/cutscenedefs.mmu` decodes to 80 cutscenes, 3,379 timestamps and
7,513 commands. `tools/analyze_cutscene_keys.py` matched all 80 database keys to
the embedded legacy `.xcut` paths with the same non-reflected CRC-32 as the
named-state dispatcher, closing the key-to-name question; the mapping is in
`docs/cutscene_keys.txt`.
`cutscene keys [list|key <hex>|path <text>]` lists the 80 recovered raw key/path
rows separately from the baked asset table used by `cutscene list`; no new
hash is computed by the query.

The protobuf dispatcher is `0x621e00`; the legacy `.xcut` tokenizer dispatcher
is `0x6228A0`. Neither uses numeric opcodes: identity is `Command.name`
(field 1). The recognized strings are `DisplayObject`, `GetInGameObject`,
`MoveObject`, `PlayAnim`, `KillObject`, `PlaySound`, `StopSound`,
`PlayMusicLoop`, `PlayMusicOnce`, `WriteText`, `EraseText` and `EndScene`.
`EraseText` exists in the legacy string table but no effect was observed.
`EndScene` sets `BaseCutscene+0x34 = 1` and stops command processing.

MMUPAK v5 bakes the decoded cutscene command tables, so the shipped runtime no
longer needs the original `.mmu` files or a protobuf library. `CutsceneRuntime`
preserves timestamp order, element lifetime, `MoveObject` duration/speed
conversion, animation fields, text state and `EndScene`. The SDL2 front end
routes `PlaySound`, `PlayMusicLoop`, `PlayMusicOnce` and `StopSound` to baked
SFX/music by resource base name.

Element images follow the original resource chain. `DisplayObject` supplies a
resource such as `TextBG.xgun`; the runtime rebuilds
`..\Cutscenes\CSN00Logos\TextBG.xgun`, hashes it with the same non-reflected
CRC-32, and finds the `guns.mmu` object key `0x5122d3ed`. `PlayAnim` hashes its
animation name (`TextBG` -> `0xd727394d`) and indexes the object's parallel
`spriteKeys`/`sprites` arrays to reach sprite `0x4fcab796`, whose first packed
frame is `EmptyScreen.png`. `GetInGameObject` has no resource or animation
field; its id (`Bass`, `MegaMan`, ...) is matched against the baked editor-side
first-sprite path of each object, preferring an exact path component
(`MegaMan`) over a loose substring (`WilyMachine` before
`WilyMachine-Pillar`). MMUPAK v5 therefore also stores 316 object
records and 904 animation/sprite links. `resolveCutsceneSprite()` returns the
object, sprite and image keys, and the **View → Cutscene Runtime** panel renders
the packed image beside each live element. `cutscene list`, `cutscene start
CSN00Logos.xcut` (or just `CSN00Logos`, matched against the path base name),
`cutscene step 0.1`, `cutscene status` and `cutscene events` expose the decoded
stream for verification.

## Raw camera and room-transition evidence

`tools/analyze_camera_evidence.py` joins camera evidence that was already in the
checked-in reports; it adds no scroll speed, direction, trigger or room
destination. `docs/camera_evidence.txt` / `CameraEvidenceTable.h` record the
18 slots of `.?AVCamera@@` at vftable VA `0x69775c`, the 29 slots of
`.?AVCameraTarget@@` at `0x6924ec`, ctor `0x578aa0`, the complete `Idle`/
`Move` slot blocks (`0x578bef` hash `0x49646c65` slots `25,24,23`;
`0x578cd9` hash `0x4d6f7665` slots `28,27,26`), and the two observed setter
edges (`slot 3 func=0x5a6ef0 call=0x5a6f4b Glued`,
`slot 24 func=0x578a30 call=0x578a6e Move`). The verbatim operand table keeps
offsets `+0x58`, `+0x5c`, `+0x168`, `+0x16c`, `+0x178`, `+0x17c`, `+0x184`,
globals such as `_DAT_006987d0`, `_DAT_00698448`, `_DAT_00698e48`,
`_DAT_00698e40`, `_DAT_00698458`, `_DAT_00698790` and `_DAT_00698788`, and
literals such as `rand() % 5 + -2`; none is converted into a camera rule.
`camera [summary|slots|target|states|edges|operands|func <va>]` exposes those
rows. The portable `Camera::follow`/`clamp` blend and the harness
`x >= worldWidth() - 48.0f` scene advance remain explicitly labelled harness
policy, not a recovered transition.

## Raw player-spawn evidence

`tools/analyze_player_spawn_evidence.py` reads levels.mmu with the recovered
`mmu.Level`/`mmu.Scene`/`mmu.GameSpawnObject` schema and pairs scene order with
the exact editor paths in `tools/scene_names.inc`. It decodes 791 scenes and
4,926 spawn records with 82 unique exact `type` tokens. Exact-token checks are
`MegaMan=0`, `ProtoMan=0`, `Bass=0`, `Zero=0`, `Treble=0`, `ZPrototype=0`,
`Z-PROTOTYPE=0`, `MEGA MAN=0`, `player=0`, `Player=0`; `ShadowMegaMan=77` and
`MegaTank=3`. Lowercase substring counts are `megaman=77` and `player=0`, so
the current substring rule selected `ShadowMegaMan` records even though that
is a distinct exact token. `docs/player_spawn_evidence.txt` and
`PlayerSpawnEvidence.h` preserve the 77 records with scene index/path/key,
spawn index, x/y, packed flag word and raw `attr1`. For example the first row
is scene `247` (`\Levels\NitroMan_Section_Alt3.xscn`), key `0xf8bdc84b`,
spawn `5`, `type=ShadowMegaMan`, `x=465.0`, `y=192.0`, `flags=15`, `attr1=50`.
The flag derivation lines from `tools/bake_assets.cpp:594-598` and
`core/src/PackedArchive.cpp:214-218` are copied verbatim. No exact playable
spawn is designated; World's proxy now prefers exact `MegaMan`/`player`
equality and otherwise keeps index `0` as an explicit proxy.
`playerspawn [summary|checks|candidates|scene <n>|exact <token>]` queries the
raw table.

## Raw player-control evidence

`tools/analyze_player_control_evidence.py` joins already-recovered rows without
deriving timing or physics values. `docs/player_control_evidence.txt` and
`PlayerControlEvidenceTable.h` keep 16 registered state names/hashes (including
`Jump 0x4a756d70`, `Shoot 0x1dbbe2aa`, `Slide 0x19bde9bb`, `Charge 0xd8727fd5`,
`Charging 0xcf3c247b`, `CometDash 0x070772c3`), 17 verified parameter hashes
(`MoveSpeed 0x57469074`, `JumpHeight 0xfb2481ab`, `ShootDelay 0xe1e66eb0`,
`SlideSpeed 0x8846cdf9`, `ChargeTime 0xb40a8909`, and others), and 33
interest-only edge rows. MegaMan's raw state graph starts in `Wait` at
`0x5b47f9` and includes the `Slide` setter `func=0x5a8df0 call=0x5a8e30`;
ProtoMan includes `Charge` setters at `call=0x571fa8` and `call=0x572327` plus
`Shoot` at `call=0x571e96`; Bass includes `Buster` cues at `call=0x570376`,
`call=0x57056c`, `call=0x5706f3`, `Shoot` at `call=0x56fcc7` and `Jump` at
`call=0x5708c3`. MegaTankShot starts in `Charging` and has `ChargeUp` cue
`hash=0xcf301f6c call=0x55f241`. No MegaMan `Charge`/`Charging` setter appears
in the joined rows, so none is claimed for the portable player.

Character-select rows remain exact: `MEGA MAN` is action `0x48`, label VA
`0x68d794`, object `0x6c580c`, value `0x61dc80(U,0)`; `Z-PROTOTYPE` is action
`0x49`, label VA `0x68d7a0`, object `0x6c582c`, value `0x61dc80(U,1)`.
`docs/protos/savedgame.proto:15` is copied verbatim as
`optional bool use_charge_shot = 10 [default = false];`. The recoverable icon
paths include `..\Menus\Data\MegaBusterIcon.xspr` and
`..\Menus\Data\ZDoubleJumpIcon.xspr` with exact keys `0x55ede1ae` and
`0xeea22b3c`. The joined 82 decompiled operand lines preserve the exact
strings and masks around `0x5b02f0`, `0x5aa470`, `0x5b15d0`, `0x5aea90`,
`0x5aefc0` and `0x5a7a50` without assigning trigger or duration meanings.
`playerctl [summary|states|keys|edges|chars|resources|operands|state <name>|key <name>|func <va>]`
exposes the table. The portable speed/gravity/jump/knockback constants and the
slide/shoot hooks remain explicitly unverified placeholders; in particular the
current slide hook follows the `CometDash` early return, so it is not claimed
as a reconstructed Slide entry.
