// Text in the game's own 8x8 sprite font (Font.png, extracted at build time by
// tools/recomp/extract_font.py), for the port's messages: the "L AND X FOR MENU"
// hint and the missing-game-files screen. Characters outside ASCII
// 32-127 are skipped; lowercase is drawn as uppercase (the game's menus are).
#include "port.h"

#include "mmu_font.h"

#include <cctype>
#include <cstring>

namespace {
SDL_Texture* g_font = nullptr;
SDL_Renderer* g_font_renderer = nullptr;
}  // namespace

bool rt_text_available() { return MMU_FONT_OK != 0; }

int rt_text_width(const char* s) { return (int)std::strlen(s) * 8; }

void rt_text_draw(SDL_Renderer* r, const char* s, int x, int y, int scale, uint8_t alpha) {
#if MMU_FONT_OK
    if (g_font_renderer != r) {
        if (g_font) SDL_DestroyTexture(g_font);
        g_font = SDL_CreateTexture(r, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, MMU_FONT_W, MMU_FONT_H);
        if (g_font) {
            SDL_UpdateTexture(g_font, nullptr, kMmuFont, MMU_FONT_W * 4);
            SDL_SetTextureBlendMode(g_font, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(g_font, SDL_ScaleModeNearest);
        }
        g_font_renderer = r;
    }
    if (!g_font) return;
    SDL_SetTextureAlphaMod(g_font, alpha);
    for (int i = 0; s[i]; ++i) {
        const int ch = std::toupper((unsigned char)s[i]);
        if (ch <= 32 || ch > 127) continue;
        const int g = ch - 32;
        const SDL_Rect src{(g % 16) * 8, (g / 16) * 8, 8, 8};
        const SDL_Rect dst{x + i * 8 * scale, y, 8 * scale, 8 * scale};
        SDL_RenderCopy(r, g_font, &src, &dst);
    }
#else
    (void)r; (void)s; (void)x; (void)y; (void)scale; (void)alpha;
#endif
}

// Title screen: the "PRESS ENTER" painted into Title-BG2 becomes "PRESS +" on
// Switch 2 (no Enter key). Called on each uploaded game frame (ARGB8888 rows);
// replaces the text only where the frame matches it exactly (so never during
// fades or anything drawn over it). "PRESS " keeps the original pixels; the
// "+" is drawn to match: 2 px strokes, equal arms, on the baseline.
bool rt_patch_title_prompt(uint8_t* pixels, int pitch, int w, int h) {
#if MMU_PROMPT_OK
    constexpr int X = MMU_PROMPT_X, Y = MMU_PROMPT_Y, W = MMU_PROMPT_W, H = MMU_PROMPT_H;
    if (w < X + W || h < Y + H) return false;
    auto px = [&](int x, int y) { return (uint32_t*)(pixels + (size_t)(Y + y) * (size_t)pitch) + X + x; };
    for (int v = 0; v < MMU_PROMPT_VARIANTS; ++v) {
        bool match = true;
        for (int y = 0; y < H && match; ++y)
            for (int x = 0; x < W; ++x)
                if ((*px(x, y) ^ kMmuPromptText[v][y * W + x]) & 0xffffffu) { match = false; break; }
        if (!match) continue;
        static const char* const kPlus[H] = {"......", "..##..", "..##..", "######", "######", "..##..", "..##.."};
        constexpr int kKeep = 48, kPlusW = 6, kShift = (W - (kKeep + kPlusW)) / 2;  // "PRESS " + "+", centred
        uint32_t text[H][W];
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x) text[y][x] = kMmuPromptText[v][y * W + x];
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) *px(x, y) = kMmuPromptPlain[v][y * W + x];
            for (int x = 0; x < kKeep; ++x)
                if (text[y][x] != kMmuPromptPlain[v][y * W + x]) *px(x + kShift, y) = text[y][x];
            for (int x = 0; x < kPlusW; ++x)
                if (kPlus[y][x] == '#') *px(kKeep + kShift + x, y) = 0xffffffffu;  // white, like the text
        }
        return true;
    }
#else
    (void)pixels; (void)pitch; (void)w; (void)h;
#endif
    return false;
}

// Controller mapping screen (Switch 2): "PUSH [ESC] TO SKIP A FUNCTION"
// (Menus/Main/ControlInfo.png) becomes "PUSH [-+ZR] ..." (the port's skip
// chord). Found in the uploaded frame by its exact ink pattern (cached), then
// redrawn from the pattern made at build time by extract_font.py.
bool rt_patch_control_info(uint8_t* pixels, int pitch, int w, int h) {
#if MMU_SKIPINFO_OK
    constexpr int W = MMU_SKIPINFO_W, H = MMU_SKIPINFO_H;
    auto at = [&](int x, int y) { return (uint32_t*)(pixels + (size_t)y * (size_t)pitch) + x; };
    auto matches = [&](int x0, int y0) {
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x)
                if (((*at(x0 + x, y0 + y) & 0xffffffu) == MMU_SKIPINFO_INK) != (kMmuSkipOld[y][x] == '#')) return false;
        return true;
    };
    static int cx = -1, cy = -1;
    if (cx < 0 || cx + W > w || cy + H > h || !matches(cx, cy)) {
        cx = cy = -1;
        for (int y = 0; y + H <= h && cx < 0; ++y)
            for (int x = 0; x + W <= w; ++x)
                if ((*at(x, y + 1) & 0xffffffu) == MMU_SKIPINFO_INK && matches(x, y)) { cx = x; cy = y; break; }
        if (cx < 0) return false;
    }
    const uint32_t bg = *at(cx + 19, cy);  // a background pixel next to the text
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            uint32_t* p = at(cx + x, cy + y);
            if (kMmuSkipNew[y][x] == '#') *p = 0xff000000u | MMU_SKIPINFO_INK;
            else if (kMmuSkipOld[y][x] == '#') *p = bg;
        }
    return true;
#else
    (void)pixels; (void)pitch; (void)w; (void)h;
    return false;
#endif
}
