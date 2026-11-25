# Static recompilation of MMU.exe

`mmu_recomp` runs the original game logic: MMU.exe and the libraries it ships
are translated instruction by instruction into C (`tools/recomp/lift.py`) and
compiled for the host. Nothing is hand-reimplemented except the OS layer. This
complements the documented reconstruction in `core/` (see
`player_reconstruction.md`): the goal is to run *exactly* like Windows first
and to replace/document functions afterwards.

## Build and run

```sh
pip3 install -r tools/recomp/requirements.txt     # capstone, protobuf, Pillow (+ unicorn for difftest)
make macos                                         # builds build-macos/recomp/mmu_recomp
./build-macos/recomp/mmu_recomp /path/to/MegaManUnlimited
```

Without an argument, desktop builds look for the game in `game/` next to the
program, then next to the program itself, then `./game` (the release layout).

### Switch 2

```sh
make switch2          # -> build-switch2/recomp/mmu_recomp.nro
```

Copy the whole original `MegaManUnlimited` folder (MMU.exe, all DLLs,
`Assets/`, `options.xopts`) to `sdmc:/switch/mmu_recomp/game/` and launch
`mmu_recomp.nro`. The folder must be writable (settings and saves). Optionally
`-DMMU_RECOMP_ROMFS_DIR=<dir containing game/>` embeds a read-only fallback.
The runtime allocates the whole guest space up front (`RT_GUEST_SIZE`,
~1.4 GiB; the game peaks near 1 GiB), so run it with full application memory
(title override), not in applet mode.

The recomp does not use `Assets/assets.pak` (that is the reconstruction's
baked pak), so it can be left off the SD card.

**Logs** (`recomp/src/log.cpp`), in `sdmc:/switch/mmu_recomp/`:

* `log.txt`: everything the runtime and the game print (stdout line-buffered,
  stderr unbuffered on the same descriptor), rewritten on every launch and
  capped at 2000 lines so it never grows on a player's SD card (crash reports
  are always written).
* `boot.txt`: unbuffered, time-stamped breadcrumbs (start, applet type and
  memory, game loaded, heap exhausted, ExitProcess, crash), so a hard crash
  still shows how far the run got; rewritten per launch, 300 lines.
* **Developer logging**: create a file named `debug_log` next to the NRO.
  Then the line caps are off, a status line every 2 s (`RECOMP_STATUS=s`)
  gives frame/FPS, guest heap use, process memory and the current guest
  function, a sampling profiler prints the top guest functions every 10 s,
  and the NRO serves its log over the network (the console cannot run an FTP
  server next to the game): `nc <switch-ip> 28280` (or
  `tools/recomp/watch_switch_log.sh <switch-ip>`, which reconnects) streams the
  last 64 KiB and then every new line. The IP is printed in boot.txt. Players
  get none of this (no sockets are opened).
* Crashes: the libnx exception handler prints the PC/LR as NRO offsets
  (`aarch64-none-elf-addr2line -e build-switch2/recomp/mmu_recomp <offset>`),
  the fault address (also as a guest address), registers and the last 64
  guest function entries. Horizon may not deliver every fault to the process;
  then the breadcrumbs and the last status line are the evidence.
* Launched with `nxlink -s mmu_recomp.nro` (hbmenu netloader), stdout/stderr
  stream to the PC terminal instead of log.txt.

On the host, the same status lines are available with `RECOMP_STATUS=1`, the
profiler with `RECOMP_PROFILE=1`, and fatal signals print the guest call ring.

**Controls**: the game only reads joystick 0 through SDL events (0x440d30:
axes 0-2, hat 0 and buttons 0-9 become virtual keys 323-342; bindings in
`options.xopts` `<joystick-controls>`). On Switch the controller is presented
as a *virtual pad* whose button numbers match the game's factory defaults
(0x4fbb10, "RESTORE DEFAULT"):

| Switch | virtual | action |
|--------|---------|--------|
| B (south) | b1 | jump |
| Y (west) | b0 | fire |
| ZR | b2 | slide |
| L | b5 | next weapon (the game's "L Button") |
| R | b4 | previous weapon |
| + / − | b9 / b8 | start / select |
| A, X, ZL | b3, b6, b7 | free for remapping |
| D-pad / left stick | hat / axes 0-1 | move |

The `<joystick-controls>` in the SD copy of `options.xopts` must hold the
defaults (fire 331, jump 332, slide 333, previous 335, next 336, select 339,
start 340, right/left/up/down 323-326) or "RESTORE DEFAULT" must be chosen in
the options menu; in-game remapping works on top. `RECOMP_VIRTUAL_PAD=1`
enables the same layout on desktop (positional, e.g. Xbox A = jump);
otherwise desktop passes the raw SDL joystick through.

`MMU_GAME_DIR` (CMake, `GAME=` for make) / the first argument is the original game folder
(MMU.exe, the DLLs and `Assets/`). The generated C (~29k functions, 48 files)
lives only in `build-*/recomp/gen` and is derived from your own copy of the
game: never commit it.

Debug environment variables:

| Variable | Effect |
|----------|--------|
| `RECOMP_SCREENSHOT=f.bmp`, `RECOMP_SCREENSHOT_AT=s` | save the presented frame (alpha channel is 0: view without alpha) |
| `RECOMP_QUIT_AFTER=s` | exit after `s` seconds of game time |
| `RECOMP_DUMPTEX=prefix`, `RECOMP_DUMPTEX_EVERY=n` / `_AT=n` | dump the game's own uploaded frame textures |
| `RECOMP_GLTRACE=1` | first calls of every emulated GL entry point |
| `RECOMP_TRACE_LOAD=1` | LoadLibrary / GetProcAddress |
| `RECOMP_AUDIO_STATS=1` | RMS of the mixed audio once per second |
| `RECOMP_TRACE_MIXER=1` | log Mix_LoadMUS_RW / Mix_PlayMusic / Mix_VolumeMusic |
| `RECOMP_LOCKSTEP=dir` | snapshot + trace for tools/recomp/lockstep.py |
| `RECOMP_STACK_NOTES=1` | functions returning with unbalanced ESP (expected for SEH helpers) |
| `RECOMP_PROFILE=1` | sampling profiler: top guest functions every 10 s |
| `RECOMP_VERIFY_BLIT=1` / `RECOMP_ORIGINAL_BLIT=1` | run the native pixel-alpha blitter and the lifted original on the same pixels and count differences / use only the original |
| `RECOMP_BENCH_GFX=1` | time the lifted SDL_gfx calls the game uses (filledTrigonRGBA, boxRGBA, zoomSurface) |
| `RECOMP_TRACE_STATES=1` | log every state change (`state: Select`), pad events and the A/B menu bindings |
| `RECOMP_AUTOPRESS=6,9,22:H,25:S,27:pad:b` | input script: tap keys (SDL key names, `up`/`down`/`esc`, default Return) or pad buttons at those times (seconds) |
| `RECOMP_FAKE_PAD=1` | attach a virtual game controller for `pad:` presses (with `RECOMP_VIRTUAL_PAD=1`) |
| `RECOMP_SWITCH_TEXT=1`, `RECOMP_NO_KEYBOARD_MENU=1` | preview the Switch wording / options menu on desktop |

Faults print the last 64 function entries. Unknown indirect-call targets are
appended to `recomp_missing_entries.txt`; add them to
`tools/recomp/extra_entries.txt`.

## What is translated vs. shimmed

Translated (original machine code → C), rebased into a ~1.4 GiB guest address
space laid out like a 32-bit Windows process (`recomp/include/recomp/cpu.h`):
MMU.exe, msvcr90, msvcp90, SDL 1.2, SDL_gfx 2.0.24, SDL_image, SDL_mixer,
libogg/libvorbis/libvorbisfile (MinGW; their `msvcrt.dll` imports bind to the
translated msvcr90), and the shipped ogg/vorbis/vorbisfile DLLs.

Host shims (`recomp/src/imports_*.cpp`): KERNEL32 (files mapped to the game
folder via a virtual `C:\MMU`, heaps, TLS, code page 1252, time),
ADVAPI32/WINMM/USER32 bits, the OS-facing SDL 1.2 exports listed in
`SDL_SHIMMED` (window, events with 1.2 keysyms, joysticks, timing, audio
device), and the 18 OPENGL32 calls (texture upload + ortho quads on SDL2).

## Exactness

* x87 runs on IEEE doubles: the CRT sets 53-bit precision (`_PC_53`), so
  results are bit-identical; FCW rounding control is honoured.
* SSE2 (msvcr90 libm/memcpy) and MMX (SDL blitters) are translated; CPUID
  reports an SSE2 CPU like any machine running the game today.
* Flags are computed only where read (CFG liveness); semantics follow the SDM.
* `tools/recomp/difftest.py` compares translated functions against the
  original x86 under Unicorn (FPCW 0x27F): 400 MMU.exe functions and all
  testable libogg/libvorbis/libvorbisfile functions match
  (`--module <dll>` tests one DLL; needs `cmake --build ... --target mmu_recomp_testlib`).
* `tools/recomp/lockstep.py` replays one real call from a memory snapshot in
  both worlds and reports the first diverging function. Capture with
  `RECOMP_LOCKSTEP=<dir>` (currently wired to `Mix_LoadMUS_RW` in
  `imports_debug.cpp`; add other exports to `INTERPOSED` in lift.py). This
  found the `fxch st(i)` operand bug that silenced all Ogg music (1,224
  instructions were swapping ST(0) with itself).

## Status (2026-11-24, updated)

* Boots through both MSVC runtime DLLs, the static constructors and SDL/mixer
  init; plays the boot sequence (MegaPhilX logo, credits, 25th anniversary,
  warning) with WAV sound effects.
* Ogg music plays (fixed: `fxch st(i)` translation).
* **Open:** a few frames of the title screen flash before the logos; the
  original does not show them.
* Switch 2: `mmu_recomp.nro` builds; game files on the SD card (see above).
  Not yet verified on hardware.
* Frame rate follows the display's vsync (e.g. 120 Hz on ProMotion Macs); the
  game's logic is delta-time based, but a 60 Hz cap may be needed for
  exactness.

## Port changes

Deliberate differences from the Windows game, kept small and listed here.
Game-code changes are `PATCHES` / `HOOKS` in `tools/recomp/lift.py` (byte
patches checked against the original bytes; C callbacks in
`recomp/src/patches.cpp` / `imports_sdl.cpp`).

* **Presentation** (`imports_sdl.cpp`): on Switch the window is created at the
  panel size (a game-sized window is upscaled by the system, blurry).
* **Scaling**: the game always draws its 256x224 view
  into a native-size target, placed per frame from the live DISPLAY SIZE value
  (`*[0x6c55d8]`): 1X-4X integer scale (clamped to what fits, centred), FULL
  fits the height with square pixels, WIDE (new value 6: clamp patch at
  0x4fc444, label hook at 0x4fb244) fills the screen. Desktop windows follow
  the option live.
* **Screen shaders** (`postfx.cpp`, `recomp/shaders`, MIT perfect-retroshaders,
  Switch 2 / GLES2 only), the modes and colour filters of OpenHCL-Switch2
  (scanlines, CRT tube, pixel grid, DMG / GBC LCD, SMS TV; GB / SGB palettes),
  with live parameter tuning in the port menu; saved in `mmu_port.ini`
  (`shader=`, `filter=`, `shader_param=`, `master_volume=`).
* **Menus**: A confirms (menu action 0x1d) and B cancels (0x26), added to the
  input manager *[0x6c55fc] after the default bindings, after options.xopts
  is applied, and after the mapping screens rebind a button (they rebind
  every action of the old button, which could drop these) (hook
  `rt_hook_menu_buttons`).
* **Performance**: SDL 1.2's generic per-pixel alpha blitter
  (`BlitNtoNPixelAlpha`, SDL.dll 0x10014030, lifted at 0x58414030) draws
  every sprite pixel (sprites are RGBA, the screen surface is not) and, built
  unoptimised and lifted, was the game's hottest function. A skip hook
  replaces it with a native version of exactly the same arithmetic (unsigned
  wrap-around ALPHA_BLEND, Duff's-device quirks; `RECOMP_VERIFY_BLIT` checked
  225k calls identical); about 9.5x faster. Skip hooks work on DLL functions
  too (relocated VAs).
* **Switch wording**: the title's "PRESS ENTER" (painted into Title-BG2.png)
  becomes "PRESS +", the controller mapping screen's "PUSH [ESC] TO SKIP A
  FUNCTION" (ControlInfo.png) shows [-] [ZR] key caps, and its prompts read
  "BUTTON FOR ..." instead of "PUSH KEY FOR ...". The art is replaced in the
  uploaded frame only where it matches exactly; the patterns are made at build
  time from the game's own images by `extract_font.py`.
* **Switch options menu**: no MAP KEYBOARD INPUTS (no keyboard; its screen
  would wait forever). Its label objects are blanked and its item id zeroed
  before the menu is built, so the cursor skips the row (`rt_menu_tweaks`,
  `rt_hook_options_update`).
* **Cheats** (port menu): see "Port menu".
* **Loading bar** is drawn over the game's own loading splash.
* **Latency**: Switch waits for the GPU after each present (`glFinish`,
  `RECOMP_GLFINISH=0` disables) so at most one frame is queued.
* **Joystick mapping skip**: on the MAP CONTROLLER INPUTS screen (0x4fcf30,
  hooked), - and ZR within 3 frames skip the entry (the game's Escape); a lone
  - or ZR is delivered 3 frames late on that screen only, so it can still be
  mapped.
* **Loading**: 1 MiB stdio buffers for read-only files (the asset load is ~105k
  small reads; 25 s on Switch before); a progress bar (bytes read / size of
  `Assets/*.mmu`) is drawn while the initial load blocks frames.
* **NRO**: title "Mega Man Unlimited", author "MegaPhilX (Switch 2 port by
  mysterypaint)", icon extracted from MMU.exe at build time
  (the 1UP Mega Man head from gameobjects.mmu, `tools/recomp/extract_icon.py`,
  needs Pillow). The Switch software keyboard runs on a helper thread so the
  audio keeps playing while it is open.

## Port menu

`recomp/src/port_menu.cpp` (Dear ImGui, `mmu_imgui`). **L and X together** toggle it
(F1 on a keyboard); after the initial load, "L AND X FOR MENU" appears in the
game's own 8x8 font (`native_text.cpp`, font extracted at build time by
`tools/recomp/extract_font.py`) and fades out in 4 quantized steps. On Switch
the menu fills the screen with the tabs on the left (Shipwright / 2Ship2Harkinian
/ Starship style); on desktop it is a window over the game. Controls: ZL / ZR change
tabs (or touch / D-pad + A on the tab list), D-pad left / right also crosses
between the tab list and the page when nothing further is that way, A
(right) selects and B (bottom) goes back (`ConfigNavSwapGamepadButtons`), the
touch screen drives the pointer, and number fields open the Switch software
keyboard once per activation. While it is open the game gets no input (held
buttons are released on open). Quit Game asks for confirmation.

* **Settings**: every game option (the options object *[0x6c55d8]: +0 display,
  +4 music, +8 sfx, +0xc filtering, +0x10 down+jump slide, +0x14 analog
  dead zone, +0x18 vsync, +0x1c alternate music, +0x20 Omega weapons, +0x24
  short cutscenes), screen shader and colour filter (all of OpenHCL-Switch2's),
  and live shader tuning: the menu hides, a panel with every shader parameter
  stays over the game and hides after 0.5 s without input.
* **Audio**: master (port, ramped gain on the mixed output), music and sound
  effects (the game's values, 0-128, applied with the lifted Mix_VolumeMusic /
  Mix_Volume).
* **Cheats**: infinite health (hook 0x5b0130 cancels damage), lives / screws /
  E-Tanks / W-Tanks (kept from dropping in the live slot), weapon energy (hook
  on the setter 0x420eb0), Moon Jump (0x5af591, after Fall's gravity step, sets the
  jump speed against the gravity global 0x6c5598 while Jump is held, so gravity
  never builds up, off ladders and under ceilings too; release resets vy to 0;
  0x5af037 cuts a released jump to 0), fast knockback (Hurt 0x5acbf0 / HurtSlide
  0x5a8df0 get their timer +0x178 and push +0x60 zeroed so they end on their
  first update; HurtKnockback 0x5a9040 is skipped and switches straight to Hurt),
  always charged Mega Buster shots (skip hook on press-fire 0x61c510 fires a full
  charge through 0x61d280; release hook stops a second shot), jump height multiplier
  (0x5a9ef3 / 0x5af76c, after the jump velocity GameParam 0xccf2a219).
* **Save Editor**: the live slot (save manager *[0x6c55a4] + 0xc8) or any of
  SavedGame.slots: lives, screws, E-Tanks (0-4), small E-Tanks (0-3, carrying
  into E-Tanks), W-Tanks (0-4), Eddie, Beat, Spike Shields, Energy Balancer
  (on/off), YOKU letters, difficulty, story progress, play time, weapons
  acquired and (live) weapon energy and Mega Man's health (player +0x150,
  player from hook 0x5b02f0); plus SavedGame's global progress. Layouts come
  from the protobuf offset tables in MMU.exe (SavedGame 0x697230,
  SavedGameSlot 0x6972e0); edits set the field's has-bit so the game
  serializes them on its next save.

Settings live in `mmu_port.ini` (Switch: next to the logs; desktop: the game
folder): `shader=`, `filter=`, `master_volume=`, `shader_param=<mode> <name> <v>`.

Missing game files: `missing_game.cpp` shows which file is missing, where the
files go and MegaPhilX's official page; X exits.

Audio during long work between frames (DISPLAY SIZE re-uploads every texture)
is kept fed from the GL shims (`pump_audio_soon`).

Debug: `RECOMP_MENU=<tab>` opens the menu at start (9: the shader panel),
`RECOMP_MENU_SLOT=<n>` selects a saved slot, `RECOMP_SCREENSHOT` also captures
the missing-files screen.

## Distribution

`make dist` builds `build-dist/mmu_recomp-switch2.zip`: `switch/mmu_recomp/
mmu_recomp.nro` and `README.txt` (from `dist/README.txt`: changelog, install
for every platform, controls, and the official page
https://megaphilx.com/index.php/home/games/mega-man-unlimited/ for the game
files). No game files are included, but the binaries contain code and the
small images (font, icon, prompts) derived from the game at build time.

Release builds (desktop binaries need no installed SDL: SDL 2.32 is linked
statically; Windows ships SDL2.dll):

* **macOS** (universal arm64 + x86_64, macOS 11+): build SDL2 from source
  with `-DSDL_SHARED=OFF -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0`, then configure this project with the
  same architectures and `-DSDL2_DIR=<sdl>/lib/cmake/SDL2`; build target
  `mmu_recomp`.
* **Linux** (x86_64, glibc 2.34+): `tools/release/build_linux.sh <game dir>
  <SDL2 source dir>` cross-compiles in Docker (`tools/release/linux.Dockerfile`,
  works on Apple Silicon through colima), with libstdc++/libgcc static.
* **Windows** (x86_64): MinGW-w64 cross build,
  `-DCMAKE_TOOLCHAIN_FILE=cmake/MinGW64Toolchain.cmake
  -DSDL2_DIR=<SDL2-devel-mingw>/x86_64-w64-mingw32/lib/cmake/SDL2`; fully
  static except SDL2.dll. COFF linkers mishandle weak symbols, so the import
  stubs are plain functions there and the runtime's definitions win
  (`--allow-multiple-definition`, runtime objects linked first).
* **Switch 2**: `make dist`.

CMake treats only the devkitPro toolchain ("Generic" system) as Switch
(`MMU_SWITCH`); other cross builds are desktop builds.
