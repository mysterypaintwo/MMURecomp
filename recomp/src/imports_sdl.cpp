// Host (SDL2) implementations of the OS-facing SDL 1.2 exports, the 18
// OPENGL32 entry points MMU.exe uses, and USER32 bits. Everything else in
// SDL.dll / SDL_mixer / SDL_gfx runs as lifted original code.
//
// Rendering: the game composes each frame on software SDL 1.2 surfaces,
// uploads it with glTexImage2D and draws textured quads under glOrtho. We
// keep GL texture objects as SDL2 textures and draw the quads with
// SDL_RenderGeometry. Audio: SDL_mixer's lifted mix callback is pulled on the
// game thread and queued to an SDL2 audio device (no guest threads).
#include "recomp/runtime.h"
#include "recomp/shims.h"
#include "port.h"

extern "C" uint64_t rt_io_bytes;  // imports_kernel32.cpp

#include <SDL.h>

#include <dirent.h>
#include <sys/stat.h>
#include <array>
#include <deque>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" uint32_t rt_guest_call(uint32_t va, unsigned nargs, const uint32_t* args);
extern "C" uint32_t rt_heap_used(void);
extern "C" uint32_t rt_find_export(const char* module, const char* name);

namespace {

#define ARG(n) rt_arg(c, n)
#define ARGF(n) rt_argf(c, n)
#define CRET(v) do { c->eax = (uint32_t)(v); RT_RET_CDECL(c); return; } while (0)
#define SRET(v, n) do { c->eax = (uint32_t)(v); RT_RET_STDCALL(c, n); return; } while (0)

SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
uint32_t g_screen = 0;        // guest SDL_Surface* returned by SDL_SetVideoMode
uint32_t g_keystate = 0;      // guest Uint8[SDLK_LAST]
SDL_Joystick* g_fake_pad = nullptr;  // RECOMP_FAKE_PAD (development)
constexpr int kSdlkLast = 323;
uint32_t g_mod = 0;
uint32_t g_errbuf = 0;
const auto g_t0 = std::chrono::steady_clock::now();

// ---- SDL 1.2 key mapping
int key12(SDL_Keycode k) {
    if (k >= 0 && k < 128) return (int)k; // ASCII range is identical
    switch (k) {
    case SDLK_UP: return 273; case SDLK_DOWN: return 274; case SDLK_RIGHT: return 275;
    case SDLK_LEFT: return 276; case SDLK_INSERT: return 277; case SDLK_HOME: return 278;
    case SDLK_END: return 279; case SDLK_PAGEUP: return 280; case SDLK_PAGEDOWN: return 281;
    case SDLK_NUMLOCKCLEAR: return 300; case SDLK_CAPSLOCK: return 301;
    case SDLK_SCROLLLOCK: return 302; case SDLK_RSHIFT: return 303; case SDLK_LSHIFT: return 304;
    case SDLK_RCTRL: return 305; case SDLK_LCTRL: return 306; case SDLK_RALT: return 307;
    case SDLK_LALT: return 308; case SDLK_RGUI: return 309; case SDLK_LGUI: return 310;
    case SDLK_MODE: return 313; case SDLK_PRINTSCREEN: return 316; case SDLK_PAUSE: return 19;
    case SDLK_KP_PERIOD: return 266; case SDLK_KP_DIVIDE: return 267;
    case SDLK_KP_MULTIPLY: return 268; case SDLK_KP_MINUS: return 269; case SDLK_KP_PLUS: return 270;
    case SDLK_KP_ENTER: return 271; case SDLK_KP_EQUALS: return 272;
    default: break;
    }
    if (k >= SDLK_F1 && k <= SDLK_F12) return 282 + (int)(k - SDLK_F1);
    if (k >= SDLK_KP_1 && k <= SDLK_KP_9) return 257 + (int)(k - SDLK_KP_1);
    if (k == SDLK_KP_0) return 256;
    return 0;
}
uint32_t mod12(Uint16 m) {
    uint32_t r = 0;
    if (m & KMOD_LSHIFT) r |= 0x0001; if (m & KMOD_RSHIFT) r |= 0x0002;
    if (m & KMOD_LCTRL) r |= 0x0040; if (m & KMOD_RCTRL) r |= 0x0080;
    if (m & KMOD_LALT) r |= 0x0100; if (m & KMOD_RALT) r |= 0x0200;
    if (m & KMOD_NUM) r |= 0x1000; if (m & KMOD_CAPS) r |= 0x2000;
    return r;
}

// ---- joysticks
std::vector<SDL_Joystick*> g_joys;
uint32_t joy_handle(int index) { return 0x7f000000u + (uint32_t)index; }
SDL_Joystick* joy_of(uint32_t h) {
    const uint32_t i = h - 0x7f000000u;
    return i < g_joys.size() ? g_joys[i] : nullptr;
}
// SDL 1.2 events carry the device index; SDL2 events carry an instance id.
int joy_index_of(SDL_JoystickID id) {
    for (size_t i = 0; i < g_joys.size(); ++i)
        if (g_joys[i] && SDL_JoystickInstanceID(g_joys[i]) == id) return (int)i;
    return -1;
}

// Virtual pad (default on Switch, RECOMP_VIRTUAL_PAD=1 elsewhere): the game
// reads joystick 0 through events only (0x440d30: axes 0-2, hat 0, buttons
// 0-9 -> virtual keys 323..342). A GameController is presented with the
// button numbers of the game's factory defaults (0x4fbb10 / "RESTORE
// DEFAULT"): b0 fire, b1 jump, b2 slide, b4 previous weapon, b5 next weapon,
// b8 select, b9 start. SDL GameController buttons are positional (Xbox
// names), so on Switch: B (south) jump, Y (west) fire, ZR slide, L next
// weapon, R previous weapon, + start, - select; D-pad -> hat, left stick ->
// axes 0/1.
bool virtual_pad_enabled() {
    static const bool on = [] {
        if (const char* v = getenv("RECOMP_VIRTUAL_PAD")) return atoi(v) != 0;
#if defined(__SWITCH__)
        return true;
#else
        return false;
#endif
    }();
    return on;
}
std::vector<SDL_GameController*> g_pads;  // by device index, when virtual
uint8_t g_pad_hat = 0, g_pad_trig[2] = {0, 0};
int8_t g_last_zone[2] = {0, 0};  // left stick zone per axis (-1/0/1)
int pad_button(int b) {
    switch (b) {
    case SDL_CONTROLLER_BUTTON_A: return 1;              // south (Switch B): jump
    case SDL_CONTROLLER_BUTTON_X: return 0;              // west (Switch Y): fire
    case SDL_CONTROLLER_BUTTON_B: return 3;              // east (Switch A)
    case SDL_CONTROLLER_BUTTON_Y: return 6;              // north (Switch X)
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return 5;   // L: next weapon
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return 4;  // R: previous weapon
    case SDL_CONTROLLER_BUTTON_BACK: return 8;           // -: select
    case SDL_CONTROLLER_BUTTON_START: return 9;          // +: start
    default: return -1;
    }
}
uint8_t pad_hat_bit(int b) {
    switch (b) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP: return SDL_HAT_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return SDL_HAT_RIGHT;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return SDL_HAT_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return SDL_HAT_LEFT;
    default: return 0;
    }
}
bool is_virtual(int index) { return index >= 0 && (size_t)index < g_pads.size() && g_pads[(size_t)index]; }

// ---- skip chord on the joystick mapping screen
// The mapping screen (0x4fcf30, hooked: rt_hook_joymap) skips an entry on an
// Escape key-state edge. A controller has no Escape, so - and ZR pressed
// within kChordFrames frames of each other skip instead. While that screen is
// active (only then), a - or ZR press is held back for up to kChordFrames
// frames to tell a chord from a deliberate press of one of them, which is then
// delivered late (with its release, if that came meanwhile).
constexpr unsigned kChordFrames = 3;
unsigned g_frame = 0;               // SDL_GL_SwapBuffers count
unsigned g_joymap_frame = ~0u;      // last frame the mapping screen ran
unsigned g_esc_release = 0;         // frame at which the synthetic Escape is released
std::deque<std::array<uint8_t, 20>> g_pending_events;
struct ChordKey { bool held = false, released = false, swallow_release = false; unsigned frame = 0; uint8_t which = 0; };
ChordKey g_chord[2];  // [0] = - (virtual b8), [1] = ZR (virtual b2)
int chord_slot(uint8_t button) { return button == 8 ? 0 : button == 2 ? 1 : -1; }
bool joymap_active() { return g_joymap_frame != ~0u && g_frame - g_joymap_frame <= 2; }
void queue_button(uint8_t which, uint8_t button, bool down) {
    std::array<uint8_t, 20> e{};
    e[0] = down ? 10 : 11; e[1] = which; e[2] = button; e[3] = down;
    g_pending_events.push_back(e);
}
void press_escape() {
    if (!g_keystate) return;
    rt_mem[g_keystate + 27] = 1;
    g_esc_release = g_frame + 2;
}
// true: deliver this (virtual pad) event now.
bool chord_filter(const uint8_t* out) {
    if (out[0] != 10 && out[0] != 11) return true;
    const int slot = chord_slot(out[2]);
    if (slot < 0) return true;
    ChordKey& k = g_chord[slot];
    ChordKey& other = g_chord[1 - slot];
    if (out[0] == 10) {
        if (!joymap_active()) return true;
        if (other.held && g_frame - other.frame <= kChordFrames) {
            other.held = false;
            other.swallow_release = !other.released;
            k = ChordKey{};
            k.swallow_release = true;
            press_escape();
            return false;
        }
        k = ChordKey{true, false, false, g_frame, out[1]};
        return false;
    }
    if (k.swallow_release) { k.swallow_release = false; return false; }
    if (k.held) { k.released = true; return false; }
    return true;
}
void chord_tick() {
    if (g_esc_release && g_frame >= g_esc_release && g_keystate) {
        rt_mem[g_keystate + 27] = 0;
        g_esc_release = 0;
    }
    for (int slot = 0; slot < 2; ++slot) {
        ChordKey& k = g_chord[slot];
        if (!k.held || g_frame - k.frame <= kChordFrames) continue;
        const uint8_t button = slot == 0 ? 8 : 2;
        queue_button(k.which, button, true);
        if (k.released) queue_button(k.which, button, false);
        k = ChordKey{};
    }
}

// ---- audio
SDL_AudioDeviceID g_audio = 0;
uint32_t g_audio_cb = 0, g_audio_ud = 0, g_audio_buf = 0, g_audio_len = 0;
bool g_audio_paused = true;
SDL_AudioSpec g_audio_spec{};
void pump_audio() {
    if (!g_audio || g_audio_paused || !g_audio_cb) return;
    const Uint32 target = (Uint32)g_audio_len * 3; // keep ~3 callback buffers queued
    int guard = 0;
    while (SDL_GetQueuedAudioSize(g_audio) < target && guard++ < 8) {
        memset(rt_mem + g_audio_buf, g_audio_spec.silence, g_audio_len);
        const uint32_t args[3] = {g_audio_ud, g_audio_buf, g_audio_len};
        rt_guest_call(g_audio_cb, 3, args);
        // Master volume (port menu): ramped across the buffer so changes are smooth.
        static float gain = 1.0f;
        const float target_gain = rt_master_volume();
        if ((gain < 0.999f || target_gain < 0.999f) && g_audio_spec.format == AUDIO_S16LSB) {
            int16_t* p = (int16_t*)(rt_mem + g_audio_buf);
            const uint32_t n = g_audio_len / 2;
            const float step = (target_gain - gain) / (float)(n ? n : 1);
            for (uint32_t k = 0; k < n; ++k, gain += step) p[k] = (int16_t)(p[k] * gain);
        }
        gain = target_gain;
        SDL_QueueAudio(g_audio, rt_mem + g_audio_buf, g_audio_len);
        static const bool stats = getenv("RECOMP_AUDIO_STATS") != nullptr;
        if (stats && g_audio_spec.format == AUDIO_S16LSB) {
            static double acc = 0;
            static uint64_t n = 0, bytes = 0;
            const int16_t* p = (const int16_t*)(rt_mem + g_audio_buf);
            for (uint32_t k = 0; k < g_audio_len / 2; ++k) acc += (double)p[k] * p[k];
            n += g_audio_len / 2;
            bytes += g_audio_len;
            const uint64_t per_sec = (uint64_t)g_audio_spec.freq * g_audio_spec.channels * 2;
            if (bytes >= per_sec) {
                fprintf(stderr, "audio rms=%.0f\n", sqrt(acc / (double)n));
                acc = 0; n = 0; bytes = 0;
            }
        }
    }
}

// ---- GL emulation (fixed-function subset used by MMU.exe)
struct GlTex {
    SDL_Texture* tex = nullptr;
    int w = 0, h = 0;
    bool linear = false;
};
std::unordered_map<uint32_t, GlTex> g_tex;
uint32_t g_next_tex = 1, g_bound = 0;
float g_proj[16], g_model[16];
int g_matrix_mode = 0x1700; // GL_MODELVIEW
int g_vp[4] = {0, 0, 256, 224};
float g_clear[4] = {0, 0, 0, 1};
std::vector<SDL_Vertex> g_verts;
float g_tc[2] = {0, 0};
int g_prim = -1;
bool g_texture2d = false;
bool g_blend = false; // GL_BLEND: MMU.exe never enables it, so quads are opaque
void identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}
float* cur_matrix() { return g_matrix_mode == 0x1701 ? g_proj : g_model; }
void mul(float* m, const float* r) { // m = m * r (column-major)
    float o[16];
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row) {
            float s = 0;
            for (int k = 0; k < 4; ++k) s += m[k * 4 + row] * r[col * 4 + k];
            o[col * 4 + row] = s;
        }
    memcpy(m, o, sizeof(o));
}
// Long work between frames (DISPLAY SIZE changes re-upload every texture)
// would starve the audio queue: GL shims top it up every few milliseconds.
void pump_audio_soon() {
    static auto last = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    if (now - last < std::chrono::milliseconds(4)) return;
    last = now;
    pump_audio();
}

// ---- presentation
// The game draws a 256x224 orthographic view (glOrtho 0..256 x 0..224) into
// whatever viewport its DISPLAY SIZE asked for. Everything is drawn into a
// native-resolution target instead and placed once per frame (present_frame):
//   DISPLAY SIZE 1X-4X  integer scale (clamped to what fits), centred
//   FULL                fit the height, square pixels
//   WIDE (port: 6)      fill the window
// The value is read live from the game's options (*[0x6c55d8], getter 0x4bd9f0).
constexpr int kNativeW = 256, kNativeH = 224;
}  // namespace
void rt_pump_audio() { pump_audio(); }
extern "C" bool rt_postfx_draw(SDL_Renderer* renderer, SDL_Texture* tex, const SDL_Rect* vp);
extern "C" void rt_postfx_finish(SDL_Renderer* r);
extern "C" void rt_postfx_set(int mode);
extern "C" int rt_postfx_mode(void);
extern "C" void rt_postfx_set_filter(int n);
extern "C" int rt_postfx_filter(void);
extern "C" void rt_postfx_save(FILE* f);
extern "C" void rt_postfx_load_line(const char* line);
namespace {
// Port settings (shader) kept next to the logs on Switch, in the game folder
// elsewhere: mmu_port.ini, one "key=value" per line.
std::string port_ini_path() {
#if defined(__SWITCH__)
    return "sdmc:/switch/mmu_recomp/mmu_port.ini";
#else
    return std::string(rt_game_dir()) + "/mmu_port.ini";
#endif
}
void load_port_settings() {
    if (FILE* f = fopen(port_ini_path().c_str(), "r")) {
        char line[128];
        while (fgets(line, sizeof line, f)) {
            int v;
            if (sscanf(line, "shader=%d", &v) == 1) rt_postfx_set(v);
            if (sscanf(line, "filter=%d", &v) == 1) rt_postfx_set_filter(v);
            rt_postfx_load_line(line);
            if (sscanf(line, "master_volume=%d", &v) == 1) rt_menu_set_master(v / 100.0f);
        }
        fclose(f);
    }
    if (const char* v = getenv("RECOMP_SHADER")) rt_postfx_set(atoi(v));
}
SDL_Texture* g_target = nullptr;
void present_logical_size() {}
// The game's own FILTERING option smooths the final scale (unless a shader draws it).
void apply_filtering() {
    const uint32_t opts = rt_rd32(0x6c55d8u);
    const bool smooth = opts && rt_rd32(opts + 0xc) == 1;
    static int last = -1;
    if (g_target && (int)smooth != last) {
        SDL_SetTextureScaleMode(g_target, smooth ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
        last = smooth;
    }
}
int display_size_option() {
    const uint32_t opts = rt_rd32(0x6c55d8u);
    return opts ? (int)rt_rd32(opts) : 1;
}
void ensure_target() {
    if (g_target || !g_renderer) return;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    g_target = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, kNativeW, kNativeH);
    if (g_target) {
        SDL_SetTextureBlendMode(g_target, SDL_BLENDMODE_NONE);
        SDL_SetRenderTarget(g_renderer, g_target);
    }
}
SDL_Rect frame_rect(int ow, int oh) {
    const int v = display_size_option();
    if (v == 6) return SDL_Rect{0, 0, ow, oh};
    int w, h;
    if (v >= 1 && v <= 4) {
        // N game pixels per window point (HiDPI windows have more pixels per point).
        int ww = ow;
        if (g_window) SDL_GetWindowSize(g_window, &ww, nullptr);
        int n = v * (ww > 0 && ow >= ww ? ow / ww : 1);
        while (n > 1 && (kNativeW * n > ow || kNativeH * n > oh)) --n;
        w = kNativeW * n;
        h = kNativeH * n;
    } else {
        h = oh;
        w = oh * kNativeW / kNativeH;
        if (w > ow) { w = ow; h = ow * kNativeH / kNativeW; }
    }
    return SDL_Rect{(ow - w) / 2, (oh - h) / 2, w, h};
}
void sync_window_to_option() {
#if !defined(__SWITCH__)
    // Desktop: follow DISPLAY SIZE changes live (Windows needs a restart).
    static int last = -1;
    const int v = display_size_option();
    if (!g_window || v == last) return;
    last = v;
    if (v == 5) {
        SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(g_window, 0);
        if (v >= 1 && v <= 4) SDL_SetWindowSize(g_window, kNativeW * v, kNativeH * v);
    }
#endif
}
// "L AND X FOR MENU" in the game's font, drawn into the 256x224 frame once the
// initial load is done: held ~2.5 s, then faded in 4 quantized steps (NES style).
uint64_t assets_size();
void draw_menu_hint() {
    using namespace std::chrono;
    static steady_clock::time_point start;
    static bool started = false;
    if (!started) {
        const uint64_t total = assets_size();
        if (!total || rt_io_bytes < total || g_frame < 3) return;
        start = steady_clock::now();
        started = true;
    }
    // Hold 2.5 s, then 4 quantized alpha steps of 8 frames at 60 Hz each.
    static const uint8_t kAlpha[4] = {255, 170, 85, 40};
    const double t = duration<double>(steady_clock::now() - start).count();
    const int step = t < 2.5 ? 0 : 1 + (int)((t - 2.5) / (8.0 / 60.0));
    if (step > 3 || rt_menu_open()) return;
#if defined(__SWITCH__)
    const char* text = "L AND X FOR MENU";
#else
    const char* text = "F1 / L AND X FOR MENU";
#endif
    rt_text_draw(g_renderer, text, (kNativeW - rt_text_width(text)) / 2, kNativeH - 24, 1, kAlpha[step]);
}

void present_frame() {
    if (!g_renderer) return;
    ensure_target();
    sync_window_to_option();
    draw_menu_hint();  // into the game target: scaled and shaded with the game
    SDL_SetRenderTarget(g_renderer, nullptr);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    int ow = 0, oh = 0;
    SDL_GetRendererOutputSize(g_renderer, &ow, &oh);
    const SDL_Rect dst = frame_rect(ow, oh);
    apply_filtering();
    if (g_target && !rt_postfx_draw(g_renderer, g_target, &dst)) SDL_RenderCopy(g_renderer, g_target, nullptr, &dst);
    rt_menu_draw();
}

uint32_t guest_screen_surface(int w, int h) {
    // SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, R, G, B, A) in lifted SDL.dll.
    const uint32_t create = rt_find_export("SDL.dll", "SDL_CreateRGBSurface");
    const uint32_t args[8] = {0, (uint32_t)w, (uint32_t)h, 32, 0x00ff0000u, 0x0000ff00u, 0x000000ffu, 0};
    return rt_guest_call(create, 8, args);
}

bool gl_trace(const char* name) {
    static const bool on = getenv("RECOMP_GLTRACE") != nullptr;
    static std::unordered_map<std::string, int> counts;
    return on && counts[name]++ < 6;
}

} // namespace


// ---- loading bar
// The game loads its assets (~410 MiB of Assets/*.mmu) in one blocking call
// after the first frame. While frames stall and files are being read, draw a
// progress bar (bytes read / size of Assets/) so the screen is not frozen.
namespace {
std::chrono::steady_clock::time_point g_last_swap = std::chrono::steady_clock::now();
uint64_t assets_size() {
    static uint64_t total = [] {
        uint64_t sum = 0;
        const std::string dir = std::string(rt_game_dir()) + "/Assets";
        if (DIR* d = opendir(dir.c_str())) {
            while (dirent* e = readdir(d)) {
                const std::string name = e->d_name;
                if (name.size() < 4 || name.compare(name.size() - 4, 4, ".mmu") != 0) continue;
                struct stat st;
                if (stat((dir + "/" + name).c_str(), &st) == 0) sum += (uint64_t)st.st_size;
            }
            closedir(d);
        }
        return sum;
    }();
    return total;
}
}  // namespace
extern "C" uint64_t rt_io_bytes;
extern "C" void rt_load_progress(void) {
    using namespace std::chrono;
    if (!g_renderer) return;
    const auto now = steady_clock::now();
    static steady_clock::time_point last_draw;
    if (now - g_last_swap < milliseconds(150) || now - last_draw < milliseconds(33)) return;
    last_draw = now;
    const uint64_t total = assets_size();
    if (!total || rt_io_bytes >= total) return;  // only the initial asset load
    const double f = (double)rt_io_bytes / (double)total;
    SDL_PumpEvents();  // keep the OS (HOME menu, window) responsive
    SDL_SetRenderTarget(g_renderer, nullptr);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    int ow = 0, oh = 0;
    SDL_GetRendererOutputSize(g_renderer, &ow, &oh);
    const SDL_Rect frame = frame_rect(ow, oh);
    // Over the game's last frame (its loading splash), not a blank screen.
    if (g_target && !rt_postfx_draw(g_renderer, g_target, &frame)) SDL_RenderCopy(g_renderer, g_target, nullptr, &frame);
    const int px = std::max(1, frame.h / kNativeH);  // one game pixel
    const SDL_Rect outline{frame.x + frame.w / 8, frame.y + frame.h * 3 / 4, frame.w * 3 / 4, 8 * px};
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(g_renderer, &outline);
    const SDL_Rect inner{outline.x + px, outline.y + px, outline.w - 2 * px, outline.h - 2 * px};
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(g_renderer, &inner);
    const SDL_Rect fill{inner.x + px, inner.y + px, (int)((inner.w - 2 * px) * f), inner.h - 2 * px};
    SDL_SetRenderDrawColor(g_renderer, 0, 168, 248, 255);  // Mega Man blue
    SDL_RenderFillRect(g_renderer, &fill);
    SDL_RenderPresent(g_renderer);
    SDL_SetRenderTarget(g_renderer, g_target);
    pump_audio();
}

unsigned rt_frame_count() { return g_frame; }
SDL_Texture* rt_game_target() { return g_target; }
void rt_release_virtual_pad() {
    for (size_t j = 0; j < g_pads.size(); ++j) {
        if (!g_pads[j]) continue;
        for (uint8_t b = 0; b < 10; ++b) queue_button((uint8_t)j, b, false);
        std::array<uint8_t, 20> hat{};
        hat[0] = 9; hat[1] = (uint8_t)j;
        g_pending_events.push_back(hat);
        for (uint8_t ax = 0; ax < 2; ++ax) {
            std::array<uint8_t, 20> a{};
            a[0] = 7; a[1] = (uint8_t)j; a[2] = ax;
            g_pending_events.push_back(a);
        }
    }
    g_pad_hat = 0;
    g_pad_trig[0] = g_pad_trig[1] = 0;
    g_last_zone[0] = g_last_zone[1] = 0;
}
void rt_save_port_settings() {
    if (FILE* f = fopen(port_ini_path().c_str(), "w")) {
        fprintf(f, "shader=%d\nfilter=%d\nmaster_volume=%d\n", rt_postfx_mode(), rt_postfx_filter(),
                (int)(rt_master_volume() * 100 + 0.5f));
        rt_postfx_save(f);
        fclose(f);
    }
}

// Joystick mapping screen ran this frame (HOOKS in tools/recomp/lift.py).
extern "C" void rt_hook_joymap(Cpu*) { g_joymap_frame = g_frame; }

// Called from blocking points so audio keeps flowing.
extern "C" void rt_host_pump(void) { pump_audio(); }

#define SDLX(name) extern "C" void imp_SDL__##name(Cpu* c)
#define GLX(name) extern "C" void imp_OPENGL32__##name(Cpu* c)
#define U32X(name) extern "C" void imp_USER32__##name(Cpu* c)

// ---------------------------------------------------------------- core
SDLX(SDL_Init) {
    const uint32_t f = ARG(0);
    Uint32 flags = SDL_INIT_TIMER;
    if (f & 0x20) flags |= SDL_INIT_VIDEO;
    if (f & 0x10) flags |= SDL_INIT_AUDIO;
    if (f & 0x200) flags |= SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;
    if (!g_keystate) g_keystate = rt_malloc(kSdlkLast);
    const int r = SDL_InitSubSystem(flags);
    // Development: RECOMP_FAKE_PAD attaches a virtual game controller that the
    // RECOMP_AUTOPRESS script presses ("pad:<SDL button name>").
    if (r == 0 && (flags & SDL_INIT_JOYSTICK) && getenv("RECOMP_FAKE_PAD") && !g_fake_pad) {
        SDL_VirtualJoystickDesc d{};
        d.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
        d.type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
        d.naxes = SDL_CONTROLLER_AXIS_MAX;
        d.nbuttons = SDL_CONTROLLER_BUTTON_MAX;
        d.name = "Fake pad";
        const int idx = SDL_JoystickAttachVirtualEx(&d);
        if (idx >= 0) g_fake_pad = SDL_JoystickOpen(idx);
        fprintf(stderr, "recomp: fake pad %s\n", g_fake_pad ? "attached" : SDL_GetError());
    }
    CRET(r == 0 ? 0 : (uint32_t)-1);
}
SDLX(SDL_InitSubSystem) { imp_SDL__SDL_Init(c); }
SDLX(SDL_QuitSubSystem) { CRET(0); }
SDLX(SDL_WasInit) { CRET(ARG(0)); }
SDLX(SDL_Quit) {
    if (g_audio) SDL_CloseAudioDevice(g_audio);
    g_audio = 0;
    CRET(0);
}
SDLX(SDL_SetModuleHandle) { CRET(0); }
SDLX(SDL_RegisterApp) { CRET(0); }
SDLX(SDL_UnregisterApp) { CRET(0); }
SDLX(SDL_getenv) { CRET(0); }
SDLX(SDL_putenv) { CRET(0); }
SDLX(SDL_CDNumDrives) { CRET(0); }

// ---------------------------------------------------------------- video
SDLX(SDL_SetVideoMode) {
    pump_audio_soon();
    const int w = (int)ARG(0), h = (int)ARG(1);
    const uint32_t flags = ARG(3);
    fprintf(stderr, "recomp: SDL_SetVideoMode(%d, %d, %u, 0x%x)\n", w, h, ARG(2), flags);
    if (!g_window) {
        // DISPLAY SIZE N requests 256N x 224N; FULL requests the desktop size
        // with SDL_FULLSCREEN (0x80000000). On Switch the window is the screen.
        const bool full = (flags & 0x80000000u) != 0;
#if defined(__SWITCH__)
        // The window is the screen: create it at the panel's size (a smaller
        // window is scaled up by the system: blurry and stretched).
        SDL_DisplayMode dm{};
        if (SDL_GetCurrentDisplayMode(0, &dm) != 0 || dm.w <= 0) { dm.w = 1280; dm.h = 720; }
        (void)full;
        g_window = SDL_CreateWindow("Mega Man Unlimited", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w, dm.h,
                                    SDL_WINDOW_SHOWN);
        fprintf(stderr, "recomp: window %dx%d (game asked %dx%d)\n", dm.w, dm.h, w, h);
#else
        g_window = SDL_CreateWindow("Mega Man Unlimited", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    full ? kNativeW * 3 : w, full ? kNativeH * 3 : h,
                                    SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI |
                                        (full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
#endif
        g_renderer = SDL_CreateRenderer(g_window, -1,
                                        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
        if (!g_target) fprintf(stderr, "recomp: renderer %s\n", [] { SDL_RendererInfo i{}; SDL_GetRendererInfo(g_renderer, &i); return i.name ? i.name : "?"; }());
        identity(g_proj);
        identity(g_model);
        ensure_target();
        rt_menu_init(g_window, g_renderer);
        load_port_settings();
    }
    g_vp[2] = w;
    g_vp[3] = h;
    present_logical_size();
    if (!g_screen) g_screen = guest_screen_surface(w, h);
    if (g_screen) rt_wr32(g_screen, rt_rd32(g_screen) | (flags & 0x2u)); // keep SDL_OPENGL
    CRET(g_screen);
}
SDLX(SDL_GetVideoSurface) { CRET(g_screen); }
SDLX(SDL_GetVideoInfo) {
    static uint32_t info = 0;
    if (!info) info = rt_malloc(24);
    CRET(info);
}
SDLX(SDL_VideoDriverName) { CRET(0); }
SDLX(SDL_ListModes) { CRET(0xffffffffu); }
SDLX(SDL_VideoModeOK) { CRET(ARG(2) ? ARG(2) : 32); }
SDLX(SDL_WM_SetCaption) {
    if (g_window && ARG(0)) SDL_SetWindowTitle(g_window, rt_str(ARG(0)));
    CRET(0);
}
SDLX(SDL_WM_SetIcon) { CRET(0); }
SDLX(SDL_WM_GrabInput) { CRET(0); }
SDLX(SDL_WM_IconifyWindow) { CRET(1); }
SDLX(SDL_WM_ToggleFullScreen) {
    if (g_window) {
        const bool fs = SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_SetWindowFullscreen(g_window, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    CRET(1);
}
SDLX(SDL_ShowCursor) { CRET(SDL_ShowCursor((int)ARG(0))); }
SDLX(SDL_UpdateRect) { CRET(0); }
SDLX(SDL_UpdateRects) { CRET(0); }
SDLX(SDL_Flip) { CRET(0); }

// ---------------------------------------------------------------- GL attributes / swap
SDLX(SDL_GL_SetAttribute) { CRET(0); }
SDLX(SDL_GL_GetAttribute) {
    if (ARG(1)) rt_wr32(ARG(1), ARG(0) == 16 /* SWAP_CONTROL */ ? 1u : 8u);
    CRET(0);
}
SDLX(SDL_GL_GetProcAddress) { CRET(0); }
SDLX(SDL_GL_LoadLibrary) { CRET(0); }
extern "C" void rt_dev_bench(Cpu* c);  // patches.cpp (RECOMP_BENCH_GFX)
extern "C" void rt_menu_tweaks();      // patches.cpp (Switch: no keyboard mapping item)
SDLX(SDL_GL_SwapBuffers) {
    const unsigned frames = ++g_frame;
    if (frames == 60) rt_dev_bench(c);
    rt_menu_tweaks();
    // Development: RECOMP_AUTOPRESS=6,9,12:down,13:ret taps keys at those times
    // (seconds; key ret/up/down/left/right/esc, default ret), e.g. to skip the intro.
    if (static const char* auto_press = getenv("RECOMP_AUTOPRESS"); auto_press) {
        static std::vector<std::pair<double, SDL_Keycode>> presses;  // keycode, or -1 - pad button
        static size_t next = 0;
        static int release_at = -1;
        static SDL_Keycode held = 0;
        if (presses.empty()) {
            for (const char* p = auto_press; *p;) {
                const double at = atof(p);
                SDL_Keycode k = SDLK_RETURN;
                while (*p && *p != ',' && *p != ':') ++p;
                if (*p == ':') {
                    const std::string name(p + 1, strcspn(p + 1, ","));
                    if (name.rfind("pad:", 0) == 0) k = -1 - (SDL_Keycode)SDL_GameControllerGetButtonFromString(name.c_str() + 4);
                    else if (name == "up") k = SDLK_UP;
                    else if (name == "down") k = SDLK_DOWN;
                    else if (name == "left") k = SDLK_LEFT;
                    else if (name == "right") k = SDLK_RIGHT;
                    else if (name == "esc") k = SDLK_ESCAPE;
                    else if (SDL_GetKeyFromName(name.c_str()) != SDLK_UNKNOWN) k = SDL_GetKeyFromName(name.c_str());
                    while (*p && *p != ',') ++p;
                }
                presses.push_back({at, k});
                if (*p) ++p;
            }
        }
        const double t = std::chrono::duration<double>(std::chrono::steady_clock::now() - g_t0).count();
        SDL_Event e{};
        if ((int)frames == release_at && held < 0) {
            if (g_fake_pad) SDL_JoystickSetVirtualButton(g_fake_pad, -1 - held, 0);
        } else if (next < presses.size() && t >= presses[next].first && presses[next].second < 0) {
            held = presses[next].second;
            if (g_fake_pad) SDL_JoystickSetVirtualButton(g_fake_pad, -1 - held, 1);
            ++next; release_at = (int)frames + 20;
        } else if ((int)frames == release_at) {
            e.type = SDL_KEYUP; e.key.keysym.sym = held; e.key.keysym.scancode = SDL_GetScancodeFromKey(held);
            e.key.state = SDL_RELEASED;
            SDL_PushEvent(&e);
        } else if (next < presses.size() && t >= presses[next].first) {
            held = presses[next].second;
            e.type = SDL_KEYDOWN; e.key.keysym.sym = held; e.key.keysym.scancode = SDL_GetScancodeFromKey(held);
            e.key.state = SDL_PRESSED;
            SDL_PushEvent(&e); ++next; release_at = (int)frames + 20;
        }
    }
    {
        using namespace std::chrono;
        const double t = duration_cast<milliseconds>(steady_clock::now() - g_t0).count() / 1000.0;
        static const char* shot = getenv("RECOMP_SCREENSHOT");
        static const double shot_at = getenv("RECOMP_SCREENSHOT_AT") ? atof(getenv("RECOMP_SCREENSHOT_AT")) : 5.0;
        present_frame();
        if (shot && g_renderer && t >= shot_at) {
            int w = 0, h = 0;
            SDL_GetRendererOutputSize(g_renderer, &w, &h);
            SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
            if (s && SDL_RenderReadPixels(g_renderer, nullptr, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch) == 0)
                SDL_SaveBMP(s, shot);
            if (s) SDL_FreeSurface(s);
            fprintf(stderr, "recomp: screenshot %s after %u frames (%.1fs)\n", shot, frames, t);
            shot = nullptr;
        }
        static const double quit_after = getenv("RECOMP_QUIT_AFTER") ? atof(getenv("RECOMP_QUIT_AFTER")) : 0.0;
        if (quit_after > 0.0 && t >= quit_after) {
            fprintf(stderr, "recomp: quitting after %u frames (%.1fs), guest heap high-water %.1f MiB\n", frames, t,
                    rt_heap_used() / 1048576.0);
            exit(0);
        }
    }
    if (g_renderer) {
        SDL_RenderPresent(g_renderer);
        rt_postfx_finish(g_renderer);  // Switch: keep at most one frame queued (input latency)
        SDL_SetRenderTarget(g_renderer, g_target);
    }
    g_last_swap = std::chrono::steady_clock::now();
    pump_audio();
    rt_log_status(frames);
    rt_cheats_tick();
    CRET(0);
}

// ---------------------------------------------------------------- events
SDLX(SDL_PumpEvents) { pump_audio(); CRET(0); }
SDLX(SDL_PollEvent) {
    const uint32_t ev = ARG(0);
    SDL_Event e;
    pump_audio();
    chord_tick();
    if (!g_pending_events.empty()) {
        if (ev) memcpy(rt_mem + ev, g_pending_events.front().data(), 20);
        g_pending_events.pop_front();
        CRET(1);
    }
    while (SDL_PollEvent(&e)) {
        if (rt_menu_event(e)) continue;  // port menu open (or its toggle): not for the game
        uint8_t out[20];
        memset(out, 0, sizeof(out));
        bool emit = false;
        switch (e.type) {
        case SDL_QUIT:
            out[0] = 12; emit = true; break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            if (e.key.repeat) break;
            const int sym = key12(e.key.keysym.sym);
            const bool down = e.type == SDL_KEYDOWN;
            if (g_keystate && sym > 0 && sym < kSdlkLast) rt_mem[g_keystate + (uint32_t)sym] = down;
            g_mod = mod12(e.key.keysym.mod);
            out[0] = down ? 2 : 3;
            out[2] = down ? 1 : 0;
            out[4] = (uint8_t)e.key.keysym.scancode;
            memcpy(out + 8, &sym, 4);
            memcpy(out + 12, &g_mod, 4);
            emit = sym != 0;
            break;
        }
        case SDL_JOYAXISMOTION: {
            const int j = joy_index_of(e.jaxis.which);
            if (j < 0 || is_virtual(j)) break;
            out[0] = 7; out[1] = (uint8_t)j; out[2] = e.jaxis.axis;
            memcpy(out + 4, &e.jaxis.value, 2); emit = true; break;
        }
        case SDL_JOYHATMOTION: {
            const int j = joy_index_of(e.jhat.which);
            if (j < 0 || is_virtual(j)) break;
            out[0] = 9; out[1] = (uint8_t)j; out[2] = e.jhat.hat; out[3] = e.jhat.value;
            emit = true; break;
        }
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP: {
            const int j = joy_index_of(e.jbutton.which);
            if (j < 0 || is_virtual(j)) break;
            out[0] = e.type == SDL_JOYBUTTONDOWN ? 10 : 11; out[1] = (uint8_t)j;
            out[2] = e.jbutton.button; out[3] = e.jbutton.state; emit = true; break;
        }
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP: {
            const int j = joy_index_of(e.cbutton.which);
            if (!is_virtual(j)) break;
            const bool down = e.type == SDL_CONTROLLERBUTTONDOWN;
            out[1] = (uint8_t)j;
            if (const uint8_t bit = pad_hat_bit(e.cbutton.button)) {
                g_pad_hat = down ? (g_pad_hat | bit) : (g_pad_hat & ~bit);
                out[0] = 9; out[2] = 0; out[3] = g_pad_hat; emit = true;
            } else if (const int b = pad_button(e.cbutton.button); b >= 0) {
                out[0] = down ? 10 : 11; out[2] = (uint8_t)b; out[3] = down; emit = chord_filter(out);
                static const bool trace = getenv("RECOMP_TRACE_STATES") != nullptr;
                if (trace) fprintf(stderr, "pad: SDL button %d -> joystick button %d %s%s\n", e.cbutton.button, b,
                                   down ? "down" : "up", emit ? "" : " (held back)");
            }
            break;
        }
        case SDL_CONTROLLERAXISMOTION: {
            const int j = joy_index_of(e.caxis.which);
            if (!is_virtual(j)) break;
            out[1] = (uint8_t)j;
            if (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                // The hat and axes 0/1 share the game's direction state: only
                // forward zone changes (centre/negative/positive), so stick
                // jitter does not cancel a held D-pad direction.
                const int ax = e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX ? 0 : 1;
                const int8_t zone = e.caxis.value < -8000 ? -1 : e.caxis.value > 8000 ? 1 : 0;
                if (zone != g_last_zone[ax]) {
                    g_last_zone[ax] = zone;
                    const int16_t v = (int16_t)(zone * 32767);
                    out[0] = 7; out[2] = (uint8_t)ax;
                    memcpy(out + 4, &v, 2); emit = true;
                }
            } else if (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                const int t = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT;  // ZR: slide (b2), ZL: b7
                const uint8_t pressed = e.caxis.value > 16384;
                if (pressed != g_pad_trig[t]) {
                    g_pad_trig[t] = pressed;
                    out[0] = pressed ? 10 : 11; out[2] = t ? 2 : 7; out[3] = pressed; emit = chord_filter(out);
                }
            }
            break;
        }
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED || e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                out[0] = 1; out[1] = e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED; out[2] = 2; emit = true;
            }
            break;
        default:
            break;
        }
        if (emit) {
            if (ev) memcpy(rt_mem + ev, out, sizeof(out));
            CRET(1);
        }
    }
    CRET(0);
}
SDLX(SDL_WaitEvent) { imp_SDL__SDL_PollEvent(c); }
SDLX(SDL_PeepEvents) { CRET(0); }
SDLX(SDL_PushEvent) { CRET(0); }
SDLX(SDL_EventState) { CRET(1); }
SDLX(SDL_GetKeyState) {
    if (!g_keystate) g_keystate = rt_malloc(kSdlkLast);
    if (ARG(0)) rt_wr32(ARG(0), kSdlkLast);
    CRET(g_keystate);
}
SDLX(SDL_GetModState) { CRET(g_mod); }
SDLX(SDL_EnableUNICODE) { CRET(0); }
SDLX(SDL_EnableKeyRepeat) { CRET(0); }
SDLX(SDL_GetMouseState) { if (ARG(0)) rt_wr32(ARG(0), 0); if (ARG(1)) rt_wr32(ARG(1), 0); CRET(0); }
SDLX(SDL_GetAppState) { CRET(7); }

// ---------------------------------------------------------------- timing
SDLX(SDL_GetTicks) {
    using namespace std::chrono;
    CRET((uint32_t)duration_cast<milliseconds>(steady_clock::now() - g_t0).count());
}
SDLX(SDL_Delay) {
    pump_audio();
    SDL_Delay(ARG(0));
    CRET(0);
}

// ---------------------------------------------------------------- joysticks
SDLX(SDL_NumJoysticks) { CRET(SDL_NumJoysticks()); }
SDLX(SDL_JoystickOpen) {
    const int i = (int)ARG(0);
    if ((size_t)i >= g_joys.size()) g_joys.resize((size_t)i + 1, nullptr);
    if (!g_joys[(size_t)i]) {
        if (virtual_pad_enabled() && SDL_IsGameController(i)) {
            if ((size_t)i >= g_pads.size()) g_pads.resize((size_t)i + 1, nullptr);
            g_pads[(size_t)i] = SDL_GameControllerOpen(i);
            if (g_pads[(size_t)i]) g_joys[(size_t)i] = SDL_GameControllerGetJoystick(g_pads[(size_t)i]);
            fprintf(stderr, "recomp: joystick %d (%s) as the virtual pad\n", i, SDL_GameControllerNameForIndex(i));
        }
        if (!g_joys[(size_t)i]) g_joys[(size_t)i] = SDL_JoystickOpen(i);
    }
    CRET(g_joys[(size_t)i] ? joy_handle(i) : 0);
}
SDLX(SDL_JoystickClose) { CRET(0); }
SDLX(SDL_JoystickUpdate) { SDL_JoystickUpdate(); CRET(0); }
SDLX(SDL_JoystickEventState) { CRET(SDL_JoystickEventState((int)ARG(0))); }
SDLX(SDL_JoystickName) { CRET(0); }
SDLX(SDL_JoystickOpened) { CRET(joy_of(joy_handle((int)ARG(0))) != nullptr); }
SDLX(SDL_JoystickIndex) { CRET(ARG(0) - 0x7f000000u); }
SDLX(SDL_JoystickNumAxes) { SDL_Joystick* j = joy_of(ARG(0)); CRET(j ? SDL_JoystickNumAxes(j) : 0); }
SDLX(SDL_JoystickNumButtons) { SDL_Joystick* j = joy_of(ARG(0)); CRET(j ? SDL_JoystickNumButtons(j) : 0); }
SDLX(SDL_JoystickNumHats) { SDL_Joystick* j = joy_of(ARG(0)); CRET(j ? SDL_JoystickNumHats(j) : 0); }
SDLX(SDL_JoystickGetAxis) { SDL_Joystick* j = joy_of(ARG(0)); CRET(j ? (uint32_t)(int32_t)SDL_JoystickGetAxis(j, (int)ARG(1)) : 0); }
SDLX(SDL_JoystickGetButton) { SDL_Joystick* j = joy_of(ARG(0)); CRET(j ? SDL_JoystickGetButton(j, (int)ARG(1)) : 0); }
SDLX(SDL_JoystickGetHat) { SDL_Joystick* j = joy_of(ARG(0)); CRET(j ? SDL_JoystickGetHat(j, (int)ARG(1)) : 0); }

// ---------------------------------------------------------------- audio
SDLX(SDL_OpenAudio) {
    const uint32_t want = ARG(0), have = ARG(1);
    // SDL 1.2 SDL_AudioSpec: freq(4) format(2) channels(1) silence(1) samples(2)
    // padding(2) size(4) callback(4) userdata(4).
    SDL_AudioSpec spec{};
    spec.freq = (int)rt_rd32(want);
    uint16_t fmt, samples;
    memcpy(&fmt, rt_mem + want + 4, 2);
    memcpy(&samples, rt_mem + want + 10, 2);
    spec.format = fmt;
    spec.channels = rt_mem[want + 6];
    spec.samples = samples;
    g_audio = SDL_OpenAudioDevice(nullptr, 0, &spec, &g_audio_spec, 0);
    if (!g_audio) CRET((uint32_t)-1);
    g_audio_cb = rt_rd32(want + 16);
    g_audio_ud = rt_rd32(want + 20);
    g_audio_len = g_audio_spec.size;
    g_audio_buf = rt_malloc(g_audio_len);
    const uint32_t out = have ? have : want;
    memcpy(rt_mem + out, rt_mem + want, 24);
    rt_mem[out + 7] = g_audio_spec.silence;
    rt_wr32(out + 12, g_audio_spec.size);
    CRET(0);
}
SDLX(SDL_CloseAudio) { if (g_audio) SDL_CloseAudioDevice(g_audio); g_audio = 0; CRET(0); }
SDLX(SDL_PauseAudio) {
    g_audio_paused = ARG(0) != 0;
    if (g_audio) SDL_PauseAudioDevice(g_audio, g_audio_paused);
    CRET(0);
}
SDLX(SDL_LockAudio) { CRET(0); }
SDLX(SDL_UnlockAudio) { CRET(0); }
SDLX(SDL_GetAudioStatus) { CRET(g_audio ? (g_audio_paused ? 2 : 1) : 0); }
SDLX(SDL_AudioDriverName) { CRET(0); }

// ---------------------------------------------------------------- threads (single guest thread)
SDLX(SDL_CreateThread) { rt_fault(0, "SDL_CreateThread (guest threads not supported)"); }
SDLX(SDL_WaitThread) { CRET(0); }
SDLX(SDL_KillThread) { CRET(0); }
SDLX(SDL_ThreadID) { CRET(0x1004); }
SDLX(SDL_CreateMutex) { CRET(rt_malloc(8)); }
SDLX(SDL_DestroyMutex) { CRET(0); }
SDLX(SDL_mutexP) { CRET(0); }
SDLX(SDL_mutexV) { CRET(0); }
SDLX(SDL_CreateSemaphore) { CRET(rt_malloc(8)); }
SDLX(SDL_DestroySemaphore) { CRET(0); }
SDLX(SDL_SemPost) { CRET(0); }
SDLX(SDL_SemWait) { CRET(0); }
SDLX(SDL_SemTryWait) { CRET(0); }
SDLX(SDL_SemWaitTimeout) { CRET(0); }
SDLX(SDL_SemValue) { CRET(0); }
SDLX(SDL_CreateCond) { CRET(rt_malloc(8)); }
SDLX(SDL_DestroyCond) { CRET(0); }
SDLX(SDL_CondSignal) { CRET(0); }
SDLX(SDL_CondBroadcast) { CRET(0); }
SDLX(SDL_CondWait) { CRET(0); }
SDLX(SDL_CondWaitTimeout) { CRET(0); }
SDLX(SDL_AddTimer) { CRET(0); }
SDLX(SDL_RemoveTimer) { CRET(0); }
SDLX(SDL_SetTimer) { CRET(0); }

// ---------------------------------------------------------------- OPENGL32 (stdcall)
GLX(glClearColor) { for (int k = 0; k < 4; ++k) g_clear[k] = ARGF((unsigned)k); SRET(0, 4); }
GLX(glClear) {
    if (g_renderer) {
        SDL_SetRenderDrawColor(g_renderer, (Uint8)(g_clear[0] * 255), (Uint8)(g_clear[1] * 255),
                               (Uint8)(g_clear[2] * 255), 255);
        SDL_RenderClear(g_renderer);
    }
    SRET(0, 1);
}
GLX(glViewport) { if (gl_trace("glViewport")) fprintf(stderr, "gl glViewport %d %d %d %d\n", (int)ARG(0),(int)ARG(1),(int)ARG(2),(int)ARG(3));
    for (int k = 0; k < 4; ++k) g_vp[k] = (int)ARG((unsigned)k);
    present_logical_size();
    SRET(0, 4);
}
GLX(glMatrixMode) { g_matrix_mode = (int)ARG(0); SRET(0, 1); }
GLX(glLoadIdentity) { identity(cur_matrix()); SRET(0, 0); }
GLX(glOrtho) { if (gl_trace("glOrtho")) fprintf(stderr, "gl glOrtho %f %f %f %f\n", rt_argd(c,0),rt_argd(c,2),rt_argd(c,4),rt_argd(c,6));
    const double l = rt_argd(c, 0), r = rt_argd(c, 2), b = rt_argd(c, 4), t = rt_argd(c, 6),
                 n = rt_argd(c, 8), f = rt_argd(c, 10);
    float m[16];
    identity(m);
    m[0] = (float)(2.0 / (r - l));
    m[5] = (float)(2.0 / (t - b));
    m[10] = (float)(-2.0 / (f - n));
    m[12] = (float)(-(r + l) / (r - l));
    m[13] = (float)(-(t + b) / (t - b));
    m[14] = (float)(-(f + n) / (f - n));
    mul(cur_matrix(), m);
    SRET(0, 12);
}
GLX(glEnable) { if (gl_trace("glEnable")) fprintf(stderr, "gl glEnable %x\n", ARG(0)); if (ARG(0) == 0x0de1) g_texture2d = true; if (ARG(0) == 0x0be2) g_blend = true; SRET(0, 1); }
GLX(glFlush) { SRET(0, 0); }
GLX(glGenTextures) {
    pump_audio_soon();
    for (uint32_t k = 0; k < ARG(0); ++k) rt_wr32(ARG(1) + 4 * k, g_next_tex++);
    SRET(0, 2);
}
GLX(glDeleteTextures) {
    pump_audio_soon();
    for (uint32_t k = 0; k < ARG(0); ++k) {
        auto it = g_tex.find(rt_rd32(ARG(1) + 4 * k));
        if (it != g_tex.end()) {
            if (it->second.tex) SDL_DestroyTexture(it->second.tex);
            g_tex.erase(it);
        }
    }
    SRET(0, 2);
}
GLX(glBindTexture) { if (gl_trace("glBindTexture")) fprintf(stderr, "gl glBindTexture %x %u\n", ARG(0),ARG(1)); g_bound = ARG(1); SRET(0, 2); }
GLX(glTexParameteri) {
    const uint32_t pname = ARG(1), value = ARG(2);
    GlTex& t = g_tex[g_bound];
    if (pname == 0x2800 || pname == 0x2801) { // MAG/MIN filter
        t.linear = value == 0x2601;
        if (t.tex) SDL_SetTextureScaleMode(t.tex, t.linear ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
    }
    SRET(0, 3);
}
GLX(glTexImage2D) { pump_audio_soon(); if (gl_trace("glTexImage2D")) fprintf(stderr, "gl glTexImage2D tgt=%x lvl=%u ifmt=%x %ux%u fmt=%x type=%x px=%x\n", ARG(0),ARG(1),ARG(2),ARG(3),ARG(4),ARG(6),ARG(7),ARG(8));
    const int w = (int)ARG(3), h = (int)ARG(4);
    const uint32_t format = ARG(6), pixels = ARG(8);
    {
        static int uploads = 0;
        static const char* dump = getenv("RECOMP_DUMPTEX");
        static const int dump_at = getenv("RECOMP_DUMPTEX_AT") ? atoi(getenv("RECOMP_DUMPTEX_AT")) : 600;
        static const int every = getenv("RECOMP_DUMPTEX_EVERY") ? atoi(getenv("RECOMP_DUMPTEX_EVERY")) : 0;
        ++uploads;
        if (dump && pixels && (uploads == dump_at || (every && uploads % every == 0))) {
            SDL_Surface* sf = SDL_CreateRGBSurfaceWithFormatFrom(rt_mem + pixels, w, h, 32, w * 4, SDL_PIXELFORMAT_ARGB8888);
            char name[512];
            snprintf(name, sizeof(name), every ? "%s_%05d.bmp" : "%s", dump, uploads);
            if (sf) { SDL_SaveBMP(sf, name); SDL_FreeSurface(sf); }
            fprintf(stderr, "recomp: dumped texture upload #%d (%dx%d fmt %x) to %s\n", uploads, w, h, format, dump);
        }
    }
    GlTex& t = g_tex[g_bound];
    if (g_renderer && (!t.tex || t.w != w || t.h != h)) {
        if (t.tex) SDL_DestroyTexture(t.tex);
        t.tex = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
        SDL_SetTextureBlendMode(t.tex, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(t.tex, t.linear ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
        t.w = w;
        t.h = h;
    }
    if (t.tex && pixels) {
        // GL_BGRA (0x80e1) bytes B,G,R,A == ARGB8888 little-endian; GL_RGBA swaps R/B.
        void* dst;
        int pitch;
        if (SDL_LockTexture(t.tex, nullptr, &dst, &pitch) == 0) {
            const uint8_t* src = rt_mem + pixels;
            for (int y = 0; y < h; ++y) {
                uint8_t* d = (uint8_t*)dst + (size_t)y * (size_t)pitch;
                const uint8_t* s = src + (size_t)y * (size_t)w * 4;
                if (format == 0x80e1) {
                    memcpy(d, s, (size_t)w * 4);
                } else if (format == 0x1908) {
                    for (int x = 0; x < w; ++x) {
                        d[4 * x + 0] = s[4 * x + 2]; d[4 * x + 1] = s[4 * x + 1];
                        d[4 * x + 2] = s[4 * x + 0]; d[4 * x + 3] = s[4 * x + 3];
                    }
                } else { // GL_RGB / GL_BGR
                    for (int x = 0; x < w; ++x) {
                        const uint8_t* p = src + ((size_t)y * (size_t)w + (size_t)x) * 3;
                        const bool bgr = format == 0x80e0;
                        d[4 * x + 0] = bgr ? p[0] : p[2]; d[4 * x + 1] = p[1];
                        d[4 * x + 2] = bgr ? p[2] : p[0]; d[4 * x + 3] = 255;
                    }
                }
            }
            // Switch 2 wording for keyboard-only prompts (RECOMP_SWITCH_TEXT previews it on desktop).
#if defined(__SWITCH__)
            constexpr bool switch_text = true;
#else
            static const bool switch_text = getenv("RECOMP_SWITCH_TEXT") != nullptr;
#endif
            if (switch_text) {
                rt_patch_title_prompt((uint8_t*)dst, pitch, w, h);
                if (joymap_active()) rt_patch_control_info((uint8_t*)dst, pitch, w, h);
            }
            SDL_UnlockTexture(t.tex);
        }
    }
    SRET(0, 9);
}
GLX(glBegin) { if (gl_trace("glBegin")) fprintf(stderr, "gl glBegin %x\n", ARG(0)); g_prim = (int)ARG(0); g_verts.clear(); SRET(0, 1); }
GLX(glTexCoord2f) { if (gl_trace("glTexCoord2f")) fprintf(stderr, "gl glTexCoord2f %f %f\n", ARGF(0), ARGF(1)); g_tc[0] = ARGF(0); g_tc[1] = ARGF(1); SRET(0, 2); }
GLX(glVertex3f) { if (gl_trace("glVertex3f")) fprintf(stderr, "gl glVertex3f %f %f %f\n", ARGF(0),ARGF(1),ARGF(2));
    float v[4] = {ARGF(0), ARGF(1), ARGF(2), 1.0f};
    float mv[16], o[4];
    memcpy(mv, g_proj, sizeof(mv));
    mul(mv, g_model);
    for (int r = 0; r < 4; ++r)
        o[r] = mv[r] * v[0] + mv[4 + r] * v[1] + mv[8 + r] * v[2] + mv[12 + r] * v[3];
    SDL_Vertex vert{};
    vert.position.x = (o[0] / o[3] * 0.5f + 0.5f) * (float)kNativeW;   // native target (present_frame)
    vert.position.y = (1.0f - (o[1] / o[3] * 0.5f + 0.5f)) * (float)kNativeH;
    vert.color = SDL_Color{255, 255, 255, 255};
    vert.tex_coord.x = g_tc[0];
    vert.tex_coord.y = g_tc[1];
    g_verts.push_back(vert);
    SRET(0, 3);
}
GLX(glEnd) {
    if (g_renderer && g_prim == 7 /* GL_QUADS */) {
        SDL_Texture* tex = nullptr;
        auto it = g_tex.find(g_bound);
        if (g_texture2d && it != g_tex.end()) tex = it->second.tex;
        if (tex) SDL_SetTextureBlendMode(tex, g_blend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
        std::vector<int> idx;
        for (size_t q = 0; q + 3 < g_verts.size(); q += 4) {
            const int b = (int)q;
            idx.insert(idx.end(), {b, b + 1, b + 2, b, b + 2, b + 3});
        }
        SDL_RenderGeometry(g_renderer, tex, g_verts.data(), (int)g_verts.size(), idx.data(), (int)idx.size());
    }
    g_verts.clear();
    g_prim = -1;
    SRET(0, 0);
}
GLX(wglGetProcAddress) { SRET(0, 1); }

// ---------------------------------------------------------------- USER32 (stdcall)
U32X(ShowCursor) { SDL_ShowCursor(ARG(0) ? SDL_ENABLE : SDL_DISABLE); SRET(ARG(0) ? 0 : (uint32_t)-1, 1); }
U32X(GetSystemMetrics) {
    SDL_DisplayMode dm{};
    SDL_GetDesktopDisplayMode(0, &dm);
    switch (ARG(0)) {
    case 0: SRET(dm.w ? dm.w : 1920, 1);
    case 1: SRET(dm.h ? dm.h : 1080, 1);
    default: SRET(0, 1);
    }
}
