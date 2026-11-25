// Host entry for the statically recompiled MMU.exe.
//   mmu_recomp [game_dir]   (folder with MMU.exe, its DLLs and Assets/)
// Desktop default: game/ next to the program, the program's own folder, ./game.
//
// Switch 2: the game folder defaults to sdmc:/switch/mmu_recomp/game/ (copy
// the whole MegaManUnlimited folder there; it must be writable for
// options.xopts and saves). A romfs:/game/ embedded in the .nro is used when
// that folder is absent (read-only: settings/saves will not persist).
#include "recomp/cpu.h"
#include "port.h"

#include <cstdio>
#include <string>
#include <sys/stat.h>

#if defined(__SWITCH__) || defined(__SWITCH2__)
extern "C" unsigned romfsMountSelf(const char* name);
#endif

static bool has_exe(const std::string& dir) {
    struct stat st;
    return stat((dir + "/MMU.exe").c_str(), &st) == 0;
}

// First missing game file, or nullptr when everything the port loads is there.
static const char* missing_file(const std::string& dir) {
    static const char* kFiles[] = {
        "MMU.exe", "msvcr90.dll", "msvcp90.dll", "SDL.dll", "SDL_gfx.dll", "SDL_image.dll", "SDL_mixer.dll",
        "libogg-0.dll", "libvorbis-0.dll", "libvorbisfile-3.dll", "ogg.dll", "vorbis.dll", "vorbisfile.dll",
        "Assets/misc.mmu", "Assets/menus.mmu", "Assets/gameobjects.mmu", "Assets/guns.mmu", "Assets/levels.mmu",
        "Assets/levelart.mmu", "Assets/cutscenes.mmu", "Assets/cutscenedefs.mmu"};
    for (const char* f : kFiles) {
        struct stat st;
        if (stat((dir + "/" + f).c_str(), &st) != 0) return f;
    }
    return nullptr;
}

int main(int argc, char** argv) {
    std::string dir;
#if defined(__SWITCH__) || defined(__SWITCH2__)
    rt_log_init("sdmc:/switch/mmu_recomp");  // log.txt + boot.txt (or nxlink)
    dir = "sdmc:/switch/mmu_recomp/game";
    if (!has_exe(dir)) {
        romfsMountSelf("romfs");
        if (has_exe("romfs:/game")) dir = "romfs:/game";
    }
    (void)argc;
    (void)argv;
#else
    rt_log_init(nullptr);  // crash signal handlers; RECOMP_STATUS=s for status lines
    if (argc > 1) {
        dir = argv[1];
    } else {
        // Release layout: a game/ folder next to the program (or the game files
        // right beside it); then ./game.
        std::string base;
        if (char* b = SDL_GetBasePath()) { base = b; SDL_free(b); }
        for (const std::string& d : {base + "game", base.empty() ? std::string(".") : base.substr(0, base.size() - 1),
                                     std::string("game")})
            if (has_exe(d)) { dir = d; break; }
        if (dir.empty()) dir = base + "game";
    }
#endif
    if (const char* missing = missing_file(dir)) {
        rt_crumb("ERROR: %s not found in %s", missing, dir.c_str());
        return rt_missing_game_screen(dir.c_str(), missing);
    }
    rt_crumb("game folder %s; reserving %u MiB of guest memory", dir.c_str(), RT_GUEST_SIZE >> 20);
    if (!rt_init(dir.c_str())) {
        rt_crumb("ERROR: failed to load the game from %s (guest memory: %u MiB)", dir.c_str(), RT_GUEST_SIZE >> 20);
        rt_log_memory("load failed");
        return 1;
    }
    rt_log_memory("game loaded; running");
    const int rc = rt_run_entry();
    rt_crumb("game returned %d", rc);
    return rc;
}
