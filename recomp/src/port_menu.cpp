// Port menu (Dear ImGui), full screen in the style of Shipwright's.
//
// Controls: the L Button and X together toggle it (F1 on a keyboard). ZL / ZR change tabs (so does the touch screen, or the D-pad +
// A on the tab list); D-pad left / right also moves between the tab list and
// the page; A (Nintendo right) selects, B goes back. The touch screen works as a pointer, and number fields
// open the Switch software keyboard once per activation.
// Tabs: Settings (every game option, screen shader + colour filter, live shader
// tuning), Audio, Cheats, Save Editor; Quit Game (with a confirmation).
// Game data addresses are in port.h; see docs/recompilation.md "Port menu".
#include "port.h"

#include <atomic>
#include <thread>

#include "recomp/runtime.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(__SWITCH__)
#include <switch.h>
#endif

extern "C" uint32_t rt_guest_call(uint32_t va, unsigned nargs, const uint32_t* args);
extern "C" uint32_t rt_find_export(const char* module, const char* name);
extern "C" int rt_postfx_mode(void);
extern "C" int rt_postfx_count(void);
extern "C" const char* rt_postfx_mode_label(int n);
extern "C" void rt_postfx_set(int n);
extern "C" int rt_postfx_filter(void);
extern "C" int rt_postfx_filter_count(void);
extern "C" const char* rt_postfx_filter_label(int n);
extern "C" void rt_postfx_set_filter(int n);
extern "C" int rt_postfx_param_count(void);
extern "C" void rt_postfx_param_info(int i, const char** desc, float* lo, float* hi, float* step);
extern "C" float rt_postfx_param_get(int i);
extern "C" void rt_postfx_param_set(int i, float v);
extern "C" void rt_postfx_param_reset(void);
void rt_save_port_settings();

PortCheats g_cheats;
uint32_t g_player = 0;
unsigned g_player_frame = 0;

namespace {

using Clock = std::chrono::steady_clock;

SDL_Renderer* g_renderer = nullptr;
SDL_Window* g_window = nullptr;
bool g_ready = false, g_open = false;
bool g_shader_edit = false;  // menu hidden, only the shader panel over the game
bool g_quit_prompt = false;
Clock::time_point g_last_input = Clock::now();
float g_master = 1.0f;
int g_tab = 0;
int g_slot = -1;  // -1: the slot being played (live copy), 0..: SavedGame.slots[n]
bool g_l_held = false, g_x_held = false, g_x_eaten = false;
// D-pad left/right between the tab list and the page: when ImGui's own
// navigation did not move (nothing further that way), jump to the other side.
ImGuiID g_tab_ids[4] = {};
ImGuiWindow* g_tabs_window = nullptr;
ImGuiWindow* g_page_window = nullptr;
int g_jump_dir = 0, g_jump_wait = 0;
ImGuiID g_jump_from = 0;
bool g_zl = false, g_zr = false;

constexpr int kTabs = 4;
const char* kTabNames[kTabs] = {"Settings", "Audio", "Cheats", "Save Editor"};

int g_buttons_held = 0;  // a held button / tilted stick counts as input (shader panel)
bool g_axis_held = false;
float idle_seconds() {
    if (g_buttons_held > 0 || g_axis_held) return 0.0f;
    return std::chrono::duration<float>(Clock::now() - g_last_input).count();
}

// ImGui works in output pixels (HiDPI desktop windows: points x 2).
float pixel_ratio() {
    int ww = 0, ow = 0, oh = 0;
    SDL_GetWindowSize(g_window, &ww, nullptr);
    SDL_GetRendererOutputSize(g_renderer, &ow, &oh);
    return ww > 0 ? (float)ow / (float)ww : 1.0f;
}

// ---- guest memory helpers
uint32_t save_mgr() { return rt_rd32(mmu::kSaveMgrPtr); }
uint32_t options() { return rt_rd32(mmu::kOptionsPtr); }
int32_t rd_i(uint32_t a) { return (int32_t)rt_rd32(a); }
void wr_i(uint32_t a, int32_t v) { rt_wr32(a, (uint32_t)v); }
double rd_d(uint32_t a) { double v; std::memcpy(&v, rt_mem + a, 8); return v; }
void wr_d(uint32_t a, double v) { std::memcpy(rt_mem + a, &v, 8); }
void set_has_bit(uint32_t msg, uint32_t has_off, int bit) {
    if (bit < 0) return;
    const uint32_t a = msg + has_off + 4u * (uint32_t)(bit / 32);
    rt_wr32(a, rt_rd32(a) | (1u << (bit % 32)));
}

// The live player only counts if seen recently (it is gone between stages).
uint32_t player() { return g_player && rt_frame_count() - g_player_frame < 30 ? g_player : 0; }

float weapon_energy(int index) {
    const uint32_t sm = save_mgr();
    if (!sm) return 0;
    rt_cpu.ecx = sm;
    const uint32_t arg = (uint32_t)index;
    rt_guest_call(mmu::kWeaponEnergyGet, 1, &arg);
    Cpu* c = &rt_cpu;
    const double v = c->st[c->top & 7u];
    c->top = (c->top + 1u) & 7u;  // pop the returned float
    return (float)v;
}
void set_weapon_energy(int index, float v) {
    const uint32_t sm = save_mgr();
    if (!sm) return;
    rt_cpu.ecx = sm;
    uint32_t args[2] = {(uint32_t)index, 0};
    std::memcpy(&args[1], &v, 4);
    rt_guest_call(mmu::kWeaponEnergySet, 2, args);
}
float max_weapon_energy() {
    float v;
    std::memcpy(&v, rt_mem + mmu::kMaxWeaponEnergy, 4);
    return v > 0 && v < 1000 ? v : 28.0f;
}
void mixer(const char* name, unsigned n, const uint32_t* args) {
    if (const uint32_t f = rt_find_export("SDL_mixer.dll", name)) rt_guest_call(f, n, args);
}

// ---- Switch software keyboard for number fields: opens once when the text
// field itself becomes active (not its - / + buttons, and not every frame it
// stays active), then deactivates it. `label` is the InputInt/InputDouble label
// (InputScalar's text field is PushID(label) + "").
bool number_keyboard(double* value, bool integer, const char* label) {
    const char* header = label;
#if defined(__SWITCH__)
    ImGui::PushID(label);
    const ImGuiID text_id = ImGui::GetID("");
    ImGui::PopID();
    ImGuiContext& g = *GImGui;
    if (g.ActiveId != text_id || !g.ActiveIdIsJustActivated) return false;
    SwkbdConfig kbd;
    if (R_FAILED(swkbdCreate(&kbd, 0))) return false;
    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetType(&kbd, SwkbdType_NumPad);
    swkbdConfigSetHeaderText(&kbd, header);
    char init[32], out[32] = {0};
    std::snprintf(init, sizeof init, integer ? "%.0f" : "%.1f", *value);
    swkbdConfigSetInitialText(&kbd, init);
    // swkbdShow blocks until the keyboard closes. Run it on a helper thread
    // and keep feeding the audio (pulled on this thread) meanwhile, so the
    // music does not stop while typing.
    std::atomic<bool> done{false};
    Result rc = 0;
    std::thread shown([&] { rc = swkbdShow(&kbd, out, sizeof out); done = true; });
    while (!done) {
        rt_pump_audio();
        svcSleepThread(4000000ull);  // 4 ms
    }
    shown.join();
    swkbdClose(&kbd);
    ImGui::ClearActiveID();
    if (R_FAILED(rc) || !out[0]) return false;
    *value = std::atof(out);
    return true;
#else
    (void)value; (void)integer; (void)header;
    return false;
#endif
}
bool input_int(const char* label, int* v, int lo, int hi) {
    bool changed = ImGui::InputInt(label, v);
    double d = *v;
    if (number_keyboard(&d, true, label)) { *v = (int)d; changed = true; }
    if (changed) *v = std::clamp(*v, lo, hi);
    return changed;
}

// ---- save layout (offset tables in MMU.exe: SavedGame 0x697230, SavedGameSlot 0x6972e0)
enum Kind { INT, BOOL, DOUBLE };
constexpr uint32_t kYoku = 8, kBeat = 12, kEddie = 16, kSpike = 20, kBalancer = 24, kETanks = 28, kWTanks = 32,
                   kBolts = 36, kSmallE = 40, kLives = 44, kPlayTime = 48, kStory = 68, kSignature = 72,
                   kAcquired = 76, kFilled = 88, kZero = 89, kDifficulty = 92;
int has_bit_of(uint32_t off) {  // field index in SavedGameSlot
    switch (off) {
    case kYoku: return 0; case kBeat: return 1; case kEddie: return 2; case kSpike: return 3;
    case kBalancer: return 4; case kETanks: return 5; case kWTanks: return 6; case kBolts: return 7;
    case kSmallE: return 8; case kLives: return 9; case kPlayTime: return 10; case kStory: return 11;
    case kFilled: return 13; case kSignature: return 15; case kDifficulty: return 16; case kZero: return 17;
    default: return -1;
    }
}
void slot_set(uint32_t slot, uint32_t off, int v) {
    wr_i(slot + off, v);
    set_has_bit(slot, mmu::kSlotHasBits, has_bit_of(off));
}
bool slot_int(uint32_t slot, const char* label, uint32_t off, int lo, int hi) {
    int v = rd_i(slot + off);
    ImGui::PushID((int)off);
    const bool changed = input_int(label, &v, lo, hi);
    ImGui::PopID();
    if (changed) slot_set(slot, off, v);
    return changed;
}
bool slot_bool(uint32_t slot, const char* label, uint32_t off, bool int_field) {
    bool v = int_field ? rd_i(slot + off) != 0 : rt_mem[slot + off] != 0;
    if (!ImGui::Checkbox(label, &v)) return false;
    if (int_field) wr_i(slot + off, v); else rt_mem[slot + off] = v;
    set_has_bit(slot, mmu::kSlotHasBits, has_bit_of(off));
    return true;
}

const char* kWeaponNames[mmu::kWeaponCount] = {
    "Mega Buster", "Jet Missile", "Tank Arsenal", "Nail Shield", "Nitro Blast", "Yoku Attack", "Glue Shot",
    "Rainbow Beam", "Comet Dash", "Yoyo Cutter", "Rush Jet", "Rush Coil", "Water Cannon"};

struct GameField { const char* label; uint32_t off; Kind kind; int bit; };
const GameField kGameFields[] = {
    {"Completed Easy", 24, BOOL, 1}, {"Completed Normal", 25, BOOL, 2}, {"Completed Insta-Death", 26, BOOL, 3},
    {"Completed Easy (Zero)", 109, BOOL, 30}, {"Completed Normal (Zero)", 110, BOOL, 31},
    {"Completed Insta-Death (Zero)", 111, BOOL, 32}, {"Defeated Whirlpool Man", 27, BOOL, 8},
    {"Use charge shot", 108, BOOL, 9},
    {"Endless score", 28, INT, 4}, {"Endless hi-score", 32, INT, 5}, {"Endless boss score", 36, INT, 6},
    {"Endless boss hi-score", 40, INT, 7}, {"Endless score (Zero)", 128, INT, 33},
    {"Endless hi-score (Zero)", 132, INT, 34}, {"Endless boss score (Zero)", 136, INT, 35},
    {"Endless boss hi-score (Zero)", 140, INT, 36}, {"Gauntlet score", 144, INT, 37},
    {"Gauntlet hi-score", 148, INT, 38}, {"Gauntlet score (Zero)", 152, INT, 39},
    {"Gauntlet hi-score (Zero)", 156, INT, 40}, {"Gauntlet play time", 160, DOUBLE, 41},
    {"Gauntlet best time", 168, DOUBLE, 42}, {"Gauntlet best time (Zero)", 176, DOUBLE, 43},
};
const char* kBosses[10] = {"Nail Man", "Tank Man", "Jet Man", "Glue Man", "Trinitro Man", "Rainbow Man",
                           "Yoyo Man", "Comet Woman", "Yoku Man", "Whirlpool Man"};

void game_field(uint32_t sm, const char* label, uint32_t off, Kind kind, int bit) {
    const uint32_t a = sm + off;
    bool changed = false;
    ImGui::PushID((int)off);
    if (kind == BOOL) {
        bool v = rt_mem[a] != 0;
        if (ImGui::Checkbox(label, &v)) { rt_mem[a] = v; changed = true; }
    } else if (kind == INT) {
        int v = rd_i(a);
        if (input_int(label, &v, 0, 2147483647)) { wr_i(a, v); changed = true; }
    } else {
        double v = rd_d(a);
        changed = ImGui::InputDouble(label, &v, 1.0, 60.0, "%.1f s");
        changed |= number_keyboard(&v, false, label);
        if (changed) wr_d(a, std::max(0.0, v));
    }
    if (changed) set_has_bit(sm, mmu::kGameHasBits, bit);
    ImGui::PopID();
}

// ---------------------------------------------------------------- pages
const char* kDisplay[] = {"1X", "2X", "3X", "4X", "FULL", "WIDE"};

void shader_combos() {
    bool changed = false;
    const int shader = rt_postfx_mode();
    if (ImGui::BeginCombo("Screen shader", rt_postfx_mode_label(shader))) {
        for (int k = 0; k < rt_postfx_count(); ++k)
            if (ImGui::Selectable(rt_postfx_mode_label(k), k == shader)) { rt_postfx_set(k); changed = true; }
        ImGui::EndCombo();
    }
    const int filter = rt_postfx_filter();
    if (ImGui::BeginCombo("Colour filter", rt_postfx_filter_label(filter))) {
        for (int k = 0; k < rt_postfx_filter_count(); ++k)
            if (ImGui::Selectable(rt_postfx_filter_label(k), k == filter)) { rt_postfx_set_filter(k); changed = true; }
        ImGui::EndCombo();
    }
    if (changed) rt_save_port_settings();
}

void page_settings() {
    const uint32_t o = options();
    ImGui::SeparatorText("Screen");
    if (o) {
        int display = std::clamp(rd_i(o + 0) - 1, 0, 5);
        if (ImGui::Combo("Display size", &display, kDisplay, 6)) wr_i(o + 0, display + 1);
        bool filtering = rd_i(o + 0xc) != 0;
        if (ImGui::Checkbox("Filtering (smooth scaling)", &filtering)) wr_i(o + 0xc, filtering);
        bool vsync = rd_i(o + 0x18) != 0;
        if (ImGui::Checkbox("VSync", &vsync)) {
            wr_i(o + 0x18, vsync);
            SDL_RenderSetVSync(g_renderer, vsync);
        }
    }
    shader_combos();
    if (ImGui::Button("Adjust shader on the game screen...")) {
        g_shader_edit = true;
        g_last_input = Clock::now();  // show the panel right away
    }
#if !defined(__SWITCH__)
    ImGui::TextDisabled("Screen shaders are drawn on Switch 2 only.");
#endif
    if (!o) return;
    ImGui::SeparatorText("Gameplay");
    bool b = rd_i(o + 0x10) != 0;
    if (ImGui::Checkbox("Down + Jump slides", &b)) wr_i(o + 0x10, b);
    int analog = rd_i(o + 0x14);
    if (ImGui::SliderInt("Analog stick dead zone", &analog, 1000, 30000)) wr_i(o + 0x14, analog);
    b = rd_i(o + 0x1c) != 0;
    if (ImGui::Checkbox("Alternate music", &b)) wr_i(o + 0x1c, b);
    b = rd_i(o + 0x20) != 0;
    if (ImGui::Checkbox("Omega weapons", &b)) wr_i(o + 0x20, b);
    b = rd_i(o + 0x24) != 0;
    if (ImGui::Checkbox("Short cutscenes", &b)) wr_i(o + 0x24, b);
    ImGui::Spacing();
    ImGui::TextWrapped("Game options are written to options.xopts the next time the game saves its options "
                       "(leaving its OPTIONS menu).");
}

void page_audio() {
    int master = (int)std::lround(g_master * 100);
    if (ImGui::SliderInt("Master volume", &master, 0, 100, "%d%%")) g_master = master / 100.0f;
    if (ImGui::IsItemDeactivatedAfterEdit()) rt_save_port_settings();  // not every frame while dragging
    const uint32_t o = options();
    if (!o) return;
    int music = rd_i(o + 4), sfx = rd_i(o + 8);
    if (ImGui::SliderInt("Music volume", &music, 0, 128)) {
        wr_i(o + 4, music);
        const uint32_t v = (uint32_t)music;
        mixer("Mix_VolumeMusic", 1, &v);
    }
    if (ImGui::SliderInt("Sound effects volume", &sfx, 0, 128)) {
        wr_i(o + 8, sfx);
        const uint32_t args[2] = {0xffffffffu, (uint32_t)sfx};
        mixer("Mix_Volume", 2, args);
    }
}

void page_cheats() {
    ImGui::Checkbox("Infinite health", &g_cheats.infinite_health);
    ImGui::Checkbox("Infinite lives", &g_cheats.infinite_lives);
    ImGui::Checkbox("Infinite weapon energy", &g_cheats.infinite_ammo);
    ImGui::Checkbox("Infinite screws", &g_cheats.infinite_bolts);
    ImGui::Checkbox("Infinite E-Tanks", &g_cheats.infinite_e_tanks);
    ImGui::Checkbox("Infinite W-Tanks", &g_cheats.infinite_w_tanks);
    ImGui::Checkbox("Moon Jump (hold Jump to keep rising)", &g_cheats.moon_jump);
    ImGui::Checkbox("Fast knockback (recover instantly when hurt)", &g_cheats.fast_knockback);
    ImGui::Checkbox("Always charged shots (Mega Buster)", &g_cheats.always_charged);
    ImGui::SliderFloat("Jump height", &g_cheats.jump_multiplier, 0.5f, 2.5f, "x%.2f");
    if (ImGui::Button("Reset jump height")) g_cheats.jump_multiplier = 1.0f;
}

void page_save() {
    const uint32_t sm = save_mgr();
    if (!sm) { ImGui::TextDisabled("No save data loaded yet."); return; }
    const uint32_t slots = rt_rd32(sm + 8);
    const int count = rd_i(sm + 12);
    char preview[32];
    std::snprintf(preview, sizeof preview, g_slot < 0 ? "Current game (live)" : "Slot %d", g_slot + 1);
    if (ImGui::BeginCombo("Slot", preview)) {
        if (ImGui::Selectable("Current game (live)", g_slot < 0)) g_slot = -1;
        for (int k = 0; k < count && k < 16; ++k) {
            char name[32];
            std::snprintf(name, sizeof name, "Slot %d", k + 1);
            if (ImGui::Selectable(name, g_slot == k)) g_slot = k;
        }
        ImGui::EndCombo();
    }
    const uint32_t slot = g_slot < 0 ? sm + mmu::kCurrentSlot : (slots ? rt_rd32(slots + 4u * (uint32_t)g_slot) : 0);
    if (!slot) return;
    if (g_slot >= 0) ImGui::TextWrapped("Saved slots are written to savedgame.mmu the next time the game saves.");

    if (g_slot < 0) {
        ImGui::SeparatorText("Mega Man");
        if (const uint32_t p = player()) {
            int hp = rd_i(p + mmu::kPlayerHealth);
            if (ImGui::SliderInt("Health", &hp, 1, 28)) wr_i(p + mmu::kPlayerHealth, hp);
        } else {
            ImGui::TextDisabled("Health: not in a stage");
        }
    }
    ImGui::SeparatorText("Items");
    slot_int(slot, "Extra lives", kLives, 0, 99);
    slot_int(slot, "Screws (bolts)", kBolts, 0, 9999);
    // E-Tanks 0-4. Small E-Tanks 0-3: a fourth makes an E-Tank, going below 0 breaks one.
    slot_int(slot, "E-Tanks", kETanks, 0, 4);
    {
        int small = rd_i(slot + kSmallE), e = rd_i(slot + kETanks);
        ImGui::PushID("small");
        if (input_int("Small E-Tanks", &small, -1, 4)) {
            if (small > 3) { if (e < 4) { ++e; small = 0; } else small = 3; }
            if (small < 0) { if (e > 0) { --e; small = 3; } else small = 0; }
            slot_set(slot, kSmallE, small);
            slot_set(slot, kETanks, e);
        }
        ImGui::PopID();
    }
    slot_int(slot, "W-Tanks", kWTanks, 0, 4);
    slot_int(slot, "Eddie (helper)", kEddie, 0, 9);
    slot_int(slot, "Beat (bird)", kBeat, 0, 9);
    slot_int(slot, "Spike Shields", kSpike, 0, 9);
    slot_bool(slot, "Energy Balancer", kBalancer, true);
    int yoku = rd_i(slot + kYoku);
    bool yoku_changed = false;
    ImGui::TextUnformatted("YOKU letters");
    const char* letters = "YOKU";
    for (int k = 0; k < 4; ++k) {
        const char id[2] = {letters[k], 0};
        bool on = (yoku >> k) & 1;
        ImGui::SameLine();
        if (ImGui::Checkbox(id, &on)) { yoku = on ? yoku | (1 << k) : yoku & ~(1 << k); yoku_changed = true; }
    }
    if (yoku_changed) slot_set(slot, kYoku, yoku);

    ImGui::SeparatorText("Progress");
    const char* diffs[] = {"Easy", "Normal", "Insta-Death"};
    int diff = rd_i(slot + kDifficulty);
    if (diff >= 0 && diff <= 2 && ImGui::Combo("Difficulty", &diff, diffs, 3)) slot_set(slot, kDifficulty, diff);
    slot_int(slot, "Story progress", kStory, 0, 99);
    {
        double t = rd_d(slot + kPlayTime);
        bool changed = ImGui::InputDouble("Play time", &t, 1.0, 60.0, "%.1f s");
        changed |= number_keyboard(&t, false, "Play time");
        if (changed) { wr_d(slot + kPlayTime, std::max(0.0, t)); set_has_bit(slot, mmu::kSlotHasBits, 10); }
    }
    slot_bool(slot, "Slot in use", kFilled, false);
    slot_bool(slot, "Playing as Zero", kZero, false);

    ImGui::SeparatorText("Weapons");
    const uint32_t acquired = rt_rd32(slot + kAcquired);
    const int nacq = rd_i(slot + kAcquired + 4);
    const bool live = g_slot < 0;
    const float emax = max_weapon_energy();
    const int nweap = acquired ? std::min(nacq, mmu::kWeaponCount) : 0;
    if (ImGui::Button("All")) for (int w = 0; w < nweap; ++w) rt_mem[acquired + (uint32_t)w] = 1;
    ImGui::SameLine();
    if (ImGui::Button("None")) for (int w = 1; w < nweap; ++w) rt_mem[acquired + (uint32_t)w] = 0;  // keeps the Mega Buster
    if (live) {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        if (ImGui::Button("No Ammo"))
            for (int w = 1; w < nweap; ++w)
                if (rt_mem[acquired + (uint32_t)w]) set_weapon_energy(w, 0);
        ImGui::SameLine();
        if (ImGui::Button("Maxed Out"))
            for (int w = 0; w < nweap; ++w)
                if (rt_mem[acquired + (uint32_t)w]) set_weapon_energy(w, emax);
    }
    if (ImGui::BeginTable("weapons", live ? 3 : 2, ImGuiTableFlags_SizingStretchProp)) {
        for (int w = 0; w < mmu::kWeaponCount; ++w) {
            ImGui::PushID(w);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(kWeaponNames[w]);
            ImGui::TableNextColumn();
            const bool exists = acquired && w < nacq;
            if (exists) {
                bool have = rt_mem[acquired + (uint32_t)w] != 0;
                if (ImGui::Checkbox("Acquired", &have)) rt_mem[acquired + (uint32_t)w] = have;
            }
            if (live) {
                ImGui::TableNextColumn();
                if (exists && rt_mem[acquired + (uint32_t)w]) {
                    float e = weapon_energy(w);
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::SliderFloat("##energy", &e, 0, emax, "%.0f")) set_weapon_energy(w, e);
                }
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (ImGui::CollapsingHeader("Game progress (all slots)")) {
        for (const GameField& f : kGameFields) game_field(sm, f.label, f.off, f.kind, f.bit);
        ImGui::SeparatorText("Super / Perfect bosses defeated");
        static const uint32_t kPerfect[10] = {84, 88, 92, 96, 100, 104, 112, 116, 120, 124};
        for (int k = 0; k < 10; ++k) {
            char l1[48], l2[48];
            std::snprintf(l1, sizeof l1, "Super %s", kBosses[k]);
            std::snprintf(l2, sizeof l2, "Perfect %s", kBosses[k]);
            game_field(sm, l1, 44 + 4u * (uint32_t)k, INT, 10 + k);
            game_field(sm, l2, kPerfect[k], INT, 20 + k);
        }
    }
}

// ---------------------------------------------------------------- windows
constexpr ImGuiWindowFlags kFull = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

void draw_menu() {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
#if defined(__SWITCH__)
    // Switch: the whole screen, tabs on the left (Shipwright / 2Ship / Starship).
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowBgAlpha(0.96f);
    ImGui::Begin("Mega Man Unlimited##menu", nullptr, kFull);
#else
    // Desktop: a window over the game.
    ImGui::SetNextWindowPos(ImVec2(vp->Size.x * 0.5f, vp->Size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x * 0.88f, vp->Size.y * 0.9f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.94f);
    ImGui::Begin("Mega Man Unlimited##menu", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
#endif
    const float em = ImGui::GetFontSize();
    ImGui::TextUnformatted("MEGA MAN UNLIMITED");
    const char* hint = "ZL/ZR: tab   A: select   B: back   L + X: close";
    const float hint_x = ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize(hint).x;
    if (hint_x > ImGui::CalcTextSize("MEGA MAN UNLIMITED").x + em * 2) ImGui::SameLine(hint_x);
    ImGui::TextDisabled("%s", hint);
    ImGui::Separator();

    ImGui::BeginChild("tabs", ImVec2(vp->Size.x * 0.22f, 0), ImGuiChildFlags_NavFlattened);
    g_tabs_window = ImGui::GetCurrentWindow();
    for (int k = 0; k < kTabs; ++k) {
        if (ImGui::Selectable(kTabNames[k], g_tab == k, 0, ImVec2(0, em * 2.4f))) g_tab = k;
        g_tab_ids[k] = ImGui::GetItemID();
    }
    ImGui::SetCursorPosY(std::max(ImGui::GetCursorPosY(), ImGui::GetContentRegionMax().y - em * 2.4f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    if (ImGui::Button("Quit Game", ImVec2(-1, em * 2.2f))) g_quit_prompt = true;
    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("page", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened);
    g_page_window = ImGui::GetCurrentWindow();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);
    switch (g_tab) {
    case 0: page_settings(); break;
    case 1: page_audio(); break;
    case 2: page_cheats(); break;
    default: page_save(); break;
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();

    // Left/right that ImGui could not use: cross between the tab list and the page.
    if (g_jump_dir && --g_jump_wait <= 0) {
        ImGuiContext& g = *GImGui;
        if (g.NavId == g_jump_from && g.ActiveId == 0) {
            bool on_tabs = false;
            for (ImGuiID id : g_tab_ids) on_tabs |= id != 0 && id == g.NavId;
            if (g_jump_dir < 0 && !on_tabs && g_tabs_window) {
                ImGui::SetFocusID(g_tab_ids[g_tab], g_tabs_window);
                ImGui::SetNavCursorVisible(true);
            } else if (g_jump_dir > 0 && on_tabs && g_page_window) {
                ImGui::FocusWindow(g_page_window);
                ImGui::NavInitWindow(g_page_window, true);
                ImGui::SetNavCursorVisible(true);
            }
        }
        g_jump_dir = 0;
    }

    if (g_quit_prompt) {
        ImGui::OpenPopup("Quit?");
        g_quit_prompt = false;
    }
    ImGui::SetNextWindowPos(ImVec2(vp->Size.x * 0.5f, vp->Size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Quit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextUnformatted("Quit Mega Man Unlimited?");
        ImGui::TextDisabled("Progress since the last save will be lost.");
        ImGui::Spacing();
        if (ImGui::Button("No", ImVec2(em * 7, 0))) ImGui::CloseCurrentPopup();
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Yes, quit", ImVec2(em * 7, 0))) {
            std::fprintf(stderr, "recomp: quit from the port menu\n");
            std::fflush(nullptr);
            std::exit(0);
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

// Shader tuning over the live game: the panel hides after 0.5 s without input
// and comes back on the next button press or touch.
void draw_shader_panel() {
    if (idle_seconds() > 0.5f) return;
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Size.x * 0.5f, vp->Size.y * 0.98f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x * 0.7f, 0));
    ImGui::SetNextWindowBgAlpha(0.85f);
    ImGui::Begin("Shader##panel", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    shader_combos();
    for (int i = 0; i < rt_postfx_param_count(); ++i) {
        const char* desc;
        float lo, hi, step;
        rt_postfx_param_info(i, &desc, &lo, &hi, &step);
        float v = rt_postfx_param_get(i);
        ImGui::PushID(i);
        // Whole-number parameters (mask type, layout) snap; the rest stay free
        // so D-pad tweaks (0.01 per press) are not rounded back to the old value.
        if (ImGui::SliderFloat(desc, &v, lo, hi, step >= 1.0f ? "%.0f" : "%.2f")) {
            if (step >= 1.0f) v = lo + std::round((v - lo) / step) * step;
            rt_postfx_param_set(i, v);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) rt_save_port_settings();
        ImGui::PopID();
    }
    if (ImGui::Button("Reset")) { rt_postfx_param_reset(); rt_save_port_settings(); }
    ImGui::SameLine();
    if (ImGui::Button("Done (B)")) g_shader_edit = false;
    ImGui::PopItemWidth();
    ImGui::End();
}

void apply_scale() {
    // Big and readable on a TV and in handheld: 2x at 720p, 3x at 1080p.
    const float scale = std::max(1.0f, std::floor(ImGui::GetIO().DisplaySize.y / 360.0f * 2.0f) / 2.0f);
    static float applied = 0;
    if (scale == applied) return;
    applied = scale;
    ImGuiStyle& style = ImGui::GetStyle();
    style = ImGuiStyle();
    ImGui::StyleColorsDark();
#if defined(__SWITCH__)
    style.WindowRounding = 0.0f;
#else
    style.WindowRounding = 6.0f;
#endif
    style.FrameRounding = 4.0f;
    style.ScaleAllSizes(scale);
    style.FontScaleMain = scale;
}

void set_open(bool open) {
    if (open == g_open) return;
    g_open = open;
    g_shader_edit = false;
    if (open) rt_release_virtual_pad();  // the game must not keep stale held buttons
}

void step_tab(int d) { g_tab = (g_tab + d + kTabs) % kTabs; }

// Touch drives ImGui's pointer directly (SDL's touch-to-mouse emulation is off).
void touch(const SDL_Event& e) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
    io.AddMousePosEvent(e.tfinger.x * io.DisplaySize.x, e.tfinger.y * io.DisplaySize.y);
    if (e.type == SDL_FINGERDOWN) io.AddMouseButtonEvent(0, true);
    if (e.type == SDL_FINGERUP) {
        io.AddMouseButtonEvent(0, false);
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);  // no hover after the finger lifts
    }
}

}  // namespace

float rt_master_volume() { return g_master; }
void rt_menu_set_master(float v) { g_master = std::clamp(v, 0.0f, 1.0f); }
bool rt_menu_open() { return g_open; }

void rt_menu_init(SDL_Window* window, SDL_Renderer* renderer) {
    if (g_ready) return;
    g_renderer = renderer;
    g_window = window;
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");  // touch is handled in touch()
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigNavSwapGamepadButtons = true;  // A (right) selects, B (bottom) goes back
    io.IniFilename = nullptr;
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    g_ready = true;
    if (const char* t = std::getenv("RECOMP_MENU")) {  // debug: open on a tab (9: shader panel)
        g_open = true;
        g_tab = std::atoi(t);
        if (g_tab == 9) { g_tab = 0; g_shader_edit = true; g_last_input = Clock::now() + std::chrono::seconds(10); }
    }
    if (const char* t = std::getenv("RECOMP_MENU_SLOT")) g_slot = std::atoi(t);
}

bool rt_menu_event(const SDL_Event& e) {
    if (!g_ready) return false;
    if (e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_KEYDOWN || e.type == SDL_FINGERDOWN ||
        e.type == SDL_FINGERMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEMOTION ||
        (e.type == SDL_CONTROLLERAXISMOTION && std::abs(e.caxis.value) > 8000))
        g_last_input = Clock::now();
    if (e.type == SDL_CONTROLLERBUTTONDOWN) ++g_buttons_held;
    if (e.type == SDL_CONTROLLERBUTTONUP && g_buttons_held > 0) {
        --g_buttons_held;
        g_last_input = Clock::now();
    }
    if (e.type == SDL_CONTROLLERAXISMOTION && e.caxis.axis <= SDL_CONTROLLER_AXIS_RIGHTY)
        g_axis_held = std::abs(e.caxis.value) > 8000;

    if (e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP) {
        const bool down = e.type == SDL_CONTROLLERBUTTONDOWN;
        const uint8_t b = e.cbutton.button;
        // The L Button + X (SDL "Y", north) toggles the menu, in either order. An X
        // pressed with L held (or while the menu is open) never reaches the game.
        if (b == SDL_CONTROLLER_BUTTON_Y) {
            if (down) {
                g_x_held = true;
                g_x_eaten = g_l_held || g_open;
                if (g_l_held) { set_open(!g_open); return true; }
            } else {
                g_x_held = false;
            }
            if (g_x_eaten) {
                if (!down) g_x_eaten = false;
                return true;
            }
        }
        if (b == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
            g_l_held = down;
            if (down && g_x_held) set_open(!g_open);
            if (g_open) return true;  // no tab change, no ImGui action
        }
        if (g_open && b == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) return true;
        if (g_open && down && !g_shader_edit &&
            (b == SDL_CONTROLLER_BUTTON_DPAD_LEFT || b == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            g_jump_dir = b == SDL_CONTROLLER_BUTTON_DPAD_LEFT ? -1 : +1;
            g_jump_from = GImGui->NavId;
            g_jump_wait = 2;  // after ImGui has applied (or not) its own move
        }
        // B leaves the shader panel when no combo is open.
        if (g_open && g_shader_edit && down && b == SDL_CONTROLLER_BUTTON_A &&
            !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId)) {
            g_shader_edit = false;
            return true;
        }
    }
    if (g_open && e.type == SDL_CONTROLLERAXISMOTION &&
        (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
        const bool left = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT;
        bool& held = left ? g_zl : g_zr;
        const bool now = e.caxis.value > 16384;
        if (now && !held && !g_shader_edit) step_tab(left ? -1 : +1);
        held = now;
        return true;
    }
    if (e.type == SDL_KEYDOWN && !e.key.repeat && e.key.keysym.sym == SDLK_F1) {
        set_open(!g_open);
        return true;
    }
    if (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP || e.type == SDL_FINGERMOTION) {
        if (g_open) touch(e);
        return g_open;
    }
    SDL_Event scaled = e;
    const float r = pixel_ratio();
    if (e.type == SDL_MOUSEMOTION) { scaled.motion.x = (int)(e.motion.x * r); scaled.motion.y = (int)(e.motion.y * r); }
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        scaled.button.x = (int)(e.button.x * r);
        scaled.button.y = (int)(e.button.y * r);
    }
    ImGui_ImplSDL2_ProcessEvent(&scaled);
    if (!g_open) return false;
    switch (e.type) {
    case SDL_KEYDOWN: case SDL_KEYUP: case SDL_TEXTINPUT:
    case SDL_CONTROLLERBUTTONDOWN: case SDL_CONTROLLERBUTTONUP: case SDL_CONTROLLERAXISMOTION:
    case SDL_JOYBUTTONDOWN: case SDL_JOYBUTTONUP: case SDL_JOYAXISMOTION: case SDL_JOYHATMOTION:
    case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: case SDL_MOUSEWHEEL: case SDL_MOUSEMOTION:
        return true;
    default:
        return false;
    }
}

void rt_menu_draw() {
    if (!g_ready || !g_open) return;
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    int ow = 0, oh = 0;
    SDL_GetRendererOutputSize(g_renderer, &ow, &oh);
    ImGui::GetIO().DisplaySize = ImVec2((float)ow, (float)oh);
    ImGui::GetIO().DisplayFramebufferScale = ImVec2(1, 1);
    apply_scale();
    ImGui::NewFrame();
    if (g_shader_edit) draw_shader_panel(); else draw_menu();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), g_renderer);
}

// Lives / screws / tanks cheats keep the live slot's counts from dropping.
void rt_cheats_tick() {
    const uint32_t sm = save_mgr();
    if (!sm) return;
    const uint32_t slot = sm + mmu::kCurrentSlot;
    struct Keep { bool* on; uint32_t off; int floor; } keep[] = {
        {&g_cheats.infinite_lives, kLives, 9},
        {&g_cheats.infinite_bolts, kBolts, 999},
        {&g_cheats.infinite_e_tanks, kETanks, 4},
        {&g_cheats.infinite_w_tanks, kWTanks, 4},
    };
    for (auto& k : keep)
        if (*k.on && rd_i(slot + k.off) < k.floor) wr_i(slot + k.off, k.floor);
}
