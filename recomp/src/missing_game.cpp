// Shown instead of the game when its files are missing: where to put them, where
// to get them (MegaPhilX's official page), and X (Nintendo top) to exit.
#include "port.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {
constexpr int kW = 256, kH = 224;
}

int rt_missing_game_screen(const char* dir, const char* what) {
    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    SDL_DisplayMode dm{};
#if defined(__SWITCH__)
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0 || dm.w <= 0) { dm.w = 1280; dm.h = 720; }
#else
    dm.w = kW * 3;
    dm.h = kH * 3;
#endif
    SDL_Window* win = SDL_CreateWindow("Mega Man Unlimited", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dm.w,
                                       dm.h, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer* r = win ? SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
                                                            SDL_RENDERER_TARGETTEXTURE)
                          : nullptr;
    if (!r) return 1;
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
        if (SDL_IsGameController(i)) SDL_GameControllerOpen(i);
    SDL_Texture* screen = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, kW, kH);
    SDL_SetTextureScaleMode(screen, SDL_ScaleModeNearest);

#if defined(__SWITCH__)
    const std::vector<std::string> lines = {
        "GAME FILES NOT FOUND", "", std::string("MISSING: ") + what, "",
        "COPY YOUR MEGA MAN UNLIMITED", "FOLDER (MMU.EXE, THE DLLS AND", "ASSETS) TO:", "",
        "SD:/SWITCH/MMU_RECOMP/GAME/", "", "GET THE GAME FROM MEGAPHILX'S", "OFFICIAL PAGE:", "MEGAPHILX.COM", "",
        "PRESS X TO EXIT"};
#else
    const std::vector<std::string> lines = {
        "GAME FILES NOT FOUND", "", std::string("MISSING: ") + what, "",
        "COPY YOUR MEGA MAN UNLIMITED", "FOLDER (MMU.EXE, THE DLLS AND", "ASSETS) INTO A FOLDER NAMED", "GAME NEXT TO THIS PROGRAM.", "",
        "GET THE GAME FROM MEGAPHILX'S", "OFFICIAL PAGE:", "MEGAPHILX.COM", "", "PRESS X OR ESC TO EXIT"};
#endif
    std::fprintf(stderr, "recomp: game files not found in %s (missing %s)\n", dir, what);
    for (bool done = false; !done;) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) done = true;
            if (e.type == SDL_CONTROLLERBUTTONDOWN && e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) done = true;  // Switch X
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) done = true;
        }
        SDL_SetRenderTarget(r, screen);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);
        int y = (kH - (int)lines.size() * 10) / 2;
        for (const std::string& l : lines) {
            rt_text_draw(r, l.c_str(), (kW - rt_text_width(l.c_str())) / 2, y, 1, 255);
            y += 10;
        }
        SDL_SetRenderTarget(r, nullptr);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);
        int ow = 0, oh = 0;
        SDL_GetRendererOutputSize(r, &ow, &oh);
        int n = 1;
        while (kW * (n + 1) <= ow && kH * (n + 1) <= oh) ++n;
        const SDL_Rect dst{(ow - kW * n) / 2, (oh - kH * n) / 2, kW * n, kH * n};
        SDL_RenderCopy(r, screen, nullptr, &dst);
        if (const char* shot = std::getenv("RECOMP_SCREENSHOT")) {  // debug: capture and exit
            SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, ow, oh, 32, SDL_PIXELFORMAT_ARGB8888);
            if (s && SDL_RenderReadPixels(r, nullptr, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch) == 0)
                SDL_SaveBMP(s, shot);
            if (s) SDL_FreeSurface(s);
            done = true;
        }
        SDL_RenderPresent(r);
    }
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(win);
    return 1;
}
