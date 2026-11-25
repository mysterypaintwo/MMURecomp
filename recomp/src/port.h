// Port-side state shared by the ImGui menu (port_menu.cpp), the game hooks
// (patches.cpp) and the SDL shim (imports_sdl.cpp).
#pragma once

#include <SDL.h>
#include <cstdint>

struct PortCheats {
    bool infinite_health = false;
    bool infinite_lives = false;
    bool infinite_ammo = false;
    bool infinite_bolts = false;
    bool infinite_e_tanks = false;
    bool infinite_w_tanks = false;
    bool moon_jump = false;       // hold Jump to keep rising, drop as soon as it is released
    bool fast_knockback = false;  // recover from being hurt on the next frame, no push
    bool always_charged = false;  // every Mega Buster shot is a full charged shot
    float jump_multiplier = 1.0f; // scales the jump velocity GameParam
};
extern PortCheats g_cheats;
extern uint32_t g_player;         // MegaMan object (0x5b02f0 hook), 0 = none seen
extern unsigned g_player_frame;   // frame it was last seen

// ---- game data (addresses from MMU.exe, see docs/recompilation.md "Port changes")
namespace mmu {
constexpr uint32_t kOptionsPtr = 0x6c55d8u;  // -> int[10]: display, music, sfx, filtering, ...
constexpr uint32_t kSaveMgrPtr = 0x6c55a4u;  // -> {SavedGame @0, current SavedGameSlot @0xc8, weapon energy map @0x14c}
constexpr uint32_t kCurrentSlot = 0xc8u;
constexpr uint32_t kSlotHasBits = 100u;      // SavedGameSlot::_has_bits_
constexpr uint32_t kGameHasBits = 0xbcu;     // SavedGame::_has_bits_
constexpr uint32_t kPlayerHealth = 0x150u;   // uint, max 28 (1 on Insta-Death)
constexpr uint32_t kMaxWeaponEnergy = 0x698b48u;  // float, set at startup
constexpr uint32_t kWeaponEnergyGet = 0x420e80u;  // thiscall(savemgr, index) -> float (x87)
constexpr uint32_t kWeaponEnergySet = 0x420eb0u;  // thiscall(savemgr, index, float)
constexpr int kWeaponCount = 13;
}  // namespace mmu

// imports_sdl.cpp
unsigned rt_frame_count();
void rt_pump_audio();  // feed the mixer now (imports_sdl.cpp), e.g. while a blocking applet is open
void rt_release_virtual_pad();  // queue releases for every virtual button/hat/axis
SDL_Texture* rt_game_target();

// native_text.cpp: the game's 8x8 sprite font
bool rt_text_available();
int rt_text_width(const char* s);
void rt_text_draw(SDL_Renderer* r, const char* s, int x, int y, int scale, uint8_t alpha);
bool rt_patch_title_prompt(uint8_t* pixels, int pitch, int w, int h);  // "PRESS ENTER" -> "PRESS +"
bool rt_patch_control_info(uint8_t* pixels, int pitch, int w, int h);  // "[ESC]" -> "[-+ZR]" to skip

// missing_game.cpp: explains where to get / put the game files; returns 1
int rt_missing_game_screen(const char* dir, const char* what);

// port_menu.cpp
void rt_menu_init(SDL_Window* window, SDL_Renderer* renderer);
bool rt_menu_event(const SDL_Event& e);  // true: the menu took this event
void rt_menu_draw();                     // after the game frame, before present
bool rt_menu_open();
void rt_cheats_tick();                   // once per game frame
float rt_master_volume();
void rt_menu_set_master(float v);
