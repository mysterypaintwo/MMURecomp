// Port changes to the game's own behaviour (HOOKS / PATCHES in
// tools/recomp/lift.py). Each hook runs with synced registers right before the
// instruction it is attached to. Keep this list short and documented in
// docs/recompilation.md ("Port changes").
#include "recomp/runtime.h"

#include <cstdlib>
#include <chrono>
#include <cstring>
#include <initializer_list>
#include <cstdio>
#include <vector>

namespace {
constexpr uint32_t kFullLabel = 0x65f0e8u;  // "DISPLAY SIZE: < FULL>" (24-byte slot)
void put_label(const char* text) {
    char slot[24] = {};
    std::strncpy(slot, text, sizeof slot - 1);
    std::memcpy(rt_mem + kFullLabel, slot, sizeof slot);
}
}  // namespace

// 0x4fb244: `cmp [ebp-0x1bc], 4; ja` picks the DISPLAY SIZE label from
// value-1 (5 labels). Value 6 (WIDE, allowed by the 0x4fc444 patch) reuses
// the FULL case with the slot's text swapped.
extern "C" void rt_hook_display_label(Cpu* c) {
    const uint32_t slot = c->ebp - 0x1bcu;
    const uint32_t index = rt_rd32(slot);
    if (index == 5) {
        put_label("DISPLAY SIZE: < WIDE>");
        rt_wr32(slot, 4);
    } else if (index == 4) {
        put_label("DISPLAY SIZE: < FULL>");
    }
}

extern "C" uint32_t rt_guest_call(uint32_t va, unsigned nargs, const uint32_t* args);

// ---- port menu cheats and live data (recomp/src/port_menu.cpp)
#include "port.h"

// 0x5b02f0 entry: MegaMan's message handler (thiscall), runs every frame with
// the input message: remember the player object for the save editor.
extern "C" void rt_hook_player(Cpu* c) {
    g_player = c->ecx;
    g_player_frame = rt_frame_count();
}

// 0x5b0130 entry: MegaMan::applyDamage(this, damage): health -= damage +
// this[0x3e4], death at 0. Infinite health cancels the whole amount, so the HUD
// and the hurt animation still run.
extern "C" void rt_hook_damage(Cpu* c) {
    if (!g_cheats.infinite_health) return;
    const uint32_t extra = rt_rd32(c->ecx + 0x3e4u);
    rt_wr32(c->esp + 4u, (uint32_t)(-(int32_t)extra));
}

// 0x420eb0 entry: SaveManager::setWeaponEnergy(this, index, float). Infinite
// weapon energy stores the maximum instead.
extern "C" void rt_hook_set_energy(Cpu* c) {
    if (!g_cheats.infinite_ammo) return;
    float max;
    std::memcpy(&max, rt_mem + mmu::kMaxWeaponEnergy, 4);
    if (max > 0 && max < 1000) std::memcpy(rt_mem + c->esp + 8u, &max, 4);
}

// Right after MegaMan's GameParam lookups (value in ST(0)):
// 0x5a9ef3 / 0x5af76c: jump velocity (0xccf2a219) x the jump height setting.
static float g_jump_speed = 325.0f;  // |jump velocity| last used (Moon Jump keeps it)
extern "C" void rt_hook_jump_velocity(Cpu* c) {
    double& v = c->st[c->top & 7u];
    if (g_cheats.jump_multiplier != 1.0f) v *= g_cheats.jump_multiplier;
    if (v != 0) g_jump_speed = (float)(v < 0 ? -v : v);
}
// 0x5af037: the upward speed a released Jump is cut to (0xab528d4a, -50; the
// game then keeps sign(vy) * min(|vy|, 50)). Moon Jump cuts to 0: you drop
// as soon as Jump is released.
extern "C" void rt_hook_jump_cut(Cpu* c) {
    if (g_cheats.moon_jump) c->st[c->top & 7u] = 0.0;
}
// 0x5af591, every frame of MegaMan's Fall state ([ebp-0xa8] = MegaMan), after
// gravity (vy += gravity * dt, 0x5af4c8), the fall-speed clamp and the bounce,
// right before the move uses vy (float +0x64). Moon Jump: while Jump (action
// 0x1e, held query 0x441db0) is held, vy is set to the jump speed against
// gravity, so gravity never accumulates. That holds in any Fall: off a ladder,
// and after a ceiling (the move zeroes vy, the next frame rises again). On
// release vy is reset to 0 and gravity starts from there. "Up" is against the
// sign of the gravity global (0x6c5598; negated in gravity-flip zones).
extern "C" void rt_hook_moon_jump(Cpu* c) {
    static bool floating = false;
    static unsigned last_frame = 0;
    const unsigned frame = rt_frame_count();
    if (frame - last_frame > 1) floating = false;  // left Fall since (landed, ladder, hurt...)
    last_frame = frame;
    if (!g_cheats.moon_jump) { floating = false; return; }
    const uint32_t p = rt_rd32(c->ebp - 0xa8u);
    if (!p) return;
    float gravity, vy;
    std::memcpy(&gravity, rt_mem + 0x6c5598u, 4);
    std::memcpy(&vy, rt_mem + p + 0x64u, 4);
    const float up = gravity < 0 ? 1.0f : -1.0f;
    const Cpu saved = *c;
    c->eax = 0x1e;
    rt_guest_call(0x441db0u, 0, nullptr);
    const bool held = (c->eax & 0xffu) != 0;
    *c = saved;
    if (held) {
        vy = up * g_jump_speed;
        floating = true;
    } else if (floating) {
        vy = 0;  // drop from here, gravity starts fresh
        floating = false;
    } else {
        return;
    }
    std::memcpy(rt_mem + p + 0x64u, &vy, 4);
}

// ---- A confirms, B cancels in menus (in addition to the game's own buttons)
// Bindings are FUN_004412d0(input, context, action, code, 0, 0, 0x143) on the
// input manager *[0x6c55fc]. Menu context 4: confirm 0x1d (Enter/Start), back
// 0x26 (Escape). Virtual pad (imports_sdl.cpp): A = button 3 (code 0x14e),
// B = button 1 (0x14c). Hooked after the factory defaults (0x4fc22c, 0x5dd1bc),
// after options.xopts' joystick controls are applied (0x440f93), and after the
// mapping screens rebind a button (0x4fd106 controller, 0x4fd43f keyboard),
// which rebinds every action of the old button and could drop these.
extern "C" uint32_t rt_guest_call(uint32_t va, unsigned nargs, const uint32_t* args);
extern "C" void rt_hook_menu_buttons(Cpu* c) {
    const uint32_t input = rt_rd32(0x6c55fcu);
    static const bool trace = std::getenv("RECOMP_TRACE_STATES") != nullptr;
    if (trace) std::fprintf(stderr, "menu buttons: A/B bound (input %#x)\n", input);
    if (!input) return;
    const Cpu saved = *c;
    static const uint32_t kBinds[][3] = {{4, 0x1d, 0x14e}, {4, 0x26, 0x14c}};
    for (const auto& b : kBinds) {
        const uint32_t args[6] = {b[0], b[1], b[2], 0, 0, 0x143};
        c->ecx = input;
        rt_guest_call(0x4412d0u, 6, args);
    }
    *c = saved;  // the hooked code continues with its own registers
}

// Fast knockback. Hurt (0x5acbf0) and HurtSlide (0x5a8df0) count a timer at
// +0x178 down and leave the state when it reaches 0 (to Stand/Fall/ladder/jump,
// or back to Slide); setting it to 0 on their first update ends the hurt state
// on the next frame, before Mega Man is moved. HurtKnockback (0x5a9040, the push
// that lasts until Mega Man hits something, then Hurt) is skipped and goes to
// Hurt directly. Damage and the invincibility frames stay. fastcall, ecx = MegaMan.
extern "C" void rt_hook_fast_knockback(Cpu* c) {
    if (!g_cheats.fast_knockback || !c->ecx) return;
    rt_wr32(c->ecx + 0x60u, 0);    // no horizontal push
    rt_wr32(c->ecx + 0x178u, 0);   // hurt timer: done
}
extern "C" int rt_hook_fast_knockback_push(Cpu* c) {
    if (!g_cheats.fast_knockback || !c->ecx) return 0;
    const Cpu saved = *c;
    const uint32_t args[2] = {0x67daa0u /* "Hurt" */, saved.ecx + 4u};
    rt_guest_call(0x4405c0u, 2, args);  // cdecl: switch state by name
    *c = saved;
    return 1;
}

// Always charged Mega Buster shots. The weapon object (this): +4 current weapon,
// +0x170 charge level (1 half, 2 full), +0x177 charging. 0x61d280 (fastcall)
// fires the charged shot on release from that level; 0x61c510(this, flag,
// weapon) fires on press (weapon 0xd = current).
// Press (0x61c510 entry, skip hook): for the Mega Buster (weapon 0, not as
// Zero) fire a full charged shot through 0x61d280 and return 1 ("fired": the
// shooting pose follows). Release (0x61d280 entry): with the cheat on, a charge
// built by holding the button does not fire a second shot.
static bool g_buster_firing = false;
static bool buster_selected(const Cpu* c, uint32_t weapon) {
    if (!g_cheats.always_charged || !c->ecx || weapon != 0) return false;
    const uint32_t sm = rt_rd32(mmu::kSaveMgrPtr);
    return !(sm && rt_mem[sm + mmu::kCurrentSlot + 89u]);  // playing_as_zero: weapon 0 is not the buster
}
extern "C" int rt_hook_buster_press(Cpu* c) {
    uint32_t weapon = rt_rd32(c->esp + 8u);
    if (weapon == 0xd) weapon = rt_rd32(c->ecx + 4u);
    if (!buster_selected(c, weapon)) return 0;
    const uint32_t w = c->ecx;
    const Cpu saved = *c;
    rt_mem[w + 0x177u] = 1;
    rt_wr32(w + 0x170u, 2);
    g_buster_firing = true;
    rt_guest_call(0x61d280u, 0, nullptr);  // ecx = the weapon object
    g_buster_firing = false;
    *c = saved;
    c->eax = 1;
    return 1;
}
extern "C" void rt_hook_buster_release(Cpu* c) {
    if (g_buster_firing || !buster_selected(c, rt_rd32(c->ecx + 4u))) return;
    rt_mem[c->ecx + 0x177u] = 0;  // the release function then returns 0
}

// ---- Performance: SDL 1.2's BlitNtoNPixelAlpha (SDL.dll 0x10014030, at
// 0x58414030 here), the generic per-pixel alpha blitter. The game's sprites are
// RGBA and its screen surface is not, so every sprite pixel goes through it;
// the DLL was built unoptimised, and lifted it was the hottest function in the
// game (sprite-heavy scenes dipped on Switch 2). Native version of exactly the
// same computation (cdecl, one SDL_BlitInfo*): the same 32-bit unsigned
// ALPHA_BLEND wrap-around, x86 shift-count masking, Duff's-device quirk (a
// zero width still does 4 pixels) and 2/3/4-byte pixel paths.
namespace {
inline uint32_t shl32(uint32_t v, unsigned n) { return v << (n & 31u); }
inline uint32_t shr32(uint32_t v, unsigned n) { return v >> (n & 31u); }
struct GuestFmt {  // SDL 1.2 SDL_PixelFormat
    unsigned bpp, loss[4], shift[4];
    uint32_t mask[4];
    explicit GuestFmt(uint32_t f) {
        bpp = rt_mem[f + 5u];
        for (int i = 0; i < 4; ++i) {
            loss[i] = rt_mem[f + 6u + (unsigned)i];
            shift[i] = rt_mem[f + 0xau + (unsigned)i];
            mask[i] = rt_rd32(f + 0x10u + 4u * (unsigned)i);
        }
    }
    uint32_t read(const uint8_t* p) const {
        switch (bpp) {
        case 2: return (uint32_t)p[0] | (uint32_t)p[1] << 8;
        case 3: return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16;
        case 4: { uint32_t v; std::memcpy(&v, p, 4); return v; }
        default: return 0;
        }
    }
    void split(uint32_t px, uint32_t out[4]) const {
        for (int i = 0; i < 4; ++i) out[i] = shl32(shr32(px & mask[i], shift[i]), loss[i]);
    }
    void write(uint8_t* p, const uint32_t v[4]) const {
        if (bpp == 3) {  // little-endian byte stores, no alpha
            for (int i = 0; i < 3; ++i) p[(int)shift[i] / 8] = (uint8_t)v[i];
            return;
        }
        uint32_t px = 0;
        for (int i = 0; i < 4; ++i) px |= shl32(shr32(v[i], loss[i]), shift[i]);
        if (bpp == 2) { p[0] = (uint8_t)px; p[1] = (uint8_t)(px >> 8); }
        else if (bpp == 4) std::memcpy(p, &px, 4);
    }
};
}  // namespace

static int blit_pixel_alpha_native(Cpu* c);
// RECOMP_ORIGINAL_BLIT: run the lifted original. RECOMP_VERIFY_BLIT: run both
// on the same pixels and report any difference (development check).
extern "C" int rt_hook_blit_pixel_alpha(Cpu* c) {
    static const bool off = std::getenv("RECOMP_ORIGINAL_BLIT") != nullptr;
    static const bool verify = std::getenv("RECOMP_VERIFY_BLIT") != nullptr;
    static bool in_original = false;
    if (off || in_original) return 0;
    if (!verify) return blit_pixel_alpha_native(c);
    const uint32_t info = rt_rd32(c->esp + 4u);
    const uint32_t dst = rt_rd32(info + 0x10u);
    const uint32_t dfmt = rt_rd32(info + 0x2cu);
    const int w = (int)rt_rd32(info + 0x14u), h = (int)rt_rd32(info + 0x18u);
    const size_t span = (size_t)(h > 0 ? h : 0) * ((size_t)(w > 0 ? w : 4) * rt_mem[dfmt + 5u] + rt_rd32(info + 0x1cu));
    std::vector<uint8_t> before(rt_mem + dst, rt_mem + dst + span);
    if (!blit_pixel_alpha_native(c)) return 0;
    std::vector<uint8_t> native(rt_mem + dst, rt_mem + dst + span);
    std::memcpy(rt_mem + dst, before.data(), span);
    const Cpu saved = *c;
    in_original = true;
    rt_guest_call(0x58414030u, 1, &info);
    in_original = false;
    *c = saved;
    static unsigned calls = 0, bad = 0;
    ++calls;
    if (std::memcmp(native.data(), rt_mem + dst, span) != 0) ++bad;
    if ((calls & 4095u) == 0 || (bad && bad < 4)) std::fprintf(stderr, "verify blit: %u calls, %u differ\n", calls, bad);
    return 1;  // the original's pixels are already in place
}
static int blit_pixel_alpha_native(Cpu* c) {
    const uint32_t info = rt_rd32(c->esp + 4u);
    const int width = (int)rt_rd32(info + 0x14u);
    int height = (int)rt_rd32(info + 0x18u);
    uint32_t src = rt_rd32(info + 0x00u), dst = rt_rd32(info + 0x10u);
    const int32_t srcskip = (int32_t)rt_rd32(info + 0x0cu), dstskip = (int32_t)rt_rd32(info + 0x1cu);
    const GuestFmt sf(rt_rd32(info + 0x24u)), df(rt_rd32(info + 0x2cu));
    const int count = width > 0 ? width : (width == 0 ? 4 : 0);  // DUFFS_LOOP4
    if (count == 0 && width < 0) {
        // (n + 3) / 4 <= 0 with n & 3 picking a case: do the case's pixels once.
        // Never seen in practice; leave negative widths to the original code.
        return 0;
    }
    const bool fast = sf.bpp == 4 && df.bpp == 4;
    while (height-- > 0) {
        uint8_t* s = rt_mem + src;
        uint8_t* d = rt_mem + dst;
        if (fast) {
            for (int x = 0; x < count; ++x, s += 4, d += 4) {
                uint32_t sp, dp;
                std::memcpy(&sp, s, 4);
                const uint32_t sA = shl32(shr32(sp & sf.mask[3], sf.shift[3]), sf.loss[3]);
                if (!sA) continue;
                std::memcpy(&dp, d, 4);
                uint32_t sv[4], dv[4];
                sf.split(sp, sv);
                df.split(dp, dv);
                for (int i = 0; i < 3; ++i) dv[i] = ((sv[i] - dv[i]) * sA + 255u) / 256u + dv[i];
                uint32_t out = 0;
                for (int i = 0; i < 4; ++i) out |= shl32(shr32(dv[i], df.loss[i]), df.shift[i]);
                std::memcpy(d, &out, 4);
            }
        } else {
            for (int x = 0; x < count; ++x, s += sf.bpp, d += df.bpp) {
                uint32_t sv[4];
                sf.split(sf.read(s), sv);
                if (!sv[3]) continue;
                uint32_t dv[4];
                df.split(df.read(d), dv);
                for (int i = 0; i < 3; ++i) dv[i] = ((sv[i] - dv[i]) * sv[3] + 255u) / 256u + dv[i];
                df.write(d, dv);
            }
        }
        src += (uint32_t)(count * (int)sf.bpp) + (uint32_t)srcskip;
        dst += (uint32_t)(count * (int)df.bpp) + (uint32_t)dstskip;
    }
    return 1;
}

// ---- Development: RECOMP_BENCH_GFX times the lifted SDL_gfx primitives the
// game uses for effects (filledTrigonRGBA: laser triangles; boxRGBA: flashes)
// on a 256x224 ARGB surface, and with a native version present checks that it
// draws exactly the same pixels.
namespace {
constexpr uint32_t kSdlCreateRGBSurface = 0x5843dc50u, kSdlFreeSurface = 0x5843f360u;
constexpr uint32_t kFilledTrigonRGBA = 0x58507040u, kBoxRGBA = 0x585038d0u;
uint32_t guest_call(Cpu* c, uint32_t va, std::initializer_list<uint32_t> args) {
    const Cpu saved = *c;
    std::vector<uint32_t> a(args);
    rt_guest_call(va, (unsigned)a.size(), a.data());
    const uint32_t r = c->eax;
    *c = saved;
    return r;
}
}  // namespace

extern "C" void rt_dev_bench(Cpu* c) {
    if (!std::getenv("RECOMP_BENCH_GFX")) return;
    const uint32_t surf = guest_call(c, kSdlCreateRGBSurface,
                                     {0, 256, 224, 32, 0xff0000u, 0xff00u, 0xffu, 0xff000000u});
    if (!surf) { std::fprintf(stderr, "bench: no surface\n"); return; }
    uint32_t seed = 12345;
    auto rnd = [&](uint32_t n) { seed = seed * 1103515245u + 12345u; return (seed >> 8) % n; };
    auto run = [&](const char* name, int n, bool tri) {
        const auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < n; ++i) {
            const uint32_t r = rnd(256), g = rnd(256), b = rnd(256), a = 64 + rnd(192);
            if (tri)
                guest_call(c, kFilledTrigonRGBA, {surf, rnd(300) - 20, rnd(260) - 20, rnd(300) - 20, rnd(260) - 20,
                                                  rnd(300) - 20, rnd(260) - 20, r, g, b, a});
            else
                guest_call(c, kBoxRGBA, {surf, rnd(256), rnd(224), rnd(256), rnd(224), r, g, b, a});
        }
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        std::fprintf(stderr, "bench: %d x %s: %.2f ms (%.3f ms each)\n", n, name, ms, ms / n);
    };
    run("filledTrigonRGBA", 500, true);
    run("boxRGBA", 500, false);
    {
        const uint32_t src = guest_call(c, kSdlCreateRGBSurface,
                                        {0, 64, 64, 32, 0xffu, 0xff00u, 0xff0000u, 0xff000000u});
        const double z = 2.0;
        uint32_t zw[2];
        std::memcpy(zw, &z, 8);
        const auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < 200; ++i) {
            const uint32_t out = guest_call(c, 0x5850b7c0u /* zoomSurface */, {src, zw[0], zw[1], zw[0], zw[1], 0});
            if (out) guest_call(c, kSdlFreeSurface, {out});
        }
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        std::fprintf(stderr, "bench: 200 x zoomSurface 64x64 x2: %.2f ms (%.3f ms each)\n", ms, ms / 200);
        guest_call(c, kSdlFreeSurface, {src});
    }
    guest_call(c, kSdlFreeSurface, {surf});
}

// ---- Switch 2: no keyboard, so OPTIONS has no MAP KEYBOARD INPUTS (its
// screen waits for key presses the console cannot send: a softlock). The
// options items are built from global label objects (MSVC std::string: text
// at +4 or *(+4) when capacity >= 16, size +0x14, capacity +0x18) and ids:
// "MAP KEYBOARD INPUTS" 0x6c6f74 with id 0x6c6f90 (0x39), and the " -" under
// it, 0x6c6e74 (used only there). Blanking both and giving the item id 0
// before the menu is first built leaves an empty row the cursor skips (the
// game already skips id 0 items when moving). RECOMP_NO_KEYBOARD_MENU
// previews it on desktop.
static bool no_keyboard_menu() {
#if defined(__SWITCH__)
    return true;
#else
    static const bool on = std::getenv("RECOMP_NO_KEYBOARD_MENU") != nullptr;
    return on;
#endif
}
static void blank_string(uint32_t str) {
    const uint32_t size = rt_rd32(str + 0x14u), cap = rt_rd32(str + 0x18u);
    const uint32_t text = cap >= 16u ? rt_rd32(str + 4u) : str + 4u;
    if (text && size < 64u) std::memset(rt_mem + text, ' ', size);
}
// Once per frame from SDL_GL_SwapBuffers (the static initialisers have run);
// does its work on the first call.
extern "C" void rt_menu_tweaks() {
    static bool done = false;
    if (done || !no_keyboard_menu()) return;
    done = true;
    blank_string(0x6c6f74u);
    blank_string(0x6c6e74u);
    rt_wr32(0x6c6f90u, 0);
    // The mapping prompts (" PUSH KEY FOR UP" ... " PUSH KEY FOR START", .rdata
    // 0x65f2fc-0x65f3b8, 20-byte slots) say BUTTON instead of KEY; the new text
    // is shorter, so it fits in place.
    static const char kOld[] = " PUSH KEY FOR ";
    for (uint32_t a = 0x65f2fcu; a <= 0x65f3b8u; a += 4u) {
        char* str = (char*)(rt_mem + a);
        if (std::strncmp(str, kOld, sizeof kOld - 1) != 0) continue;
        char buf[32];
        std::snprintf(buf, sizeof buf, " BUTTON FOR %s", str + sizeof kOld - 1);
        std::memcpy(str, buf, std::strlen(buf) + 1);
    }
}
// Options update (0x4fc280, thiscall): items at this+0xff0 (vector, begin +0xc,
// end +0x10; 32-byte elements, id +0x1c), cursor this+0xfcc. The cursor starts
// on item 0, the now-blank row: move it to the first selectable item.
extern "C" void rt_hook_options_update(Cpu* c) {
    if (!no_keyboard_menu()) return;
    const uint32_t vec = c->ecx + 0xff0u;
    const uint32_t first = rt_rd32(vec + 0xcu), last = rt_rd32(vec + 0x10u);
    if (!first || last <= first || (last - first) / 32u > 64u) return;
    const uint32_t n = (last - first) / 32u;
    uint32_t cur = rt_rd32(c->ecx + 0xfccu);
    if (cur >= n || rt_rd32(first + 32u * cur + 0x1cu) != 0) return;
    for (uint32_t k = 1; k < n; ++k) {
        const uint32_t i = (cur + k) % n;
        if (rt_rd32(first + 32u * i + 0x1cu) != 0) { rt_wr32(c->ecx + 0xfccu, i); return; }
    }
}

// ---- Development: RECOMP_TRACE_STATES logs every state change (0x4405c0,
// cdecl (const char* name, machine)), and rt_last_state() lets the dev input
// script (RECOMP_AUTOPRESS, imports_sdl.cpp) wait for a state.
static char g_last_state[64] = "";
extern "C" const char* rt_last_state() { return g_last_state; }
extern "C" void rt_hook_state_change(Cpu* c) {
    const uint32_t name = rt_rd32(c->esp + 4u);
    if (!name) return;
    std::snprintf(g_last_state, sizeof g_last_state, "%s", (const char*)(rt_mem + name));
    static const bool trace = std::getenv("RECOMP_TRACE_STATES") != nullptr;
    if (trace) std::fprintf(stderr, "state: %s (machine %#x)\n", g_last_state, rt_rd32(c->esp + 8u));
}
