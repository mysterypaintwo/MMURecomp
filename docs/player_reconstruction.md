# MegaMan movement and collision reconstruction

This document covers the engine pieces MegaMan's movement depends on (input,
timing, GameParams, the state machine, per-pixel collision) and the
reconstructed states. Every claim names the MMU.exe function it comes from.
The function-level ledger is `docs/reconstructed_functions.txt`; coverage
counts come from `tools/generate_coverage_report.py`.

MMU.exe is a native 32-bit C++ program (MSVC 2008, SDL 1.2). It only looks
like an NES game: positions and speeds are floats in pixels and seconds, and
every frame is scaled by a measured delta time.

## Workflow and tooling

| Tool | Purpose |
|------|---------|
| `tools/DecompileAll.java` | Ghidra headless script. Dumps every function as `<va>.c` plus an `index.txt` (VA, name, size, callees). Optional args: start VA, register-signature file, end VA. |
| `tools/register_signatures.txt` | 155 helpers that take arguments in registers (whole-program optimisation). The script appends register parameters so calls show their real arguments, e.g. `FUN_00441db0(0x16)` instead of `FUN_00441db0()`. |
| `tools/re_annotate.py` | Annotates decompiled C or disassembly with the values of `_DAT_`/`DAT_` constants (f32/f64/string) and names hashed immediates (CRC-32 of every exe string + `docs/key_names.txt`). |
| `docs/function_inventory.txt` | All 16,957 functions (VA, size, name), the coverage denominator. |

```sh
export JAVA_HOME=/opt/homebrew/opt/openjdk@25/libexec/openjdk.jdk/Contents/Home
OUT=/tmp/mmu_decomp   # ~70 MB; keep it outside the repo
analyzeHeadless ghidra_project MMU -process -noanalysis -readOnly \
  -scriptPath tools -postScript DecompileAll.java "$OUT" 0 tools/register_signatures.txt
python3 tools/re_annotate.py c "$OUT/0x5aea50.c"      # Fall.update, annotated
python3 tools/re_annotate.py dis 0x5afed0 60          # raw disassembly
python3 tools/re_annotate.py hash move_speed          # 0x9f2d439d
```

The script also repairs two helpers that Ghidra's analysis marked
"no return" although they contain `RET` (one is the `std::list` iterator
increment at `0x42f830`). Before that repair, 105 callers had their loops cut
off at the first iterator step, including the collision contact loop.

Read `(float)_DAT_xxx` casts as doubles (`d=` in the annotation).

## Engine facts

**Timing.** `0x43e4b0` sets the frame delta to `(SDL_GetTicks() - last) / 1000`.
The getter `0x43e510` caps it at 0.031 s. The harness steps a fixed 1/60 s,
which matches at 60 Hz. Timers that the game measures with `SDL_GetTicks`
(jump buffer, ladder regrab, LadderOut) run on an internal millisecond clock
in `PlayerController`.

**Input.** `0x440880` maps `options.xopts` controls to action ids. Gameplay
context: right `0x15`, left `0x16`, up `0x17`, down `0x18`, jump `0x1e`,
slide `0x1f`, fire `0x20`, next/previous weapon `0x22`/`0x23`, start `0x25`,
select `0x26`. Menu context: right `0x19`, left `0x1a`, up `0x1b`, down
`0x1c`, start `0x1d`. Each action has flags (`0x441820`): 1 = up,
2 = released this frame, 4 = held, 8 = pressed this frame. `0x441db0(id)`
means held and `0x441d90(id)` means pressed.

**GameParams.** `0x43c800(obj, hash)` linearly searches the object's
`guns.mmu` params and returns 0.0 if the key is absent. MegaMan is object
`0xc31e776a` (`Units\MegaMan\megaman.xgun`). Archive v6 bakes these rows
(`PackedArchive::gameParams()`, `paramOf()`), and `PlayerParams::loadFrom`
reads them:

| Key | Value | Used by | Meaning |
|-----|-------|---------|---------|
| `0x9f2d439d` move_speed | 90 | Run, Fall, Ladder jump-off | ground speed, air max |
| `0x16c2433f` slide_speed | 150 | Slide | slide speed |
| `0x89c83879` max_fall_speed | 500 | Fall | terminal velocity |
| `0xccf2a219` | -325 | `0x5a9de0`, Fall landing | jump velocity (× sign of gravity) |
| `0xab528d4a` | -50 | Fall | releasing jump while rising caps the upward speed at 50 |
| `0xc5e0085d` | 8000 | Fall | air acceleration while left/right is held |
| `0xc5e50e5d` | 2000 | Fall | air deceleration with no input |
| `0xba334ef9` | 50 | Fall landing | ms of jump/slide buffer before touchdown |
| `0x3944b87d` | 55 | Slide.update | slide distance in px |
| `0x731a31b3` | 75 | Ladder.update | climb speed |
| `0x863126a0` | 20 | Ladder, LadderOut | ladder-top probe distance |
| `0x6f213b03` | 14 | Ladder → LadderOut | climb-out height |
| `0x87d1161e` | 50 | LadderOut.update | ms before LadderOut resolves |
| `0xe1e903e4` | 200 | `0x5a97d0` | ms before a ladder can be re-grabbed |
| `0xbc49de4e` Fly_Speed | 400 | Fly | not reconstructed yet |
| `0x5225056c` | 0.75 | weapons/charge (8 sites) | not reconstructed yet |
| `0x13b66fc1` | 0.3 | `0x5ad160` shooting | not reconstructed yet |
| `0x422c14db` ChargeFlashDuration | 0.1 | charge | not reconstructed yet |

**Gravity.** Level setups write the global `_DAT_006c5598 = 990.0`; some write
375. `0x43c840` returns 375 while the object touches water (type
`0x2000000`). `0x5b15d0` negates the global for gravity-flip zones. Every
vertical rule is written in terms of `sign(gravity)`.

**State machine.** Each class registers its states in its constructor
(`0x401370`). For state *n* the vtable slots are enter = `18+3n`,
update = `19+3n`, exit = `20+3n` (MegaMan: 99 slots, 27 states; Stand is
`0x5a7a50`/`0x5afed0`/`0x5a7360`). `0x4405c0` switches state by name and runs
the exit and enter callbacks at once. A state set to itself is re-entered,
which Fall uses after hitting a ceiling. Per frame, MegaMan runs slot 11
(`0x5aa470`, which calls the current state's update through `0x430bc0`),
then slot 14 (`0x5b15d0`, which gathers contacts through `0x43cc00`).

## Per-pixel collision

There are no tiles or boxes.

* **Types.** `0x445130` maps an exact RGB colour to one bit. Examples:
  white `1` solid, `(0,0,127)` `2`, red `4` hazard/spikes, `(255,200,0)` `8`
  no-walk, cyan `0x10` ice, magenta `0x20` ladder, `(110,110,0)` `0x200` and
  green `0x20000` one-way, `(50,80,130)` `0x2000000` water, `(155,0,255)`
  `0x200000` instant death. The full table is `collisionTypeForRgb` in
  `core/src/Collision.cpp`, and the named constants are in
  `core/include/mmu/core/Collision.h`.
* **Environment.** `0x5b4970` converts the room's collision image (from
  `levelart.mmu`) into a per-pixel `u32` type map, built here as
  `World::levelTypes()`.
* **Bodies.** Each animation frame has a collision surface coloured with the
  same table. MegaMan's frames all share the sprite-level `single_collision`
  surface (`Collisions/megaman.png`). The baker now links it to the frames,
  and `PackedArchive::load` infers the link for older archives. Left-facing
  bodies are mirrored (`zoomSurface(-1,1)` in `0x5aa080`).
* **Test.** `0x445b20` places the body at `floor(screen position) - handle`
  and checks only pixels whose colour maps to a non-zero type. For each one
  it looks up the Environment at `trunc(camera) + pixel`. The test runs in
  screen space inside a 456×424 grid (x ∈ [-100, 356), y ∈ [-100, 324)).
  The real engine also rasterises live objects into that grid each frame
  (`0x445770` clears it); that part is **not** modelled yet.
* **Sweep.** `0x5b5220` steps the dominant axis one pixel at a time (the minor
  axis follows the slope). When blocked it tries each axis on its own and
  reports flags 1/2 (vertical) and 4/8 (horizontal). In a vertical-major
  sweep it zeroes `vy` when it cannot advance, which is how Fall detects
  landing and ceilings. One-way types (`0x200`, `0x20000`) block only a body
  moving with gravity that is not already overlapping them. Test y is
  clamped to screen rows [1, 223] (`0x5b4f30`). Port: `sweepMove`.
* **Contacts.** Slot 14 (`0x43cc00`) ORs every type under the body into
  `+0x98`, which states read as `contactFlags`.

## States

| State | Enter / Update / Exit | Reconstructed behaviour |
|-------|----------------------|-------------------------|
| Stand | `0x5a7a50` / `0x5afed0` / — | Zeroes velocity unless on ice. Ice keeps momentum and decays at 80 px/s². Left/right enters Step unless on no-walk ground. Then, in order: slide → ledge → jump → ladder. |
| Step | `0x5a8590` / `0x5afce0` / — | Enter nudges 1 px (wall mask 1 when moving right, 3 when moving left, as in the binary). Becomes Run when the Step animation finishes. |
| Run | `0x5a7310` / `0x5af930` / — | Constant ±move_speed (ice: ±80 px/s², type `0x4000`: 0). Sweep with mask 3. Still runs the slide/jump checks on the frame it returns to Stand. |
| Fall | `0x5a7eb0` / `0x5aea50` / `0x5a8d50` | Air accel 8000 / decel 2000, clamp ±move_speed, gravity, clamp ±500, jump cut to 50 on release, sweep with mask 3 + one-way. On landing: buffered jump (<50 ms) → jump again, buffered slide → Slide, otherwise Stand/Run. Ceiling → re-enter Fall. Wind (`0x8000`/`0x10000`) and bounce (`0x10000000`, vy = −60). |
| Slide | `0x5ab980` / `0x5ae630` / — | slide_speed for 55 px. Keeps sliding with no headroom (checked with the "stand" body) and reverses on opposite input there. Jumping is allowed when there is headroom. Ledge: switches to the "jump" body 12 px lower → Fall. |
| Ladder | `0x5a77c0` / `0x5adfc0` / `0x5a7280` | Climbs 75 px/s. Within 20 px of the top it finds the top edge and climbs out (LadderOut, 14 px). Descends with a sweep; floor → Stand; no ladder below → Fall. Jump drops off with ±move_speed. Exit starts the 200 ms regrab timer. |
| LadderOut | `0x5a7710` / `0x5adde0` / — | After 50 ms: from the top or descending → Ladder; climbing → Stand 15 px higher. |
| Hurt | `0x5acdc0` / `0x5acbf0` / `0x5a8030` | 1.5 s invincibility, −32 px/s knockback. The 0.5 s duration is an approximation. |

Ladder grabbing (`0x5a97d0`) needs up (or down, from the ground), a ladder
pixel 1 px in that direction, and 200 ms since the last exit. x snaps by the
first contact's dx. Ledges (`0x5a9c40`) probe 1 px toward gravity with
`3 | ladder | one-way`, ignoring the types already overlapped, so ladder tops
act as one-way floors.

Deaths (`0x5aa470`, `0x5b15d0`): more than 244 px below the camera top
(< −20 under inverted gravity), touching type `0x200000`, or touching spikes
(`4`/`0x8000000`) while not invincible. The original's spike test also looks
at `+0x1a0`/`+0x1a2` (debug invincibility) and an easy-mode branch, which
are not ported.

## Not reconstructed yet (MegaMan)

* Shooting and weapons: `0x5ad160` (2.5 KB, the shoot/charge driver), the
  `0x5a8730`/`0x5a87c0` per-frame helpers, weapon switching, and
  `WeaponAbsorb`.
* Hurt variants (`HurtKnockback`, `HurtSlide`), Death animation and respawn
  flow (`0x5b1c10`), Spawn/Teleport beam (`0x5a8160`/`0x5a9110`).
* Ride, Carried, Glued, Stun, Fly, CometDash, and the five Z-Prototype states
  (the `+0x3dc` branches).
* Live-object collision: platforms, enemies and shots in the screen grid;
  conveyor geometry (`+0x10c`); crush detection (`+0xa4`).
* Camera and room transitions (`Camera` vftable `0x69775c`,
  `docs/camera_evidence.txt`). The harness still uses a follow camera.
* Save/load (`proto/savedgame.proto`): nothing reads or writes
  `savedgame.mmu` yet.
