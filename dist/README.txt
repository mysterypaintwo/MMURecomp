MEGA MAN UNLIMITED - native port (Switch 2, macOS, Linux, Windows)
==================================================================

Mega Man Unlimited is a free fan game by MegaPhilX. This port is a static
recompilation of the original Windows game: the game's own code, translated
to run natively, so it plays like the original, plus a few port features
(screen sizes, screen shaders, and a port menu with cheats and a save editor).

This download does NOT include the game. You need your own copy of
Mega Man Unlimited, from MegaPhilX's official page:

    https://megaphilx.com/index.php/home/games/mega-man-unlimited/

The port is made for version 1.3.1 of the game (shown in the bottom right
corner of the title screen).


CHANGELOG
---------

v1.0
    Initial release!


1. GET THE GAME FILES
---------------------

1. Download Mega Man Unlimited (the Windows version) from the page above.

2. Unzip it. You get a folder (for example "MegaManUnlimited") containing:

       MMU.exe
       SDL.dll, SDL_gfx.dll, SDL_image.dll, SDL_mixer.dll
       libogg-0.dll, libvorbis-0.dll, libvorbisfile-3.dll
       ogg.dll, vorbis.dll, vorbisfile.dll
       msvcr90.dll, msvcp90.dll
       Assets/   (cutscenedefs.mmu, cutscenes.mmu, gameobjects.mmu, guns.mmu,
                  levelart.mmu, levels.mmu, menus.mmu, misc.mmu)

   The port needs all of these: copy the whole folder's contents, do not pick
   files out of it. You do not need to install or run the Windows game.

   Already played the Windows version? Your savedgame.mmu (saves) and
   options.xopts (settings) from that folder work as they are.

3. Put the files where your platform expects them (next section). The game
   folder must be writable: settings and saves are written into it.


2. INSTALL
----------

Nintendo Switch 2 (homebrew)

    sd:/switch/mmu_recomp/mmu_recomp.nro       <- from this download
    sd:/switch/mmu_recomp/game/MMU.exe         <- the game folder's contents
    sd:/switch/mmu_recomp/game/SDL.dll            (and all the other .dll files)
    sd:/switch/mmu_recomp/game/Assets/            (all the .mmu files)

    Copy what is INSIDE your MegaManUnlimited folder into
    sd:/switch/mmu_recomp/game/, so that MMU.exe ends up directly in "game"
    (not in game/MegaManUnlimited/MMU.exe).

    Launch it from the Homebrew Menu with full application memory: hold R
    while starting a game ("title override"), not from the album applet. The
    game needs about 1.4 GB of memory.

macOS (11 Big Sur or later; Apple Silicon and Intel)

    MegaManUnlimited/
        mmu_recomp            <- from this download
        game/                 <- the game folder's contents
            MMU.exe
            (the .dll files)
            Assets/

    The program is not signed by Apple. The first time, right-click (or
    Control-click) mmu_recomp, choose Open, then Open again. If macOS still
    refuses ("damaged" or "cannot be opened"), run this once in Terminal:

        xattr -dr com.apple.quarantine /path/to/MegaManUnlimited

Linux (x86-64, glibc 2.34 or later: Ubuntu 22.04, Debian 12, Fedora 35 or newer)

    MegaManUnlimited/
        mmu_recomp            <- from this download
        game/                 <- the game folder's contents
            MMU.exe
            ...

    Make it executable if needed (chmod +x mmu_recomp) and run it. X11 and
    Wayland both work; sound goes through PipeWire, PulseAudio or ALSA.

Windows (64-bit, Windows 10 or later)

    MegaManUnlimited/
        mmu_recomp.exe        <- from this download
        SDL2.dll              <- from this download
        game/                 <- the game folder's contents
            MMU.exe
            ...

On computers you can also put the game files right next to the program
(MMU.exe beside mmu_recomp), or give the game folder as the first argument:

    mmu_recomp /path/to/MegaManUnlimited

If something is missing, the port shows which file it could not find
(on Switch, press X to exit).


3. CONTROLS
-----------

Nintendo Switch 2 (defaults)

    B ............ Jump              A ............ Confirm (menus)
    Y ............ Shoot             B ............ Back (menus)
    ZR ........... Slide             + ............ Start
    L / R ........ Next / previous weapon
    - ............ Select            D-pad / left stick ... Move

    Remap them in OPTIONS > MAP CONTROLLER INPUTS; there, press - and ZR
    together to skip an entry.

Computers

    Keyboard and controller work as in the original game; remap them in
    OPTIONS (MAP KEYBOARD INPUTS / MAP CONTROLLER INPUTS).


4. PORT MENU
------------

Open or close it with L and X together (Switch) or F1 (computers).

    Settings ..... all game options, screen size (1X-4X, FULL, WIDE), screen
                   shaders and colour filters (with live tuning)
    Audio ........ master, music and sound effects volume
    Cheats ....... infinite health / lives / weapon energy / screws / tanks,
                   Moon Jump, fast knockback, always charged Mega Buster
                   shots, jump height
    Save Editor .. items, tanks, weapons and progress of every save slot
    Quit Game

On Switch: ZL / ZR change tabs (or tap a tab, or pick one with the D-pad and
A); the D-pad moves around, left / right also between the tabs and the page;
A selects, B goes back, and the touch screen works too. On computers, use the
mouse or a controller.

Port settings are saved in mmu_port.ini (Switch: next to mmu_recomp.nro;
computers: in the game folder).


5. TROUBLESHOOTING
------------------

- "Game files not found": check the folder layout above. MMU.exe must be
  directly inside the game folder.
- Switch: the game closes or stops while loading: start it with full
  application memory (hold R while starting a game), not from the album.
- Switch: the port writes sd:/switch/mmu_recomp/log.txt and boot.txt (small,
  rewritten on every launch). Include them when reporting a problem.


CREDITS
-------

Mega Man Unlimited: MegaPhilX and team.
    https://megaphilx.com/index.php/home/games/mega-man-unlimited/
Port: mysterypaint.

Mega Man is a trademark of Capcom. Mega Man Unlimited is a free fan game and
is not affiliated with or endorsed by Capcom; neither is this free, unofficial
port. Screen shaders: perfect-retroshaders by sinedied (MIT licence). Uses
SDL 2 (zlib licence) and Dear ImGui (MIT licence).
