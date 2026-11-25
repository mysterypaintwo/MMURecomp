// SDL2 front-end for the portable MMU reimplementation.
// The same source builds on macOS (brew sdl2-compat) and on Switch 2
// (devkitPro SDL2 portlib + libounce).

#include <SDL.h>

#if defined(__SWITCH__) || defined(__SWITCH2__)
#include <switch.h>
#include <imgui_internal.h>
// libounce exports this as a C symbol; its own header omits extern "C" in C++.
extern "C" unsigned romfsMountSelf(const char* name);
#endif

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include "mmu/core/ActivityMaskReaderTable.h"
#include "mmu/core/ActivityMaskTable.h"
#include "mmu/core/AssetDatabase.h"
#include "mmu/core/BootCutsceneTable.h"
#include "mmu/core/BossDamageParams.h"
#include "mmu/core/CameraEvidenceTable.h"
#include "mmu/core/CutsceneRuntime.h"
#include "mmu/core/ContinueStageTable.h"
#include "mmu/core/CoverageSummary.h"
#include "mmu/core/DamageWrapperTable.h"
#include "mmu/core/DebugConsole.h"
#include "mmu/core/EnemyEdgeFacts.h"
#include "mmu/core/CompletionCutsceneTable.h"
#include "mmu/core/DecompIndex.h"
#include "mmu/core/EnemyStateGraph.h"
#include "mmu/core/GameFlow.h"
#include "mmu/core/GameFlowEvidenceTable.h"
#include "mmu/core/GameFlowRuntime.h"
#include "mmu/core/FlowEdgeFacts.h"
#include "mmu/core/FlowEvidence.h"
#include "mmu/core/GunParams.h"
#include "mmu/core/InlineMenuResources.h"
#include "mmu/core/KeyNameCandidates.h"
#include "mmu/core/KeySweepSummary.h"
#include "mmu/core/MenuActionTable.h"
#include "mmu/core/MenuItemTables.h"
#include "mmu/core/RegisteredCutscenes.h"
#include "mmu/core/RegisteredStateNames.h"
#include "mmu/core/RttiClassNames.h"
#include "mmu/core/RttiVtables.h"
#include "mmu/core/SaveFlagTable.h"
#include "mmu/core/SceneNames.h"
#include "mmu/core/LevelArt.h"
#include "mmu/core/LevelSetupTable.h"
#include "mmu/core/ManagerFieldTable.h"
#include "mmu/core/OutroHandoffTable.h"
#include "mmu/core/PackedArchive.h"
#include "mmu/core/PauseWeaponResources.h"
#include "mmu/core/PlayerControlEvidenceTable.h"
#include "mmu/core/PlayerSpawnEvidence.h"
#include "mmu/core/ProtoIndex.h"
#include "mmu/core/StateSlotBlocks.h"
#include "mmu/core/StageSelectGrid.h"
#include "mmu/core/Types.h"
#include "mmu/core/VerifiedKeyNames.h"
#include "mmu/core/WilyAttackSelector.h"
#include "mmu/core/World.h"
#include "mmu/sdl2/AudioEngine.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifndef MMU_ASSET_DIR
#define MMU_ASSET_DIR "."
#endif

extern SDL_Renderer* g_renderer;
using mmu::u32;

#if defined(__SWITCH__) || defined(__SWITCH2__)
// Shared system font and Swkbd integration, modelled on 2ship2harkinian's
// Switch port. pl stays open while ImGui owns font data pointers from it.
namespace switchui {

namespace {
bool plReady = false;
bool keyboardReady = false;
bool keyboardShowing = false;
// WantTextInput stays set while an InputText keeps focus. Latch the field that
// opened swkbd so + / B closing the applet does not immediately reopen it on
// the next frame, then blur once the submitted text has been consumed.
ImGuiID keyboardFieldId = 0;
bool keyboardArmed = false;
bool keyboardBlurPending = false;
// ImGuiID of the console command field, recorded by drawConsole. When swkbd's
// Send button closes while this field is active, the exact typed text is
// handed to the console instead of being typed into the field.
ImGuiID commandFieldId = 0;
char submittedCommand[256] = {};
bool hasSubmittedCommand = false;
SwkbdConfig keyboard{};
char keyboardInitialText[256] = {};
char keyboardBuffer[256] = {};

void ensurePl() {
    if (plReady) return;
    if (R_SUCCEEDED(plInitialize(PlServiceType_User))) plReady = true;
}

} // namespace

bool addSharedFont(ImFontAtlas* fonts, float sizePixels) {
    ensurePl();
    if (!plReady) return false;

    PlFontData standard{};
    if (R_FAILED(plGetSharedFontByType(&standard, PlSharedFontType_Standard)))
        return false;

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.PixelSnapH = true;
    config.OversampleH = config.OversampleV = 1;
    std::snprintf(config.Name, sizeof(config.Name), "Nintendo Standard");
    fonts->AddFontFromMemoryTTF(standard.address, static_cast<int>(standard.size),
                                sizePixels, &config, fonts->GetGlyphRangesDefault());
    return true;
}

void initKeyboard() {
    if (keyboardReady) return;
    if (R_FAILED(swkbdCreate(&keyboard, 0))) return;
    swkbdConfigMakePresetDefault(&keyboard);
    // Label the + / submit key "Send" so committing the command is explicit.
    swkbdConfigSetOkButtonText(&keyboard, "Send");
    keyboardReady = true;
}

// drawConsole calls this each frame with the command field's ID so swkbd can
// tell the console field apart from other text fields (filters, etc.).
void noteCommandField(ImGuiID id) { commandFieldId = id; }

// Consume a command that swkbd's Send button submitted for the console.
bool takeSubmittedCommand(char* out, std::size_t size) {
    if (!hasSubmittedCommand) return false;
    if (out != nullptr && size != 0)
        std::snprintf(out, size, "%s", submittedCommand);
    hasSubmittedCommand = false;
    submittedCommand[0] = '\0';
    return true;
}

void processTextInput() {
    if (!keyboardReady || keyboardShowing) return;
    ImGuiIO& io = ImGui::GetIO();

    // The previous reply was consumed by the active InputText during the last
    // frame. Drop focus so the field stops advertising WantTextInput forever.
    if (keyboardBlurPending) {
        ImGui::ClearActiveID();
        io.WantTextInput = false;
        keyboardBlurPending = false;
        keyboardArmed = false;
        keyboardFieldId = 0;
        return;
    }

    const ImGuiID active = ImGui::GetActiveID();
    // A different focused item (or none) ends the previous request and re-arms.
    if (keyboardArmed && active != keyboardFieldId) {
        keyboardArmed = false;
        keyboardFieldId = 0;
    }
    if (keyboardArmed || !io.WantTextInput) return;

    ImGuiInputTextState* text = ImGui::GetInputTextState(active);
    if (text == nullptr) return;
    keyboardArmed = true;
    keyboardFieldId = active;

    // Seed the applet with whatever is already in the field; never clear it.
    const char* existing = text->GetText();
    std::snprintf(keyboardInitialText, sizeof(keyboardInitialText), "%s",
                  existing != nullptr ? existing : "");
    swkbdConfigSetInitialText(&keyboard, keyboardInitialText);

    keyboardBuffer[0] = '\0';
    keyboardShowing = true;
    const Result result = swkbdShow(&keyboard, keyboardBuffer, sizeof(keyboardBuffer));
    keyboardShowing = false;

    // Both + (Send) and B close the applet. Ignore that same press on the way
    // out; otherwise the still-focused field reopens the keyboard instantly.
    io.WantTextInput = false;
    keyboardBlurPending = true;

    // Send (+) on the console command field hands the exact typed text to the
    // console for execution. Send on any other field inserts the text normally.
    // Cancel (B) returns a failed Result and leaves the field's saved prompt
    // untouched; swkbdConfigSetInitialText restores it on the next open.
    if (R_SUCCEEDED(result) && keyboardBuffer[0] != '\0') {
        if (active == commandFieldId) {
            std::snprintf(submittedCommand, sizeof(submittedCommand), "%s",
                          keyboardBuffer);
            hasSubmittedCommand = true;
        } else {
            text = ImGui::GetInputTextState(active);
            if (text != nullptr) {
                text->SelectAll();
                io.SetAppAcceptingEvents(true);
                io.AddInputCharactersUTF8(keyboardBuffer);
            }
        }
    }
}

void shutdown() {
    if (keyboardReady) {
        swkbdClose(&keyboard);
        keyboardReady = false;
    }
    if (plReady) {
        plExit();
        plReady = false;
    }
}

} // namespace switchui
#else
// Desktop has no Swkbd applet; the console field is driven by the physical
// keyboard, so the hand-off hooks are inert.
namespace switchui {
void noteCommandField(ImGuiID) {}
bool takeSubmittedCommand(char*, std::size_t) { return false; }
} // namespace switchui
#endif

namespace {

struct SpriteViewerState {
    int selected = -1; // index into PackedArchive::sprites()
    int frame = 0;
    float elapsed = 0.0f;
    float scale = 2.0f;
    bool playing = true;
    bool showCollision = false; // the red debug wash obscures the real colors
    bool open = false;
    char filter[128] = {};
};

struct LevelViewerState {
    int selected = -1; // index into PackedArchive::scenes()
    int hoveredSpawn = -1;
    char filter[128] = {};
    float scale = 2.0f;
    ImVec2 pan{0.0f, 0.0f}; // world coordinate at the canvas top-left
    bool showGrid = true;
    bool showLabels = false;
    bool showImages = true;
    bool showBackground = true;
    bool anchorBottom = true;
    bool open = false;
};

struct GameViewState {
    int sceneIndex = 0;
    bool running = false;
    bool followPlayer = true;
    bool showLabels = false;
    bool showBackground = true;
    float zoom = 2.0f;
    float accumulator = 0.0f;
    bool open = false;
};

// File-scope UI chrome state. The existing viewer visibility flags are split
// between locals in main() and AppState, so these controls are kept together.
struct UiState {
    bool menuBarVisible = true;
    bool quitConfirmOpen = false;
    bool quitPopupRequested = false;
    int quitConfirmSelection = 1; // 0 = Yes, 1 = No; start on the safe choice
    mmu::u16 windowLayoutPending = 0; // bitmask of windows needing defaults

    // ZL+R3 hide/show is rising-edge only, never once per held frame.
    bool prevMenuToggle = false;

    // Raw gamepad state for the quit warning. A/B/Dpad/left stick are captured
    // here while the modal owns input, then consumed instead of reaching the
    // harness flow runtime.
    bool prevUp = false;
    bool prevDown = false;
    bool prevLeft = false;
    bool prevRight = false;
    bool prevConfirm = false;
    bool prevCancel = false;

    // Debug Tools menu. Right-click-drag teleports the player while enabled.
    bool debugDragPlayer = false;
    bool draggingPlayer = false;
    float dragWorldX = 0.0f;
    float dragWorldY = 0.0f;
};

std::string joinAssetPath(const std::string& dir, const std::string& file) {
    if (dir.empty()) return file;
    const char last = dir.back();
    if (last == '/' || last == '\\') return dir + file;
    return dir + "/" + file;
}

// On host the archive lives next to the executable. On Switch 2 it is baked
// into the NRO and mounted at romfs:/. An explicit CLI argument overrides both.
std::string defaultAssetDir() {
#if defined(__SWITCH__) || defined(__SWITCH2__)
    return "romfs:/";
#else
    if (char* base = SDL_GetBasePath()) {
        const std::string dir(base);
        SDL_free(base);
        return dir;
    }
    return MMU_ASSET_DIR;
#endif
}

// ---------------------------------------------------------------------------
// ImGui presentation profile
//
// 2ship2harkinian scales the ImGui style 2x on Switch (720p) and 3x on Switch 2
// (1080p), with a 1.5x font on Switch 2. We follow that model for any touch
// target, and also honour an explicit --touch-ui override on the host. This is
// debug-UI chrome only: it is not the game's own menu/HUD rendering.
// ---------------------------------------------------------------------------
struct UiProfile {
    bool touch = false;
    bool touchLayout = false;
    float styleScale = 1.0f;
    float fontScale = 1.0f;

    const char* name() const { return touch ? "touch" : "desktop"; }
};

UiProfile makeUiProfile(bool forceTouch, bool forceDesktop, int outputHeight) {
#if defined(__SWITCH__) || defined(__SWITCH2__)
    const bool consoleTarget = true;
#else
    const bool consoleTarget = false;
#endif
    const bool touch = forceTouch || (consoleTarget && !forceDesktop);
    UiProfile profile;
    profile.touch = touch;
    profile.touchLayout = touch;
    if (!touch) return profile;

    // 720p -> 2x, 1080p -> 3x, 4K -> 4x (capped so the UI stays usable).
    if (outputHeight >= 2000) {
        profile.styleScale = 4.0f;
        profile.fontScale = 1.5f;
    } else if (outputHeight >= 1000) {
        profile.styleScale = 3.0f;
        profile.fontScale = 1.5f;
    } else {
        profile.styleScale = 2.0f;
        profile.fontScale = 1.0f;
    }
    return profile;
}

void applyUiProfile(const UiProfile& profile) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (profile.touch) {
        style.WindowPadding = ImVec2(8.0f, 7.0f);
        style.FramePadding = ImVec2(8.0f, 5.0f);
        style.ItemSpacing = ImVec2(10.0f, 7.0f);
        style.ItemInnerSpacing = ImVec2(7.0f, 5.0f);
        style.TouchExtraPadding = ImVec2(4.0f, 4.0f);
        style.WindowRounding = 6.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 5.0f;
        style.ScrollbarSize = 16.0f;
        style.GrabMinSize = 20.0f;
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        style.ScaleAllSizes(profile.styleScale);
    } else {
        style.FrameRounding = 3.0f;
        style.GrabMinSize = 10.0f;
    }

    io.Fonts->Clear();
    bool addedSharedFont = false;
#if defined(__SWITCH__) || defined(__SWITCH2__)
    addedSharedFont = switchui::addSharedFont(io.Fonts, 13.0f * profile.fontScale);
#endif
    if (!addedSharedFont) {
        ImFontConfig fontConfig;
        fontConfig.SizePixels = 13.0f * profile.fontScale;
        fontConfig.PixelSnapH = true;
        fontConfig.OversampleH = fontConfig.OversampleV = 1;
        // Select the scalable default explicitly: the implicit heuristic depends
        // on properties that are only final after the first NewFrame(), and
        // would otherwise assert when a console command switches profiles.
        io.Fonts->AddFontDefaultVector(&fontConfig);
    }
    // Do not call Build(): the SDLRenderer2 backend supports dynamic font atlas
    // textures and builds the atlas on the first frame after its Init() call.
    io.FontGlobalScale = 1.0f; // size is baked into the font atlas above

    // The vendored ImGui build is the standard branch (no docking API), so the
    // touch layout relies on the existing menu bar plus large widgets instead.
    // IsTouchScreen changes several interaction thresholds in ImGui itself.
    io.ConfigFlags &= ~(ImGuiConfigFlags_IsTouchScreen |
                        ImGuiConfigFlags_NavEnableGamepad);
    if (profile.touch) {
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    }
}

// Window defaults are fractions of the current renderer output, with minimums
// scaled by the same factor as the ImGui chrome. This keeps the harness usable
// in a 1280x720 desktop window and on the fixed-resolution console targets.
enum WindowLayoutBit : mmu::u16 {
    kWindowSpriteViewer = 1u << 0,
    kWindowLevelViewer = 1u << 1,
    kWindowHarnessView = 1u << 2,
    kWindowAudio = 1u << 3,
    kWindowConsole = 1u << 4,
    kWindowFlowEvidence = 1u << 5,
    kWindowCutsceneRuntime = 1u << 6,
    kWindowInspector = 1u << 7,
    kWindowAll = 0x00ffu,
};

struct WindowLayoutDefaults {
    ImVec2 pos;
    ImVec2 size;
};

WindowLayoutDefaults computeWindowDefaults(mmu::u16 bit, const UiProfile& profile,
                                           int outputWidth, int outputHeight) {
    struct Spec {
        float x, y; // 0 = top/left, 1 = bottom/right, 0.5 = centered
        float w, h; // fractions of the available render area
        float minW, minH;
    };
    Spec spec{0.0f, 0.0f, 0.4f, 0.4f, 320.0f, 240.0f};
    switch (bit) {
    case kWindowConsole:
        spec = {0.00f, 0.00f, 0.42f, 0.46f, 420.0f, 300.0f};
        break;
    case kWindowInspector:
        spec = {1.00f, 0.00f, 0.30f, 0.46f, 300.0f, 300.0f};
        break;
    case kWindowFlowEvidence:
        spec = {0.50f, 0.00f, 0.58f, 0.64f, 600.0f, 420.0f};
        break;
    case kWindowCutsceneRuntime:
        spec = {0.00f, 1.00f, 0.42f, 0.34f, 420.0f, 240.0f};
        break;
    case kWindowAudio:
        spec = {1.00f, 1.00f, 0.42f, 0.34f, 420.0f, 240.0f};
        break;
    case kWindowSpriteViewer:
        spec = {0.00f, 0.48f, 0.34f, 0.52f, 320.0f, 360.0f};
        break;
    case kWindowLevelViewer:
    case kWindowHarnessView:
        spec = {0.50f, 0.00f, 0.68f, 0.76f, 620.0f, 520.0f};
        break;
    default:
        break;
    }

    const float width = static_cast<float>(std::max(1, outputWidth));
    const float height = static_cast<float>(std::max(1, outputHeight));
    const float scale = profile.touch ? profile.styleScale : 1.0f;
    const float marginX = 8.0f * scale;
    const float marginTop = 32.0f * scale;
    const float marginBottom = 8.0f * scale;
    const float availX = std::max(1.0f, width - marginX * 2.0f);
    const float availY = std::max(1.0f, height - marginTop - marginBottom);

    float sizeX = std::max(width * spec.w, spec.minW * scale);
    float sizeY = std::max(availY * spec.h, spec.minH * scale);
    sizeX = std::min(sizeX, availX);
    sizeY = std::min(sizeY, availY);
    const float posX = marginX + spec.x * (availX - sizeX);
    const float posY = marginTop + spec.y * (availY - sizeY);
    return {ImVec2(posX, posY), ImVec2(sizeX, sizeY)};
}

void applyWindowLayout(mmu::u16& pending, mmu::u16 bit, const UiProfile& profile,
                       int outputWidth, int outputHeight) {
    if ((pending & bit) == 0) return;
    const WindowLayoutDefaults layout =
        computeWindowDefaults(bit, profile, outputWidth, outputHeight);
    ImGui::SetNextWindowPos(layout.pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(layout.size, ImGuiCond_Always);
    pending &= static_cast<mmu::u16>(~bit);
}

struct InitialWindowSize {
    int width = 1280;
    int height = 720;
};

InitialWindowSize chooseInitialWindowSize() {
#if defined(__SWITCH__) || defined(__SWITCH2__)
    // Console output is fixed by the target; do not resize the backing window.
    return {1280, 720};
#else
    InitialWindowSize size;
    SDL_DisplayMode mode{};
    if (SDL_GetDesktopDisplayMode(0, &mode) == 0 && mode.w > 0 && mode.h > 0) {
        if (mode.w >= 1600 && mode.h >= 900) {
            size.width = 1600;
            size.height = 900;
        }
        size.width = std::min(size.width, std::max(960, mode.w - 80));
        size.height = std::min(size.height, std::max(540, mode.h - 120));
    }
    return size;
#endif
}

struct AppState {
    mmu::DebugConsole console;
    mmu::AssetDatabase assets;
    mmu::PackedArchive pak;
    std::vector<mmu::u8> pakBytes;
    std::string assetDir = defaultAssetDir();
    UiProfile ui;
    UiState uiState;
    int outputWidth = 1280;
    int outputHeight = 720;
    SpriteViewerState viewer;
    LevelViewerState levelViewer;
    GameViewState gameView;
    // Lowercased spawn type -> baked image entry index, built once per archive.
    mmu::TypeImageMap typeImages;
    std::unordered_map<u32, mmu::SceneArt> sceneArt; // keyed by scene index
    mmu::World world;
    // Separate reconstructed flow core; the harness GameRuntime below is test-only.
    mmu::GameFlowRuntime flowRuntime;
    // Condition sources consumed by KeyedFlagRuntime::evaluate. The manager
    // array is indexed by byte offset, as in FUN_00441820 (id - 0xf9).
    std::array<mmu::u8, mmu::kKeyedFlagKeyboardLimit> keyedKeyboard{};
    std::array<mmu::u8, 0x100> keyedManager{};
    mmu::CutsceneRuntime cutsceneRuntime;
    std::vector<mmu::CutsceneEvent> cutsceneEventLog;
    mmu::sdl2::AudioEngine audio;
    bool quit = false;
    SDL_Texture* preview = nullptr;
    int previewWidth = 0;
    int previewHeight = 0;
};

// Edge-triggered menu visibility. Held ZL+R3 never toggles more than once.
void updateMenuToggle(bool now, bool& visible, bool& prev) {
    if (now && !prev) visible = !visible;
    prev = now;
}

// Switch face buttons use Nintendo labels: A = Face Right, B = Face Down,
// Y = Face Left, X = Face Up. SDL's console controller backend reports these by
// position (Xbox convention), so the physical A/B buttons arrive as the
// opposite SDL face constants. The on-screen prompt names the physical buttons,
// so map them explicitly instead of assuming SDL reports Nintendo labels.
#if defined(__SWITCH__) || defined(__SWITCH2__)
inline constexpr SDL_GameControllerButton kUiConfirmButton = SDL_CONTROLLER_BUTTON_B;
inline constexpr SDL_GameControllerButton kUiCancelButton = SDL_CONTROLLER_BUTTON_A;
#else
inline constexpr SDL_GameControllerButton kUiConfirmButton = SDL_CONTROLLER_BUTTON_A;
inline constexpr SDL_GameControllerButton kUiCancelButton = SDL_CONTROLLER_BUTTON_B;
#endif

// Poll raw controller state once per frame. The ZL+R3 toggle is checked even
// when the quit warning is open; the warning consumes A/B/Dpad/left stick so
// those presses cannot also drive the harness flow runtime or ImGui nav.
void pollUiInput(AppState& state, SDL_GameController* pad) {
    UiState& ui = state.uiState;
    bool up = false, down = false, left = false, right = false;
    bool confirm = false, cancel = false, menuToggle = false;

    if (pad != nullptr) {
        up = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
        down = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
        left = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
        right = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
        confirm = SDL_GameControllerGetButton(pad, kUiConfirmButton) != 0;
        cancel = SDL_GameControllerGetButton(pad, kUiCancelButton) != 0;

        const int axisX = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
        const int axisY = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
        constexpr int kAnalogDeadzone = 8000;
        if (axisY < -kAnalogDeadzone) up = true;
        if (axisY > kAnalogDeadzone) down = true;
        if (axisX < -kAnalogDeadzone) left = true;
        if (axisX > kAnalogDeadzone) right = true;

        // SDL has no ZL button; the left trigger axis is the portable mapping.
        constexpr int kTriggerThreshold = 8000;
        const bool zl = SDL_GameControllerGetAxis(
                            pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > kTriggerThreshold;
        const bool r3 = SDL_GameControllerGetButton(
                            pad, SDL_CONTROLLER_BUTTON_RIGHTSTICK) != 0;
        menuToggle = zl && r3;
    }

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    if (keys != nullptr) {
        up |= keys[SDL_SCANCODE_UP] != 0 || keys[SDL_SCANCODE_W] != 0;
        down |= keys[SDL_SCANCODE_DOWN] != 0 || keys[SDL_SCANCODE_S] != 0;
        left |= keys[SDL_SCANCODE_LEFT] != 0 || keys[SDL_SCANCODE_A] != 0;
        right |= keys[SDL_SCANCODE_RIGHT] != 0 || keys[SDL_SCANCODE_D] != 0;
        confirm |= keys[SDL_SCANCODE_RETURN] != 0 || keys[SDL_SCANCODE_KP_ENTER] != 0;
        cancel |= keys[SDL_SCANCODE_ESCAPE] != 0;
    }

    updateMenuToggle(menuToggle, ui.menuBarVisible, ui.prevMenuToggle);

    if (!ui.quitConfirmOpen) {
        ui.quitPopupRequested = false;
        ui.prevUp = up;
        ui.prevDown = down;
        ui.prevLeft = left;
        ui.prevRight = right;
        ui.prevConfirm = confirm;
        ui.prevCancel = cancel;
        return;
    }

    if ((up && !ui.prevUp) || (left && !ui.prevLeft))
        ui.quitConfirmSelection = 0;
    if ((down && !ui.prevDown) || (right && !ui.prevRight))
        ui.quitConfirmSelection = 1;
    if (confirm && !ui.prevConfirm) {
        if (ui.quitConfirmSelection == 0) state.quit = true;
        ui.quitConfirmOpen = false;
    }
    if (cancel && !ui.prevCancel) ui.quitConfirmOpen = false;

    ui.prevUp = up;
    ui.prevDown = down;
    ui.prevLeft = left;
    ui.prevRight = right;
    ui.prevConfirm = confirm;
    ui.prevCancel = cancel;
}

void logStats(AppState& state) {
    const mmu::AssetStats& s = state.assets.stats();
    char line[256];
    std::snprintf(line, sizeof(line),
                  "sprites=%zu frames=%zu surfaces=%zu pixelBytes=%zu",
                  s.sprites, s.animationFrames, s.surfaces, s.pixelBytes);
    state.console.log(line);
    std::snprintf(line, sizeof(line),
                  "gameObjects=%zu sounds=%zu params=%zu scenes=%zu spawns=%zu",
                  s.gameObjects, s.sounds, s.gameParams, s.scenes, s.spawns);
    state.console.log(line);
    std::snprintf(line, sizeof(line),
                  "cutscenes=%zu timestamps=%zu commands=%zu",
                  s.cutscenes, s.timestamps, s.commands);
    state.console.log(line);
}

std::string lowerCopy(std::string_view text) {
    std::string out(text);
    for (char& c : out)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

std::string resourceBaseName(std::string_view text) {
    std::string out(text);
    for (char& c : out)
        if (c == '\\') c = '/';
    const std::size_t slash = out.find_last_of('/');
    if (slash != std::string::npos) out.erase(0, slash + 1);
    const std::size_t dot = out.find_last_of('.');
    if (dot != std::string::npos) out.erase(dot);
    return lowerCopy(out);
}

u32 findAudioKey(const mmu::PackedArchive& pak, std::string_view resource,
                 mmu::PackedEntryType type) {
    const std::string needle = resourceBaseName(resource);
    if (needle.empty()) return 0;
    for (const mmu::PackedEntry& entry : pak.entries()) {
        if (entry.type != type) continue;
        if (resourceBaseName(entry.name) == needle) return entry.key;
    }
    return 0;
}

// FUN_0050f7d0 starts literal .xcut paths with FUN_0050ba40 and returns
// without constructing a screen; FUN_00511350 does the same for its stage
// completion prelude. FlowEvent carries those paths, so the harness starts the
// matching decoded cutscene here rather than guessing from label text.
bool startFlowCutscene(AppState& state, std::string_view path) {
    if (path.empty()) return false;
    state.cutsceneRuntime.attach(state.assets);
    const bool started = state.cutsceneRuntime.startPathSuffix(path);
    char line[320];
    std::snprintf(line, sizeof(line), "flow cutscene '%.*s': %s",
                  static_cast<int>(path.size()), path.data(),
                  started ? "started" : "not found");
    state.console.log(line);
    if (started) state.cutsceneEventLog.clear();
    return started;
}

void dispatchCutsceneEvents(AppState& state) {
    mmu::CutsceneRuntime& cutscene = state.cutsceneRuntime;
    while (cutscene.pendingEventCount() != 0) {
        const mmu::CutsceneEvent event = cutscene.pollEvent();
        state.cutsceneEventLog.push_back(event);
        if (state.cutsceneEventLog.size() > 256)
            state.cutsceneEventLog.erase(state.cutsceneEventLog.begin());

        switch (event.kind) {
        case mmu::CutsceneCommandKind::PlaySound: {
            const u32 key = findAudioKey(state.pak, event.resource,
                                         mmu::PackedEntryType::Audio);
            if (key != 0) state.audio.playSfx(state.pak, key);
            break;
        }
        case mmu::CutsceneCommandKind::PlayMusicLoop:
        case mmu::CutsceneCommandKind::PlayMusicOnce: {
            const u32 key = findAudioKey(state.pak, event.resource,
                                         mmu::PackedEntryType::Music);
            if (key != 0) state.audio.playMusic(state.pak, key);
            break;
        }
        case mmu::CutsceneCommandKind::StopSound:
            // The original names a music loop or a single cue. Stopping by
            // decoded entry name leaves unrelated music/SFX untouched.
            state.audio.stopNamed(event.resource);
            break;
        default:
            break;
        }
    }
}

void loadAssetFile(AppState& state, const std::string& filename) {
    const std::string path = joinAssetPath(state.assetDir, filename);
    const mmu::AssetKind kind = mmu::kindForFilename(filename);
    if (kind == mmu::AssetKind::Unknown) {
        state.console.log("Unknown asset file: " + filename);
        return;
    }
    if (!state.assets.loadFile(path)) {
        state.console.log("Failed to load " + path);
        return;
    }
    state.console.log("Loaded " + filename + " as " + mmu::kindName(kind));
    logStats(state);
}

// Neutral image description shared by the raw protobuf decoder and the baked
// archive. Both paths end up with the same SDL-derived pixel format metadata.
struct ImageDesc {
    mmu::ByteView pixels;
    mmu::ByteView palette; // packed RGB triples from the baked archive
    const std::vector<mmu::ColorAsset>* paletteColors = nullptr;
    mmu::u32 width = 0, height = 0, pitch = 0;
    mmu::u32 bitsPerPixel = 0, bytesPerPixel = 0;
    mmu::u32 rLoss = 0, gLoss = 0, bLoss = 0, aLoss = 0;
    mmu::u32 rShift = 0, gShift = 0, bShift = 0, aShift = 0;
    mmu::u32 rMask = 0, gMask = 0, bMask = 0, aMask = 0;
};

ImageDesc describe(const mmu::SurfaceAsset& surface) {
    ImageDesc desc;
    desc.pixels = mmu::ByteView(surface.pixels.data(), surface.pixels.size());
    desc.paletteColors = &surface.palette;
    desc.width = surface.width;
    desc.height = surface.height;
    desc.pitch = surface.pitch;
    desc.bitsPerPixel = surface.bitsPerPixel;
    desc.bytesPerPixel = surface.bytesPerPixel;
    desc.rLoss = surface.rLoss; desc.gLoss = surface.gLoss;
    desc.bLoss = surface.bLoss; desc.aLoss = surface.aLoss;
    desc.rShift = surface.rShift; desc.gShift = surface.gShift;
    desc.bShift = surface.bShift; desc.aShift = surface.aShift;
    desc.rMask = surface.rMask; desc.gMask = surface.gMask;
    desc.bMask = surface.bMask; desc.aMask = surface.aMask;
    return desc;
}

ImageDesc describe(const mmu::PackedEntry& entry) {
    ImageDesc desc;
    desc.pixels = entry.data;
    desc.palette = entry.palette;
    desc.width = entry.width;
    desc.height = entry.height;
    desc.pitch = entry.pitch;
    desc.bitsPerPixel = entry.bitsPerPixel;
    desc.bytesPerPixel = entry.bytesPerPixel;
    desc.rLoss = entry.rLoss; desc.gLoss = entry.gLoss;
    desc.bLoss = entry.bLoss; desc.aLoss = entry.aLoss;
    desc.rShift = entry.rShift; desc.gShift = entry.gShift;
    desc.bShift = entry.bShift; desc.aShift = entry.aShift;
    desc.rMask = entry.rMask; desc.gMask = entry.gMask;
    desc.bMask = entry.bMask; desc.aMask = entry.aMask;
    return desc;
}

std::vector<mmu::u8> surfaceToRGBA(const ImageDesc& surface, int& width, int& height) {
    width = static_cast<int>(surface.width);
    height = static_cast<int>(surface.height);
    if (width <= 0 || height <= 0 || surface.pixels.empty())
        return {};

    const bool indexed = surface.bitsPerPixel == 8 &&
        ((surface.paletteColors && !surface.paletteColors->empty()) || !surface.palette.empty());
    const std::size_t bytesPerPixel = indexed ? 1u : (surface.bytesPerPixel ? surface.bytesPerPixel : 4u);

    // SDL stores rows with pitch-byte stride, but the final scanline may omit
    // padding. Deriving the stride from the actual data keeps us in bounds.
    const std::size_t availableRows = surface.pixels.size / static_cast<std::size_t>(height);
    const std::size_t rowStride = availableRows ? availableRows : surface.pitch;

    std::vector<mmu::u8> rgba(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4, 255);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            mmu::u8* out = &rgba[(static_cast<std::size_t>(y) * width + x) * 4];
            const std::size_t sourceIndex = static_cast<std::size_t>(y) * rowStride
                                          + static_cast<std::size_t>(x) * bytesPerPixel;
            if (sourceIndex + bytesPerPixel > surface.pixels.size) continue;

            if (indexed) {
                const std::size_t colorIndex = surface.pixels.data[sourceIndex];
                if (surface.paletteColors && colorIndex < surface.paletteColors->size()) {
                    out[0] = (*surface.paletteColors)[colorIndex].r;
                    out[1] = (*surface.paletteColors)[colorIndex].g;
                    out[2] = (*surface.paletteColors)[colorIndex].b;
                } else if (colorIndex * 3 + 2 < surface.palette.size) {
                    out[0] = surface.palette.data[colorIndex * 3];
                    out[1] = surface.palette.data[colorIndex * 3 + 1];
                    out[2] = surface.palette.data[colorIndex * 3 + 2];
                }
                continue;
            }

            mmu::u32 pixel = surface.pixels.data[sourceIndex];
            if (bytesPerPixel >= 2)
                pixel |= static_cast<mmu::u32>(surface.pixels.data[sourceIndex + 1]) << 8;
            if (bytesPerPixel >= 3)
                pixel |= static_cast<mmu::u32>(surface.pixels.data[sourceIndex + 2]) << 16;
            if (bytesPerPixel >= 4)
                pixel |= static_cast<mmu::u32>(surface.pixels.data[sourceIndex + 3]) << 24;

            const auto channel = [&](mmu::u32 mask, mmu::u32 shift, mmu::u32 loss) {
                const mmu::u32 bits = (pixel & mask) >> shift;
                return static_cast<mmu::u8>(loss >= 8 ? bits : (bits << loss) | (bits >> (8 - loss)));
            };
            out[0] = channel(surface.rMask, surface.rShift, surface.rLoss);
            out[1] = channel(surface.gMask, surface.gShift, surface.gLoss);
            out[2] = channel(surface.bMask, surface.bShift, surface.bLoss);
            out[3] = surface.aMask ? channel(surface.aMask, surface.aShift, surface.aLoss) : 255;
        }
    }
    return rgba;
}

void uploadPreview(AppState& state, SDL_Renderer* renderer, const ImageDesc& desc) {
    if (state.preview) {
        SDL_DestroyTexture(state.preview);
        state.preview = nullptr;
    }
    int width = 0, height = 0;
    const std::vector<mmu::u8> rgba = surfaceToRGBA(desc, width, height);
    if (rgba.empty()) return;

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STATIC, width, height);
    if (!texture) return;
    if (SDL_UpdateTexture(texture, nullptr, rgba.data(),
                          static_cast<int>(width * 4)) != 0) {
        SDL_DestroyTexture(texture);
        return;
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    state.preview = texture;
    state.previewWidth = width;
    state.previewHeight = height;
}

// Lazily uploads baked images as SDL textures. Only sprites the user actually
// looks at cost GPU memory, which matters on console builds.
class TextureCache {
public:
    explicit TextureCache(SDL_Renderer* renderer) : renderer_(renderer) {}
    ~TextureCache() { clear(); }

    SDL_Texture* get(const mmu::PackedArchive& pak, u32 entryIndex) {
        if (entryIndex >= pak.entries().size()) return nullptr;
        if (textures_.size() <= entryIndex) textures_.resize(entryIndex + 1, nullptr);
        if (textures_[entryIndex]) return textures_[entryIndex];

        int width = 0, height = 0;
        const std::vector<mmu::u8> rgba = surfaceToRGBA(describe(pak.entries()[entryIndex]),
                                                         width, height);
        if (rgba.empty()) return nullptr;
        // Our buffer is RGBA *bytes* (SDL_Color order). RGBA8888 on a
        // little-endian host means A,B,G,R bytes, so use the byte-order-safe
        // alias or red and blue get swapped.
        SDL_Texture* texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                                                 SDL_TEXTUREACCESS_STATIC, width, height);
        if (!texture) return nullptr;
        if (SDL_UpdateTexture(texture, nullptr, rgba.data(), width * 4) != 0) {
            SDL_DestroyTexture(texture);
            return nullptr;
        }
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        textures_[entryIndex] = texture;
        return texture;
    }

    void clear() {
        for (SDL_Texture* texture : textures_)
            if (texture) SDL_DestroyTexture(texture);
        textures_.clear();
    }

private:
    SDL_Renderer* renderer_ = nullptr;
    std::vector<SDL_Texture*> textures_;
};

bool loadPackedArchive(AppState& state, const std::string& filename) {
    std::string path = joinAssetPath(state.assetDir, filename);
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        // Also accept an `asset/` child directory next to the executable.
        path = joinAssetPath(state.assetDir, joinAssetPath("asset", filename));
        file.open(path, std::ios::binary);
    }
    if (!file) return false;
    file.seekg(0, std::ios::end);
    const std::streamoff length = file.tellg();
    if (length <= 0) return false;
    file.seekg(0, std::ios::beg);
    state.pakBytes.resize(static_cast<std::size_t>(length));
    file.read(reinterpret_cast<char*>(state.pakBytes.data()), length);
    if (!file || !state.pak.load(mmu::ByteView(state.pakBytes.data(), state.pakBytes.size()))) {
        state.pak.clear();
        state.pakBytes.clear();
        return false;
    }
    state.assets.clear();
    if (!state.assets.loadPackedCutscenes(state.pak))
        state.console.log("Packed archive has no decoded cutscene table (re-bake with mmu_bake).");
    logStats(state);
    char line[160];
    std::snprintf(line, sizeof(line), "Packed archive: %zu entries (%zu images)",
                  state.pak.entries().size(), state.pak.imageCount());
    state.console.log(line);
    return true;
}

int findSpriteIndex(const mmu::PackedArchive& pak, u32 key);

std::string spriteLabel(const mmu::PackedArchive& pak, const mmu::PackedSprite& sprite) {
    if (!sprite.resRoot.empty()) return std::string(sprite.resRoot);
    if (sprite.frames && sprite.frameCount) {
        const u32 index = sprite.frames[0].imageIndex;
        if (index < pak.entries().size() && !pak.entries()[index].name.empty())
            return std::string(pak.entries()[index].name);
    }
    return "key " + std::to_string(sprite.key);
}

int findSceneIndex(const mmu::PackedArchive& pak, const std::string& arg) {
    if (arg.empty()) return -1;
    if (arg[0] == '#')
        return static_cast<int>(std::strtoul(arg.c_str() + 1, nullptr, 0));
    const u32 key = static_cast<u32>(std::strtoul(arg.c_str(), nullptr, 0));
    for (std::size_t i = 0; i < pak.scenes().size(); ++i)
        if (pak.scenes()[i].key == key) return static_cast<int>(i);
    if (key < pak.scenes().size()) return static_cast<int>(key);
    return -1;
}

// Harness-only resolver from a recovered level constructor to a baked scene.
// It never invents a scene: every candidate is an exact PackedScene name from
// assets.pak, and the class-name tokens come from LevelSetupTable.h. The
// `_Section_1`/`_Section_01` preference selects by filename, not by an inferred
// gameplay transition. If that scene has no decoded spawns, the first matching
// scene with spawns is used so the harness enters a playable section; this is
// harness policy over decoded PackedScene data, not an assigned stage order.
std::string lowerAscii(std::string_view text) {
    std::string result;
    result.reserve(text.size());
    for (char ch : text)
        result.push_back(static_cast<char>(
            std::tolower(static_cast<unsigned char>(ch))));
    return result;
}

int stageSceneIndexForToken(const mmu::PackedArchive& pak,
                            const std::string& token) {
    if (token.empty()) return -1;
    const std::string prefix = "\\levels\\" + lowerAscii(token);
    int sectionOne = -1;
    int firstSection = -1;
    int firstWithSpawns = -1;
    for (std::size_t i = 0; i < pak.scenes().size(); ++i) {
        const std::string name = lowerAscii(pak.scenes()[i].name);
        if (name.rfind(prefix, 0) != 0) continue;
        if (firstSection < 0) firstSection = static_cast<int>(i);
        if (firstWithSpawns < 0 && pak.scenes()[i].spawnCount != 0)
            firstWithSpawns = static_cast<int>(i);
        const std::size_t tail = prefix.size();
        if (name.compare(tail, 11, "_section_1.") == 0 ||
            name.compare(tail, 12, "_section_01.") == 0) {
            sectionOne = static_cast<int>(i);
        }
    }
    if (sectionOne >= 0 &&
        pak.scenes()[static_cast<std::size_t>(sectionOne)].spawnCount != 0)
        return sectionOne;
    if (firstWithSpawns >= 0) return firstWithSpawns;
    return sectionOne >= 0 ? sectionOne : firstSection;
}

int stageSceneIndex(const mmu::PackedArchive& pak, mmu::u32 stageId) {
    const mmu::RecoveredLevelSetup* row = mmu::findLevelSetup(stageId);
    if (row == nullptr) return -1;
    std::string token(row->className);
    if (token.rfind("Level", 0) == 0) token.erase(0, 5);
    // The class names spell Wily stages as words; the editor paths use digits.
    // This is a filename normalization, not an assigned gameplay meaning.
    if (token == "WilyOne") token = "Wily1";
    else if (token == "WilyTwo") token = "Wily2";
    else if (token == "WilyThree") token = "Wily3";
    else if (token == "WilyFour") token = "Wily4";
    else if (token == "WilyFive") token = "Wily5";
    int index = stageSceneIndexForToken(pak, token);
    if (index >= 0) return index;
    for (const char* variant : {"Perfect", "Super"}) {
        const std::string prefix(variant);
        if (token.rfind(prefix, 0) == 0) {
            index = stageSceneIndexForToken(pak, token.substr(prefix.size()));
            if (index >= 0) return index;
        }
    }
    return -1;
}

int findFlowScreenIndex(const std::string& arg) {
    if (arg.empty()) return -1;
    if (arg[0] == '#')
        return static_cast<int>(std::strtoul(arg.c_str() + 1, nullptr, 0));
    for (std::size_t i = 0; i < mmu::kFlowScreenCount; ++i)
        if (arg == mmu::kFlowScreens[i].className) return static_cast<int>(i);
    return -1;
}

int findFlowCueIndex(const std::string& arg) {
    if (arg.empty()) return -1;
    const int count = static_cast<int>(mmu::FlowCue::BigExplosion) + 1;
    for (int i = 0; i < count; ++i) {
        const mmu::FlowCue cue = static_cast<mmu::FlowCue>(i);
        if (arg == mmu::flowCueName(cue)) return i;
    }
    return -1;
}

int findFlowLevelCueIndex(const std::string& arg) {
    if (arg.empty()) return -1;
    for (std::size_t i = 0; i < mmu::kFlowLevelCueCount; ++i) {
        const mmu::FlowLevelCue cue = static_cast<mmu::FlowLevelCue>(i);
        if (arg == mmu::flowLevelCueName(cue)) return static_cast<int>(i);
    }
    return -1;
}

const char* menuItemBehaviorName(mmu::u8 behavior) {
    switch (static_cast<mmu::MenuItemBehavior>(behavior)) {
    case mmu::MenuItemBehavior::Spacer: return "spacer";
    case mmu::MenuItemBehavior::Confirm: return "confirm";
    case mmu::MenuItemBehavior::Flash: return "flash";
    }
    return "?";
}

const char* flowEventTypeName(mmu::FlowEventType type) {
    switch (type) {
    case mmu::FlowEventType::ScreenChanged: return "screen";
    case mmu::FlowEventType::StateChanged: return "state";
    case mmu::FlowEventType::Cue: return "cue";
    case mmu::FlowEventType::LevelCue: return "levelcue";
    case mmu::FlowEventType::ScreenChangeRequested: return "exit";
    case mmu::FlowEventType::StageSelected: return "stage";
    case mmu::FlowEventType::StageCompleted: return "complete";
    case mmu::FlowEventType::ManagerBoot: return "boot";
    case mmu::FlowEventType::LevelSetupApplied: return "levelsetup";
    case mmu::FlowEventType::ActivityMaskChanged: return "mask";
    case mmu::FlowEventType::SlotSelected: return "slot";
    case mmu::FlowEventType::ResumeRequested: return "resume";
    case mmu::FlowEventType::MenuAction: return "action";
    }
    return "?";
}

const char* activityMaskReadKindName(mmu::ActivityMaskReadKind kind) {
    switch (kind) {
    case mmu::ActivityMaskReadKind::Equal: return "equal";
    case mmu::ActivityMaskReadKind::Bit: return "bit";
    case mmu::ActivityMaskReadKind::Intersect: return "intersect";
    case mmu::ActivityMaskReadKind::DynamicEqual: return "dyn_equal";
    case mmu::ActivityMaskReadKind::NonZero: return "nonzero";
    case mmu::ActivityMaskReadKind::Unknown: return "unknown";
    }
    return "?";
}

const char* managerFieldAccessKindName(mmu::ManagerFieldAccessKind kind) {
    switch (kind) {
    case mmu::ManagerFieldAccessKind::Read: return "read";
    case mmu::ManagerFieldAccessKind::Write: return "write";
    }
    return "?";
}

void registerCommands(AppState& state) {
    state.console.registerCommand("help", "list all commands", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        c.log(c.helpText());
    });
    state.console.registerCommand("clear", "clear the log", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        c.clear();
    });
    state.console.registerCommand("ui", "show or set ImGui profile: ui [desktop|touch]", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (!args.empty()) {
            if (args[0] != "desktop" && args[0] != "touch") {
                c.log("usage: ui [desktop|touch]");
                return;
            }
            int w = 0, h = 0;
            SDL_GetRendererOutputSize(g_renderer, &w, &h);
            state.ui = makeUiProfile(args[0] == "touch", args[0] == "desktop", h);
            applyUiProfile(state.ui);
        }
        char line[192];
        std::snprintf(line, sizeof(line),
                      "imgui profile=%s style=%.1fx font=%.2fx touch-layout=%s "
                      "(touch events are mapped to mouse by SDL)",
                      state.ui.name(), state.ui.styleScale, state.ui.fontScale,
                      state.ui.touchLayout ? "yes" : "no");
        c.log(line);
    });
    state.console.registerCommand("flow", "recovered game-flow evidence: flow [screens|levels|name|#index]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "screens" : args[0];
        if (what == "screens") {
            char line[192];
            std::snprintf(line, sizeof(line), "recovered menus: %zu screens, %zu level classes, %zu level cues",
                          mmu::kFlowScreenCount, mmu::kFlowLevelClassCount,
                          mmu::kFlowLevelCueCount);
            c.log(line);
            for (std::size_t i = 0; i < mmu::kFlowScreenCount; ++i) {
                const mmu::FlowScreenEvidence& e = mmu::kFlowScreens[i];
                std::snprintf(line, sizeof(line), "#%zu %-28s 0x%08x states=%u cues=%u",
                              i, e.className, e.vftableVa, e.stateCount, e.cueCount);
                c.log(line);
            }
            c.log("Use 'flow <name|#index>' for a detail view; this is evidence, not a game loop.");
            return;
        }
        if (what == "levels") {
            char line[192];
            for (std::size_t i = 0; i < mmu::kFlowLevelClassCount; ++i) {
                const mmu::FlowLevelClass& e = mmu::kFlowLevelClasses[i];
                std::snprintf(line, sizeof(line), "%-30s 0x%08x", e.name, e.vftableVa);
                c.log(line);
            }
            return;
        }
        const int index = findFlowScreenIndex(what);
        if (index < 0 || static_cast<std::size_t>(index) >= mmu::kFlowScreenCount) {
            c.log("usage: flow [screens|levels|name|#index]");
            return;
        }
        const mmu::FlowScreenEvidence& e = mmu::kFlowScreens[index];
        char line[256];
        std::snprintf(line, sizeof(line), "%s vftable=0x%08x", e.className, e.vftableVa);
        c.log(line);
        std::string states;
        for (mmu::u8 i = 0; i < e.stateCount; ++i) {
            if (i) states += ", ";
            states += mmu::flowStateName(e.states[i]);
        }
        c.log("  states: " + (states.empty() ? std::string("-") : states));
        std::string cues;
        for (mmu::u8 i = 0; i < e.cueCount; ++i) {
            if (i) cues += ", ";
            cues += mmu::flowCueName(e.cues[i]);
        }
        c.log("  cues:   " + (cues.empty() ? std::string("-") : cues));
    });
    state.console.registerCommand("decomp", "raw decompilation index: decomp [summary|list [filter]|func <va>|targets [kind]|xrefs|xref <class>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[320];
        auto emitXref = [&c, &line](const mmu::DecompStringXref& row) {
            char refs[128];
            refs[0] = '\0';
            for (mmu::u8 i = 0; i < row.refCount && i < 4; ++i)
                std::snprintf(refs + std::strlen(refs), sizeof(refs) - std::strlen(refs),
                              " 0x%06x", row.refVas[i]);
            std::snprintf(line, sizeof(line), "  %-16s string=0x%06x refs=%u%s",
                          std::string(row.className).c_str(), row.stringVa,
                          row.refCount, refs);
            c.log(line);
        };
        if (what == "list") {
            const std::string filter = args.size() >= 2 ? args[1] : std::string();
            std::size_t shown = 0;
            for (u32 va : mmu::kDecompiledFunctionVas) {
                char value[32];
                std::snprintf(value, sizeof(value), "0x%06x", va);
                if (!filter.empty() && std::string(value).find(filter) == std::string::npos)
                    continue;
                std::snprintf(line, sizeof(line), "  0x%06x", va);
                c.log(line);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "decompiled functions shown=%zu", shown);
            c.log(line);
        } else if (what == "func") {
            if (args.size() < 2) { c.log("usage: decomp func <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::snprintf(line, sizeof(line), "  0x%06x decompiled=%s", va,
                          mmu::hasDecompiledFunction(va) ? "true" : "false");
            c.log(line);
        } else if (what == "targets") {
            if (args.size() >= 2) {
                const std::string kind = args[1];
                const u32* values = nullptr;
                std::size_t count = 0;
                if (kind == "targets") {
                    values = mmu::kDecompTargetsTargetVas;
                    count = mmu::kDecompTargetsTargetVasCount;
                } else if (kind == "flow") {
                    values = mmu::kDecompFlowTargetVas;
                    count = mmu::kDecompFlowTargetVasCount;
                } else if (kind == "dispatcher") {
                    values = mmu::kDecompDispatcherTargetVas;
                    count = mmu::kDecompDispatcherTargetVasCount;
                } else if (kind == "unique") {
                    values = mmu::kDecompUniqueTargetVas;
                    count = mmu::kDecompUniqueTargetVasCount;
                } else {
                    c.log("usage: decomp targets [targets|flow|dispatcher|unique]");
                    return;
                }
                for (std::size_t i = 0; i < count; ++i) {
                    std::snprintf(line, sizeof(line), "  0x%06x", values[i]);
                    c.log(line);
                }
                std::snprintf(line, sizeof(line), "decomp %s targets=%zu",
                              kind.c_str(), count);
                c.log(line);
            } else {
                std::snprintf(line, sizeof(line),
                              "targets=%zu flow=%zu dispatcher=%zu unique=%zu",
                              mmu::kDecompTargetsTargetVasCount,
                              mmu::kDecompFlowTargetVasCount,
                              mmu::kDecompDispatcherTargetVasCount,
                              mmu::kDecompUniqueTargetVasCount);
                c.log(line);
            }
        } else if (what == "xrefs") {
            for (const mmu::DecompStringXref& row : mmu::kDecompStringXrefs)
                emitXref(row);
            std::snprintf(line, sizeof(line), "decomp string xrefs=%zu",
                          mmu::kDecompStringXrefCount);
            c.log(line);
        } else if (what == "xref") {
            if (args.size() < 2) { c.log("usage: decomp xref <class>"); return; }
            const mmu::DecompStringXref* row = mmu::decompStringXref(args[1]);
            if (row) emitXref(*row);
            else c.log("  (unknown class)");
        } else if (what != "summary") {
            c.log("usage: decomp [summary|list [filter]|func <va>|targets [kind]|xrefs|xref <class>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "decompiled functions=%zu string xrefs=%zu unique targets=%zu",
                          mmu::kDecompiledFunctionVasCount,
                          mmu::kDecompStringXrefCount,
                          mmu::kDecompUniqueTargetVasCount);
            c.log(line);
        }
    });
    state.console.registerCommand("protos", "raw proto2 schema index: protos [summary|list|schema <name>|messages <name>|message <schema> <name>|field <name>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        auto emitField = [&c, &line](const mmu::ProtoField& field) {
            std::snprintf(line, sizeof(line), "  %s.%s: %.*s",
                          std::string(field.schema).c_str(),
                          std::string(field.message).c_str(),
                          static_cast<int>(field.sourceLine.size()),
                          field.sourceLine.data());
            c.log(line);
        };
        auto findSchema = [](const std::string& name) -> const mmu::ProtoSchema* {
            return mmu::protoSchema(name);
        };
        if (what == "list") {
            for (const mmu::ProtoSchema& schema : mmu::kProtoSchemas) {
                std::snprintf(line, sizeof(line),
                              "  %-12s syntax=%s package=%s messages=%u fields=%u",
                              std::string(schema.name).c_str(),
                              std::string(schema.syntax).c_str(),
                              std::string(schema.package).c_str(),
                              schema.messageCount, schema.fieldCount);
                c.log(line);
            }
        } else if (what == "schema") {
            if (args.size() < 2) { c.log("usage: protos schema <name>"); return; }
            const mmu::ProtoSchema* schema = findSchema(args[1]);
            if (!schema) { c.log("  (unknown schema)"); return; }
            std::string_view source = schema->source;
            std::size_t pos = 0;
            while (pos < source.size()) {
                const std::size_t end = source.find('\n', pos);
                const std::size_t stop = end == std::string_view::npos ? source.size() : end;
                c.log(std::string(source.substr(pos, stop - pos)));
                if (end == std::string_view::npos) break;
                pos = end + 1;
            }
        } else if (what == "messages") {
            if (args.size() < 2) { c.log("usage: protos messages <name>"); return; }
            const mmu::ProtoSchema* schema = findSchema(args[1]);
            if (!schema) { c.log("  (unknown schema)"); return; }
            for (u32 i = 0; i < schema->messageCount; ++i) {
                const mmu::ProtoMessage& message =
                    mmu::kProtoMessages[schema->messageOffset + i];
                std::snprintf(line, sizeof(line), "  %s fields=%u",
                              std::string(message.name).c_str(), message.fieldCount);
                c.log(line);
            }
        } else if (what == "message") {
            if (args.size() < 3) { c.log("usage: protos message <schema> <name>"); return; }
            const mmu::ProtoMessage* message = mmu::protoMessage(args[1], args[2]);
            if (!message) { c.log("  (unknown message)"); return; }
            for (u32 i = 0; i < message->fieldCount; ++i)
                emitField(mmu::kProtoFields[message->fieldOffset + i]);
        } else if (what == "field") {
            if (args.size() < 2) { c.log("usage: protos field <name>"); return; }
            std::size_t shown = 0;
            for (const mmu::ProtoField& field : mmu::kProtoFields) {
                if (field.name != args[1]) continue;
                emitField(field);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "proto field matches=%zu", shown);
            c.log(line);
        } else if (what != "summary") {
            c.log("usage: protos [summary|list|schema <name>|messages <name>|message <schema> <name>|field <name>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "proto schemas=%zu messages=%zu fields=%zu",
                          mmu::kProtoSchemaCount, mmu::kProtoMessageCount,
                          mmu::kProtoFieldCount);
            c.log(line);
        }
    });
    state.console.registerCommand("flowcontinue", "raw saved-stage constructor table: flowcontinue [summary|list|stage <id>|class <name>|ctor <va>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[320];
        auto emit = [&c, &line](const mmu::RecoveredContinueStage& row) {
            std::snprintf(line, sizeof(line), "  stage=%-3u ctor=0x%06x class=%.*s",
                          row.stageId, row.constructorVa,
                          static_cast<int>(row.className.size()),
                          row.className.data());
            c.log(line);
        };
        if (what == "list") {
            for (const mmu::RecoveredContinueStage& row : mmu::kRecoveredContinueStages)
                emit(row);
        } else if (what == "stage" || what == "class" || what == "ctor") {
            if (args.size() < 2) {
                std::snprintf(line, sizeof(line), "usage: flowcontinue %s <%s>",
                              what.c_str(), what == "stage" ? "id" : what.c_str());
                c.log(line);
                return;
            }
            const mmu::RecoveredContinueStage* match = nullptr;
            if (what == "stage") {
                match = mmu::findContinueStage(
                    static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0)));
            } else if (what == "class") {
                for (const mmu::RecoveredContinueStage& row : mmu::kRecoveredContinueStages)
                    if (row.className == args[1]) { match = &row; break; }
            } else {
                const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
                for (const mmu::RecoveredContinueStage& row : mmu::kRecoveredContinueStages)
                    if (row.constructorVa == va) { match = &row; break; }
            }
            if (match) emit(*match);
            else c.log("  (no matching stage row)");
        } else if (what != "summary") {
            c.log("usage: flowcontinue [summary|list|stage <id>|class <name>|ctor <va>]");
        } else {
            std::snprintf(line, sizeof(line), "continue stages=%zu first=%u last=%u",
                          mmu::kRecoveredContinueStageCount,
                          mmu::kRecoveredContinueStages[0].stageId,
                          mmu::kRecoveredContinueStages[
                              mmu::kRecoveredContinueStageCount - 1].stageId);
            c.log(line);
        }
    });
    state.console.registerCommand("flowraw", "raw game_flow.txt lines: flowraw [summary|line <n>|find <text>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[1024];
        auto emit = [&c, &line](std::size_t number, std::string_view text) {
            std::snprintf(line, sizeof(line), "  %zu: %.*s", number,
                          static_cast<int>(text.size()), text.data());
            c.log(line);
        };
        if (what == "line") {
            if (args.size() < 2) { c.log("usage: flowraw line <n>"); return; }
            const std::size_t number = static_cast<std::size_t>(
                std::strtoul(args[1].c_str(), nullptr, 0));
            const std::string_view text = mmu::gameFlowEvidenceLine(number);
            if (text.empty()) c.log("  (line out of range)");
            else emit(number, text);
        } else if (what == "find") {
            if (args.size() < 2) { c.log("usage: flowraw find <text>"); return; }
            std::size_t shown = 0;
            for (std::size_t i = 0; i < mmu::kGameFlowEvidenceLineCount; ++i) {
                const std::string_view text = mmu::kGameFlowEvidenceLines[i];
                if (std::string(text).find(args[1]) == std::string::npos)
                    continue;
                emit(i + 1, text);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "flow raw matches=%zu", shown);
            c.log(line);
        } else if (what != "summary") {
            c.log("usage: flowraw [summary|line <n>|find <text>]");
        } else {
            const std::string_view first = mmu::kGameFlowEvidenceLines[0];
            const std::string_view last =
                mmu::kGameFlowEvidenceLines[mmu::kGameFlowEvidenceLineCount - 1];
            std::snprintf(line, sizeof(line),
                          "flow raw lines=%zu first=%.*s last=%.*s",
                          mmu::kGameFlowEvidenceLineCount,
                          static_cast<int>(first.size()), first.data(),
                          static_cast<int>(last.size()), last.data());
            c.log(line);
        }
    });
    state.console.registerCommand("flowboot", "raw manager boot-cutscene evidence: flowboot [summary|writes|play|pending]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[320];
        if (what == "writes") {
            for (const mmu::BootManagerWrite& write : mmu::kBootManagerWrites) {
                std::snprintf(line, sizeof(line), "  +0x%02x = 0x%04x",
                              write.offset, write.value);
                c.log(line);
            }
        } else if (what == "play") {
            std::snprintf(line, sizeof(line), "  play %.*s",
                          static_cast<int>(mmu::kBootPlayCutscene.size()),
                          mmu::kBootPlayCutscene.data());
            c.log(line);
        } else if (what == "pending") {
            std::snprintf(line, sizeof(line), "  pending %.*s",
                          static_cast<int>(mmu::kBootPendingCutscene.size()),
                          mmu::kBootPendingCutscene.data());
            c.log(line);
        } else if (what != "summary") {
            c.log("usage: flowboot [summary|writes|play|pending]");
        } else {
            std::snprintf(line, sizeof(line),
                          "boot mask=0x%02x constructor=0x%06x manager=0x%06x writes=%zu",
                          mmu::kBootActivityMask, mmu::kBootConstructorVa,
                          mmu::kBootManagerGlobalVa, mmu::kBootManagerWriteCount);
            c.log(line);
        }
    });
    state.console.registerCommand("flowrun", "step reconstructed flow runtime: flowrun [status|reset|screen|supplier|flags|manager|continue|complete|boot|keyflag|masks|mask|maskreads|maskread|confirm|cancel|up|down|left|right|step|cue|levelcue|events]", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        mmu::GameFlowRuntime& flow = state.flowRuntime;
        const std::string what = args.empty() ? "status" : args[0];
        const auto showStatus = [&]() {
            char line[1024];
            char stage[96] = "-";
            const std::string_view dispatcherName = flow.dispatcher().currentName();
            const mmu::FlowMenuFlags flags = flow.menuFlags();
            const mmu::RecoveredContinueStage* row =
                mmu::findContinueStage(flow.continueStageId());
            if (row != nullptr) {
                std::snprintf(stage, sizeof(stage), "%.*s/0x%06x",
                              static_cast<int>(row->className.size()),
                              row->className.data(), row->constructorVa);
            }
            std::snprintf(line, sizeof(line),
                          "flow screen=%s state=%s dispatcher=%.*s/0x%08x phase=%s sel=%d table=0x%06x t=%.2f state_t=%.2f running=%s cue=%s events=%zu flags=19:%d,1a:%d,59:%d,pause:%d sub:%d,boss:%d,four:%d,eight:%d,prev9:%d,ba8:%d cut=0x%llx mask=0x%x/0x%x mgr=phase:%u,opaque:%u,stage:%u,exit:%d continue=%u:%s",
                          mmu::flowScreenName(flow.screen()),
                          mmu::flowStateName(flow.state()),
                          static_cast<int>(dispatcherName.size()), dispatcherName.data(),
                          flow.dispatcher().currentHash(),
                          mmu::flowLevelPhaseName(flow.levelPhase()),
                          flow.selection(), flow.menuSupplier(),
                          flow.time(), flow.stateTime(),
                          flow.running() ? "yes" : "no",
                          mmu::flowCueName(flow.lastCue()), flow.pendingEventCount(),
                          flags.flag0x19 ? 1 : 0, flags.flag0x1a ? 1 : 0,
                          flags.flag0x59 ? 1 : 0,
                          flags.pauseExitStageVariant ? 1 : 0,
                          flags.subItemCount, flags.bossCount,
                          flags.fourBossesSeen ? 1 : 0,
                          flags.eightBossesSeen ? 1 : 0,
                          flags.previousActionIs9 ? 1 : 0,
                          flags.stageSelectGridFlag0xba8 ? 1 : 0,
                          static_cast<unsigned long long>(flags.completionFlags),
                          flow.keyedFlags().activityMask(),
                          flow.keyedFlags().previousActivityMask(),
                          flow.managerPhase(), flow.managerOpaque(),
                          flow.managerStagePhase(), flow.exitRequested() ? 1 : 0,
                          flow.continueStageId(), stage);
            c.log(line);
        };
        if (what == "status") {
            showStatus();
        } else if (what == "reset") {
            mmu::FlowScreen screen = mmu::FlowScreen::TitleMenu;
            if (args.size() > 1) {
                const int index = findFlowScreenIndex(args[1]);
                if (index < 0 || static_cast<std::size_t>(index) >= mmu::kFlowScreenCount) {
                    c.log("unknown flow screen: " + args[1]);
                    return;
                }
                screen = static_cast<mmu::FlowScreen>(index);
            }
            flow.reset(screen);
            showStatus();
        } else if (what == "screen") {
            if (args.size() < 2) { c.log("usage: flowrun screen <name|#index>"); return; }
            const int index = findFlowScreenIndex(args[1]);
            if (index < 0 || static_cast<std::size_t>(index) >= mmu::kFlowScreenCount) {
                c.log("unknown flow screen: " + args[1]);
                return;
            }
            flow.setScreen(static_cast<mmu::FlowScreen>(index));
            showStatus();
        } else if (what == "supplier") {
            if (args.size() > 1) {
                char* end = nullptr;
                const unsigned long value = std::strtoul(args[1].c_str(), &end, 0);
                if (*end != '\0') { c.log("usage: flowrun supplier [0xVA]"); return; }
                flow.setMenuSupplier(static_cast<mmu::u32>(value));
            }
            showStatus();
        } else if (what == "flags") {
            mmu::FlowMenuFlags flags = flow.menuFlags();
            bool valid = true;
            for (std::size_t i = 1; i < args.size(); ++i) {
                const std::size_t eq = args[i].find('=');
                if (eq == std::string::npos) { valid = false; break; }
                const std::string key = args[i].substr(0, eq);
                const std::string value = args[i].substr(eq + 1);
                char* end = nullptr;
                if (key == "cut" || key == "completion") {
                    const unsigned long long mask =
                        std::strtoull(value.c_str(), &end, 0);
                    if (*end != '\0') { valid = false; break; }
                    flags.completionFlags = static_cast<mmu::u64>(mask);
                    continue;
                }
                const long number = std::strtol(value.c_str(), &end, 0);
                if (*end != '\0') { valid = false; break; }
                if (key == "sub" || key == "subitems") {
                    flags.subItemCount = static_cast<int>(number);
                } else if (key == "boss" || key == "bosses") {
                    flags.bossCount = static_cast<int>(number);
                } else {
                    if (number != 0 && number != 1) { valid = false; break; }
                    const bool set = number == 1;
                    if (key == "19" || key == "0x19") flags.flag0x19 = set;
                    else if (key == "1a" || key == "0x1a") flags.flag0x1a = set;
                    else if (key == "59" || key == "0x59") flags.flag0x59 = set;
                    else if (key == "pause" || key == "pauseexit" ||
                             key == "exitstage") flags.pauseExitStageVariant = set;
                    else if (key == "four") flags.fourBossesSeen = set;
                    else if (key == "eight") flags.eightBossesSeen = set;
                    else if (key == "prev9") flags.previousActionIs9 = set;
                    else if (key == "ba8" || key == "stagegrid")
                        flags.stageSelectGridFlag0xba8 = set;
                    else { valid = false; break; }
                }
            }
            if (!valid) {
                c.log("usage: flowrun flags [19=0|1] [1a=0|1] [59=0|1] [pause=0|1] [sub=N] [boss=N] [four=0|1] [eight=0|1] [prev9=0|1] [ba8=0|1] [cut=HEXMASK]");
                return;
            }
            flow.setMenuFlags(flags);
            showStatus();
        } else if (what == "continue") {
            if (args.size() > 1) {
                char* end = nullptr;
                const unsigned long value = std::strtoul(args[1].c_str(), &end, 0);
                if (*end != '\0' || value > 0xFFFFFFFFul) {
                    c.log("usage: flowrun continue [stage id]");
                    return;
                }
                flow.setContinueStageId(static_cast<mmu::u32>(value));
            }
            showStatus();
        } else if (what == "manager") {
            if (args.size() > 1) {
                char* end = nullptr;
                const unsigned long phase = std::strtoul(args[1].c_str(), &end, 0);
                if (*end != '\0') { c.log("usage: flowrun manager [phase] [opaque]"); return; }
                flow.setManagerPhase(static_cast<mmu::u32>(phase));
                if (args.size() > 2) {
                    const unsigned long opaque = std::strtoul(args[2].c_str(), &end, 0);
                    if (*end != '\0') { c.log("usage: flowrun manager [phase] [opaque]"); return; }
                    flow.setManagerOpaque(static_cast<mmu::u32>(opaque));
                }
            }
            showStatus();
        } else if (what == "keyflag") {
            if (args.size() < 3) {
                c.log("usage: flowrun keyflag <key> <0|1>");
                return;
            }
            const unsigned long keyValue = std::strtoul(args[1].c_str(), nullptr, 0);
            const unsigned long conditionValue = std::strtoul(args[2].c_str(), nullptr, 0);
            if (keyValue >= mmu::KeyedFlagRuntime::kKeyCount ||
                conditionValue > 1) {
                c.log("invalid keyflag arguments");
                return;
            }
            const mmu::u32 key = static_cast<mmu::u32>(keyValue);
            const mmu::u8 value = static_cast<mmu::u8>(conditionValue);
            bool found = false;
            for (const auto& row : mmu::kSaveFlagWriters) {
                if (row.key != key) continue;
                found = true;
                if (row.aux < mmu::kKeyedFlagKeyboardLimit) {
                    state.keyedKeyboard[row.aux] = value;
                } else {
                    const mmu::u32 offset =
                        row.aux - mmu::kKeyedFlagManagerByteBias;
                    if (offset < state.keyedManager.size())
                        state.keyedManager[offset] = value;
                }
            }
            if (!found) {
                c.log("unknown keyed flag");
                return;
            }
            flow.evaluateKeyedFlags(state.keyedKeyboard.data(),
                                    state.keyedKeyboard.size(),
                                    state.keyedManager.data(),
                                    state.keyedManager.size());
            char line[160];
            std::snprintf(line, sizeof(line),
                          "keyflag %u condition=%u bits=0x%02x true=%d "
                          "rising=%d falling=%d",
                          key, value, flow.keyedFlags().bits(key),
                          flow.keyedFlags().query(key, mmu::KeyedFlagRuntime::kTrue) ? 1 : 0,
                          flow.keyedFlags().query(key, mmu::KeyedFlagRuntime::kRising) ? 1 : 0,
                          flow.keyedFlags().query(key, mmu::KeyedFlagRuntime::kFalling) ? 1 : 0);
            c.log(line);
            showStatus();
        } else if (what == "masks") {
            if (args.size() >= 2 && args[1] == "sites") {
                char siteLine[512];
                for (const mmu::RecoveredActivityMaskCall& row :
                     mmu::kRecoveredActivityMaskCalls) {
                    std::snprintf(siteLine, sizeof(siteLine),
                                  "  site=0x%06x func=0x%06x mask=0x%x owner=%.*s "
                                  "slot=%u state=%.*s strings=%.*s",
                                  row.callVa, row.functionVa, row.mask,
                                  static_cast<int>(row.owner.size()), row.owner.data(),
                                  row.slot,
                                  static_cast<int>(row.stateEvidence.size()),
                                  row.stateEvidence.data(),
                                  static_cast<int>(row.decompiledStrings.size()),
                                  row.decompiledStrings.data());
                    c.log(siteLine);
                }
                char total[96];
                std::snprintf(total, sizeof(total), "mask setter sites=%zu",
                              mmu::kRecoveredActivityMaskCallCount);
                c.log(total);
                return;
            }
            for (mmu::u32 mask = 1; mask != 0; mask <<= 1) {
                const std::size_t count = mmu::countActivityMaskCalls(mask);
                if (count == 0) continue;
                char line[128];
                std::snprintf(line, sizeof(line), "mask=0x%x sites=%zu", mask,
                              count);
                c.log(line);
            }
            c.log("boot mask is not a direct setter site; use flowrun boot");
        } else if (what == "mask") {
            if (args.size() < 2) {
                c.log("usage: flowrun mask <hex> | mask call <site va> | mask boot");
                return;
            }
            if (args[1] == "boot") {
                flow.setBootActivityMask(mmu::kBootActivityMask);
            } else if (args[1] == "call" || args[1] == "site") {
                if (args.size() < 3) {
                    c.log("usage: flowrun mask call <site va>");
                    return;
                }
                char* end = nullptr;
                const unsigned long site =
                    std::strtoul(args[2].c_str(), &end, 0);
                if (*end != '\0' || site > 0xFFFFFFFFul ||
                    !flow.applyActivityMaskCall(static_cast<mmu::u32>(site))) {
                    c.log("unknown activity-mask call site");
                    return;
                }
            } else {
                char* end = nullptr;
                const unsigned long mask =
                    std::strtoul(args[1].c_str(), &end, 0);
                if (*end != '\0' || mask > 0xFFFFFFFFul) {
                    c.log("usage: flowrun mask <hex>");
                    return;
                }
                flow.setActivityMask(static_cast<mmu::u32>(mask));
            }
            char line[192];
            std::snprintf(line, sizeof(line),
                          "activity mask=0x%x prev=0x%x (raw keyed-flag gating)",
                          flow.keyedFlags().activityMask(),
                          flow.keyedFlags().previousActivityMask());
            c.log(line);
            showStatus();
        } else if (what == "maskreads") {
            if (args.size() >= 2 && args[1] == "all") {
                char readLine[512];
                for (const mmu::RecoveredActivityMaskRead& row :
                     mmu::kRecoveredActivityMaskReads) {
                    std::snprintf(readLine, sizeof(readLine),
                                  "  site=0x%06x func=0x%06x kind=%-10s mask=0x%x "
                                  "operand=%.*s owner=%.*s slot=%u state=%.*s "
                                  "strings=%.*s",
                                  row.callVa, row.functionVa,
                                  activityMaskReadKindName(row.kind), row.mask,
                                  static_cast<int>(row.operand.size()), row.operand.data(),
                                  static_cast<int>(row.owner.size()), row.owner.data(),
                                  row.slot,
                                  static_cast<int>(row.stateEvidence.size()),
                                  row.stateEvidence.data(),
                                  static_cast<int>(row.decompiledStrings.size()),
                                  row.decompiledStrings.data());
                    c.log(readLine);
                }
                char total[96];
                std::snprintf(total, sizeof(total), "mask reader sites=%zu",
                              mmu::kRecoveredActivityMaskReadCount);
                c.log(total);
                return;
            }
            for (int i = 0; i <= static_cast<int>(mmu::ActivityMaskReadKind::Unknown); ++i) {
                const mmu::ActivityMaskReadKind kind =
                    static_cast<mmu::ActivityMaskReadKind>(i);
                const std::size_t count = mmu::countActivityMaskReads(kind);
                if (count == 0) continue;
                char line[160];
                std::snprintf(line, sizeof(line), "read kind=%-10s sites=%zu",
                              activityMaskReadKindName(kind), count);
                c.log(line);
            }
            for (mmu::u32 mask = 1; mask != 0; mask <<= 1) {
                std::size_t equal = 0;
                std::size_t bit = 0;
                for (const mmu::RecoveredActivityMaskRead& row :
                     mmu::kRecoveredActivityMaskReads) {
                    if (row.mask != mask) continue;
                    if (row.kind == mmu::ActivityMaskReadKind::Equal) ++equal;
                    if (row.kind == mmu::ActivityMaskReadKind::Bit) ++bit;
                }
                if (equal == 0 && bit == 0) continue;
                char line[160];
                std::snprintf(line, sizeof(line),
                              "read mask=0x%x equal=%zu bit=%zu", mask,
                              equal, bit);
                c.log(line);
            }
        } else if (what == "maskread") {
            if (args.size() < 2) {
                c.log("usage: flowrun maskread <site va> [dynamic mask]");
                return;
            }
            char* end = nullptr;
            const unsigned long site = std::strtoul(args[1].c_str(), &end, 0);
            if (*end != '\0' || site > 0xFFFFFFFFul) {
                c.log("usage: flowrun maskread <site va> [dynamic mask]");
                return;
            }
            const mmu::RecoveredActivityMaskRead* row =
                mmu::findActivityMaskRead(static_cast<mmu::u32>(site));
            if (row == nullptr) {
                c.log("unknown activity-mask read site");
                return;
            }
            mmu::u32 dynamicMask = 0;
            if (args.size() > 2) {
                end = nullptr;
                const unsigned long value = std::strtoul(args[2].c_str(), &end, 0);
                if (*end != '\0' || value > 0xFFFFFFFFul) {
                    c.log("usage: flowrun maskread <site va> [dynamic mask]");
                    return;
                }
                dynamicMask = static_cast<mmu::u32>(value);
            }
            char line[256];
            std::snprintf(
                line, sizeof(line),
                "read call=0x%x func=0x%x kind=%s mask=0x%x operand='%s' "
                "owner=%s state='%.*s' dynamic=0x%x result=%d",
                row->callVa, row->functionVa,
                activityMaskReadKindName(row->kind), row->mask,
                row->operand.empty() ? "-" : row->operand.data(),
                row->owner.empty() ? "-" : row->owner.data(),
                static_cast<int>(row->stateEvidence.size()),
                row->stateEvidence.empty() ? "" : row->stateEvidence.data(),
                dynamicMask,
                flow.evaluateActivityMaskRead(static_cast<mmu::u32>(site),
                                              dynamicMask) ? 1 : 0);
            c.log(line);
        } else if (what == "boot") {
            flow.emitBoot();
            while (flow.pendingEventCount() != 0) {
                const mmu::FlowEvent boot = flow.pollEvent();
                if (boot.type == mmu::FlowEventType::ManagerBoot)
                    startFlowCutscene(state, boot.cutscenePath);
            }
            showStatus();
        } else if (what == "complete") {
            if (args.size() < 2) { c.log("usage: flowrun complete <loader id>"); return; }
            char* end = nullptr;
            const unsigned long id = std::strtoul(args[1].c_str(), &end, 0);
            if (*end != '\0') { c.log("usage: flowrun complete <loader id>"); return; }
            flow.completeStage(static_cast<mmu::u32>(id));
            // Drain a copy so the debug event log stays intact while the
            // recovered stage-completion cutscene prelude is started.
            mmu::GameFlowRuntime completionProbe = flow;
            while (completionProbe.pendingEventCount() != 0) {
                const mmu::FlowEvent completed = completionProbe.pollEvent();
                if (completed.type == mmu::FlowEventType::StageCompleted)
                    startFlowCutscene(state, completed.cutscenePath);
            }
            showStatus();
        } else if (what == "confirm" || what == "cancel" || what == "up" ||
                   what == "down" || what == "left" || what == "right") {
            mmu::FlowInput input{};
            if (what == "confirm") input.confirm = true;
            else if (what == "cancel") input.cancel = true;
            else if (what == "up") input.up = true;
            else if (what == "down") input.down = true;
            else if (what == "left") input.left = true;
            else input.right = true;
            flow.step(1.0f / 60.0f, input);
            showStatus();
        } else if (what == "step") {
            const float dt = args.size() > 1 ? std::strtof(args[1].c_str(), nullptr)
                                             : 1.0f / 60.0f;
            flow.step(dt, mmu::FlowInput{});
            showStatus();
        } else if (what == "cue") {
            if (args.size() < 2) { c.log("usage: flowrun cue <name>"); return; }
            const int index = findFlowCueIndex(args[1]);
            if (index < 0) { c.log("unknown flow cue: " + args[1]); return; }
            flow.postCue(static_cast<mmu::FlowCue>(index));
            showStatus();
        } else if (what == "levelcue") {
            if (args.size() < 2) { c.log("usage: flowrun levelcue <name>"); return; }
            const int index = findFlowLevelCueIndex(args[1]);
            if (index < 0) { c.log("unknown level cue: " + args[1]); return; }
            flow.postLevelCue(static_cast<mmu::FlowLevelCue>(index));
            showStatus();
        } else if (what == "events") {
            if (flow.pendingEventCount() == 0) { c.log("  (none)"); return; }
            char line[1024];
            while (flow.pendingEventCount() != 0) {
                const mmu::FlowEvent event = flow.pollEvent();
                char detail[768] = "";
                if (event.type == mmu::FlowEventType::MenuAction) {
                    const mmu::RecoveredMenuAction* action =
                        mmu::findMenuAction(event.actionCode);
                    std::snprintf(
                        detail, sizeof(detail),
                        " action=0x%02x kind=%s label='%.*s' table=0x%06x dest=%s",
                        event.actionCode,
                        action ? mmu::menuActionKindName(action->kind) : "?",
                        static_cast<int>(event.label.size()),
                        event.label.empty() ? "" : event.label.data(),
                        event.menuSupplierVa,
                        event.hasDestination
                            ? mmu::flowScreenName(event.destination)
                            : "-");
                    if (event.continueStageId != 0) {
                        char extra[160];
                        std::snprintf(
                            extra, sizeof(extra),
                            " continue=%u class='%.*s' ctor=0x%06x",
                            event.continueStageId,
                            static_cast<int>(event.continueStageClass.size()),
                            event.continueStageClass.empty()
                                ? ""
                                : event.continueStageClass.data(),
                            event.continueConstructorVa);
                        std::strncat(detail, extra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                    if (!event.cutscenePath.empty()) {
                        char extra[192];
                        std::snprintf(
                            extra, sizeof(extra), " cutscene='%.*s'",
                            static_cast<int>(event.cutscenePath.size()),
                            event.cutscenePath.data());
                        std::strncat(detail, extra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                    if (event.hasManagerFlag0x18) {
                        char extra[96];
                        std::snprintf(extra, sizeof(extra), " flag18=%d",
                                      event.managerFlag0x18 ? 1 : 0);
                        std::strncat(detail, extra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                    if (event.hasManagerStagePhase) {
                        char extra[96];
                        std::snprintf(extra, sizeof(extra), " stagephase=%u",
                                      event.managerStagePhase);
                        std::strncat(detail, extra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                    if (event.managerStageIdCleared) {
                        std::strncat(detail, " stageid=cleared",
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                    if (event.hasManagerPhase) {
                        char extra[96];
                        std::snprintf(extra, sizeof(extra), " mgrphase=%u",
                                      event.managerPhase);
                        std::strncat(detail, extra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                    if (event.exitGame) {
                        std::strncat(detail, " quit=1",
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                    if (event.character >= 0) {
                        char extra[96];
                        std::snprintf(extra, sizeof(extra), " character=%d",
                                      event.character);
                        std::strncat(detail, extra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                } else if (event.type ==
                           mmu::FlowEventType::ActivityMaskChanged) {
                    char extra[400];
                    std::snprintf(
                        extra, sizeof(extra),
                        " mask=0x%x prev=0x%x call=0x%x func=0x%x owner=%s evidence='%.*s'",
                        event.activityMaskValue, event.activityMaskPrevious,
                        event.activityMaskCallVa,
                        event.activityMaskFunctionVa,
                        event.activityMaskOwner.empty()
                            ? "-"
                            : event.activityMaskOwner.data(),
                        static_cast<int>(event.activityMaskEvidence.size()),
                        event.activityMaskEvidence.empty()
                            ? ""
                            : event.activityMaskEvidence.data());
                    std::strncat(detail, extra,
                                 sizeof(detail) - std::strlen(detail) - 1);
                } else if (event.type == mmu::FlowEventType::ManagerBoot) {
                    char extra[400];
                    std::snprintf(
                        extra, sizeof(extra),
                        " mask=0x%x play='%.*s' pending='%.*s' writes=%u",
                        event.bootActivityMask,
                        static_cast<int>(event.cutscenePath.size()),
                        event.cutscenePath.empty() ? ""
                                                   : event.cutscenePath.data(),
                        static_cast<int>(event.pendingCutscenePath.size()),
                        event.pendingCutscenePath.empty()
                            ? ""
                            : event.pendingCutscenePath.data(),
                        event.bootWriteCount);
                    std::strncat(detail, extra,
                                 sizeof(detail) - std::strlen(detail) - 1);
                    for (mmu::u8 i = 0; i < event.bootWriteCount && i < 3; ++i) {
                        char writeExtra[64];
                        std::snprintf(writeExtra, sizeof(writeExtra),
                                      " [+0x%02x]=0x%x",
                                      event.bootWriteOffsets[i],
                                      event.bootWriteValues[i]);
                        std::strncat(detail, writeExtra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                } else if (event.type ==
                           mmu::FlowEventType::LevelSetupApplied) {
                    if (event.levelSetup != nullptr) {
                        const mmu::RecoveredLevelSetup& row = *event.levelSetup;
                        char extra[512];
                        std::snprintf(
                            extra, sizeof(extra),
                            " stage=%u ctor=0x%06x class=%s cue=0x%06x@0x%06x "
                            "env=0x%06x@0x%06x obj=+0x%02x/+0x%02x "
                            "global=0x%06x<=0x%06x loop='%.*s' intro='%.*s'",
                            row.stageId, row.constructorVa,
                            row.className.empty() ? "-" : row.className.data(),
                            row.cueRegistrarVa, row.cueRegistrarCallVa,
                            row.environmentHelperVa, row.environmentCallVa,
                            row.stageWriteOffset, row.environmentWriteOffset,
                            row.globalVa, row.globalSourceVa,
                            static_cast<int>(row.loopCueList.size()),
                            row.loopCueList.empty() ? ""
                                                   : row.loopCueList.data(),
                            static_cast<int>(row.introCueList.size()),
                            row.introCueList.empty() ? ""
                                                    : row.introCueList.data());
                        std::strncat(detail, extra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                        char zeros[256] = " zero=";
                        for (mmu::u8 i = 0; i < row.zeroWriteCount &&
                             i < mmu::kLevelSetupMaxZeroWrites; ++i) {
                            char item[24];
                            std::snprintf(item, sizeof(item), "+0x%02x%s",
                                          row.zeroWriteOffsets[i],
                                          i + 1 == row.zeroWriteCount ? "" : ",");
                            std::strncat(zeros, item,
                                         sizeof(zeros) - std::strlen(zeros) - 1);
                        }
                        std::strncat(detail, zeros,
                                     sizeof(detail) - std::strlen(detail) - 1);
                    }
                } else if (event.type == mmu::FlowEventType::StageCompleted) {
                    char extra[320];
                    std::snprintf(
                        extra, sizeof(extra),
                        " loader=0x%02x action=0x%02x mgrphase=%u cue='%.*s'",
                        event.completedStageId, event.completedActionCode,
                        event.completedManagerPhase,
                        static_cast<int>(event.completionCue.size()),
                        event.completionCue.empty() ? ""
                                                    : event.completionCue.data());
                    std::strncat(detail, extra,
                                 sizeof(detail) - std::strlen(detail) - 1);
                    if (!event.cutscenePath.empty() ||
                        event.completionFlagId != 0 ||
                        event.completionManagerState != 0) {
                        char cutsceneExtra[400];
                        std::snprintf(
                            cutsceneExtra, sizeof(cutsceneExtra),
                            " cutscene='%.*s' flag=%u keybits=0x%02x state=%u cleared=%d writes=%u",
                            static_cast<int>(event.cutscenePath.size()),
                            event.cutscenePath.empty() ? ""
                                                       : event.cutscenePath.data(),
                            event.completionFlagId, event.keyedFlagBits,
                            event.completionManagerState,
                            event.managerScreensCleared ? 1 : 0,
                            event.completionWriteCount);
                        std::strncat(detail, cutsceneExtra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                        for (mmu::u8 i = 0;
                             i < event.completionWriteCount && i < 2; ++i) {
                            char writeExtra[64];
                            std::snprintf(writeExtra, sizeof(writeExtra),
                                          " [+0x%02x]=0x%x",
                                          event.completionWriteOffsets[i],
                                          event.completionWriteValues[i]);
                            std::strncat(detail, writeExtra,
                                         sizeof(detail) - std::strlen(detail) - 1);
                        }
                        if (event.completionPendingOffset != 0) {
                            char pendingExtra[320];
                            std::snprintf(
                                pendingExtra, sizeof(pendingExtra),
                                " pending='%.*s' va=0x%x off=+0x%x flag=+0x%x=0x%x",
                                static_cast<int>(
                                    event.completionPendingPath.size()),
                                event.completionPendingPath.empty()
                                    ? ""
                                    : event.completionPendingPath.data(),
                                event.completionPendingStringVa,
                                event.completionPendingOffset,
                                event.completionPendingFlagOffset,
                                event.completionPendingFlagValue);
                            std::strncat(
                                detail, pendingExtra,
                                sizeof(detail) - std::strlen(detail) - 1);
                        }
                    }
                    if (event.completionClearWriteCount != 0) {
                        char clearExtra[96];
                        std::snprintf(clearExtra, sizeof(clearExtra),
                                      " clear=[0x%x] w=%u",
                                      event.completionClearGlobalVa,
                                      event.completionClearWriteCount);
                        std::strncat(detail, clearExtra,
                                     sizeof(detail) - std::strlen(detail) - 1);
                        for (mmu::u8 i = 0;
                             i < event.completionClearWriteCount && i < 2;
                             ++i) {
                            char clearWrite[64];
                            std::snprintf(clearWrite, sizeof(clearWrite),
                                          " [+0x%03x:%u]=0x%x",
                                          event.completionClearWriteOffsets[i],
                                          event.completionClearWriteSizes[i],
                                          event.completionClearWriteValues[i]);
                            std::strncat(
                                detail, clearWrite,
                                sizeof(detail) - std::strlen(detail) - 1);
                        }
                    }
                }
                std::snprintf(line, sizeof(line),
                              "event %-10s screen=%s state=%s value=%d cue=%s levelcue=%s%s",
                              flowEventTypeName(event.type),
                              mmu::flowScreenName(event.screen),
                              mmu::flowStateName(event.state), event.value,
                              mmu::flowCueName(event.cue),
                              mmu::flowLevelCueName(event.levelCue), detail);
                c.log(line);
            }
        } else {
            c.log("usage: flowrun [status|reset [screen]|screen <name|#index>|supplier [0xVA]|flags ...|manager [phase] [opaque]|continue [stage id]|complete <loader id>|boot|keyflag <key> <0|1>|masks|mask <hex>|mask call <site va>|maskreads|maskread <site va> [dynamic mask]|confirm|cancel|up|down|left|right|step <sec>|cue <name>|levelcue <name>|events]");
        }
    });
    state.console.registerCommand("flowlevel", "recovered level-constructor setup: flowlevel [list|cues|stage <id>|ctor <va>|loop <cue>|intro <cue>|apply <id>]", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "list" : args[0];
        const auto printRow = [&c](const mmu::RecoveredLevelSetup& row) {
            char line[512];
            char zeros[192] = "";
            for (mmu::u8 i = 0; i < row.zeroWriteCount &&
                 i < mmu::kLevelSetupMaxZeroWrites; ++i) {
                char item[24];
                std::snprintf(item, sizeof(item), ",+0x%02x",
                              row.zeroWriteOffsets[i]);
                std::strncat(zeros, item, sizeof(zeros) - std::strlen(zeros) - 1);
            }
            std::snprintf(
                line, sizeof(line),
                "setup stage=%u ctor=0x%06x class=%s cue=0x%06x@0x%06x "
                "env=0x%06x@0x%06x global=0x%06x<=0x%06x loop='%.*s' "
                "intro='%.*s' zero=%s",
                row.stageId, row.constructorVa,
                row.className.empty() ? "-" : row.className.data(),
                row.cueRegistrarVa, row.cueRegistrarCallVa,
                row.environmentHelperVa, row.environmentCallVa, row.globalVa,
                row.globalSourceVa, static_cast<int>(row.loopCueList.size()),
                row.loopCueList.empty() ? "" : row.loopCueList.data(),
                static_cast<int>(row.introCueList.size()),
                row.introCueList.empty() ? "" : row.introCueList.data(),
                zeros[0] == ',' ? zeros + 1 : "-");
            c.log(line);
        };
        if (what == "list" || what == "all") {
            for (const mmu::RecoveredLevelSetup& row :
                 mmu::kRecoveredLevelSetups) {
                printRow(row);
            }
        } else if (what == "cues") {
            for (std::string_view cue : mmu::kRecoveredLevelSetupCues) {
                c.log(std::string(cue));
            }
        } else if (what == "stage" || what == "apply") {
            if (args.size() < 2) {
                c.log("usage: flowlevel stage <id> | apply <id>");
                return;
            }
            char* end = nullptr;
            const unsigned long value = std::strtoul(args[1].c_str(), &end, 0);
            if (*end != '\0' || value > 0xFFFFFFFFul) {
                c.log("usage: flowlevel stage <id> | apply <id>");
                return;
            }
            const mmu::u32 stageId = static_cast<mmu::u32>(value);
            const mmu::RecoveredLevelSetup* row = mmu::findLevelSetup(stageId);
            if (row == nullptr) {
                c.log("unknown stage id");
                return;
            }
            printRow(*row);
            if (what == "apply") {
                state.flowRuntime.applyLevelSetup(stageId);
                c.log("applied recovered level setup");
            }
        } else if (what == "ctor") {
            if (args.size() < 2) {
                c.log("usage: flowlevel ctor <va>");
                return;
            }
            char* end = nullptr;
            const unsigned long value = std::strtoul(args[1].c_str(), &end, 0);
            if (*end != '\0' || value > 0xFFFFFFFFul) {
                c.log("usage: flowlevel ctor <va>");
                return;
            }
            const mmu::RecoveredLevelSetup* row =
                mmu::findLevelSetupByConstructor(static_cast<mmu::u32>(value));
            if (row == nullptr) {
                c.log("unknown level constructor");
                return;
            }
            printRow(*row);
        } else if (what == "loop" || what == "intro") {
            if (args.size() < 2) {
                c.log("usage: flowlevel loop|intro <cue>");
                return;
            }
            const std::string_view cue = args[1];
            bool found = false;
            for (const mmu::RecoveredLevelSetup& row :
                 mmu::kRecoveredLevelSetups) {
                const bool matches = what == "loop"
                                         ? mmu::levelSetupHasLoopCue(row, cue)
                                         : mmu::levelSetupHasIntroCue(row, cue);
                if (!matches) continue;
                printRow(row);
                found = true;
            }
            if (!found) c.log("unknown level cue");
        } else {
            c.log("usage: flowlevel [list|cues|stage <id>|ctor <va>|loop <cue>|intro <cue>|apply <id>]");
        }
    });
    state.console.registerCommand("saveflags", "raw save-flag writer/reader evidence: saveflags [summary|writers|writer <va>|mask8|mask4|readers|key <n>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[320];
        auto emitWriter = [&c, &line](const mmu::RecoveredSaveFlagWriter& row) {
            std::snprintf(line, sizeof(line),
                          "  source=0x%06x cat=%-3u key=%-3u aux=%-3u enable=%u "
                          "rank=%u..%u",
                          row.sourceVa, row.category, row.key, row.aux,
                          row.enable, row.minRank, row.maxRank);
            c.log(line);
        };
        auto emitReader = [&c, &line](const mmu::RecoveredDirectSaveFlagReader& row) {
            std::snprintf(line, sizeof(line), "  source=0x%06x key=%-3u mask=0x%02x",
                          row.sourceVa, row.key, row.mask);
            c.log(line);
        };
        if (what == "writers") {
            for (const mmu::RecoveredSaveFlagWriter& row : mmu::kSaveFlagWriters) emitWriter(row);
        } else if (what == "writer") {
            if (args.size() < 2) { c.log("usage: saveflags writer <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::RecoveredSaveFlagWriter& row : mmu::kSaveFlagWriters) {
                if (row.sourceVa != va) continue;
                emitWriter(row);
                ++shown;
            }
            if (!shown) c.log("  (unknown writer source)");
        } else if (what == "mask8" || what == "mask4") {
            const bool wide = what == "mask8";
            const u32* keys = wide ? mmu::kSaveFlagMask8Keys : mmu::kSaveFlagMask4Keys;
            const std::size_t count = wide ? mmu::kSaveFlagMask8KeyCount
                                           : mmu::kSaveFlagMask4KeyCount;
            for (std::size_t i = 0; i < count; ++i) {
                std::snprintf(line, sizeof(line), "  %s key=%u", wide ? "mask8" : "mask4", keys[i]);
                c.log(line);
            }
        } else if (what == "readers") {
            for (const mmu::RecoveredDirectSaveFlagReader& row : mmu::kDirectSaveFlagReaders) emitReader(row);
        } else if (what == "key") {
            if (args.size() < 2) { c.log("usage: saveflags key <n>"); return; }
            const u32 key = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::RecoveredSaveFlagWriter& row : mmu::kSaveFlagWriters) {
                if (row.key != key) continue;
                emitWriter(row);
                ++shown;
            }
            for (const mmu::RecoveredDirectSaveFlagReader& row : mmu::kDirectSaveFlagReaders) {
                if (row.key != key) continue;
                emitReader(row);
                ++shown;
            }
            if (!shown) c.log("  (unknown key)");
        } else if (what != "summary") {
            c.log("usage: saveflags [summary|writers|writer <va>|mask8|mask4|readers|key <n>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "save flags manager=0x%06x writers=%zu mask8=%zu mask4=%zu readers=%zu "
                          "keyboard_limit=0x%x byte_bias=0x%x",
                          mmu::kSaveFlagManagerVa, mmu::kSaveFlagWriterCount,
                          mmu::kSaveFlagMask8KeyCount, mmu::kSaveFlagMask4KeyCount,
                          mmu::kDirectSaveFlagReaderCount,
                          mmu::kKeyedFlagKeyboardLimit,
                          mmu::kKeyedFlagManagerByteBias);
            c.log(line);
        }
    });
    state.console.registerCommand("flowmgr", "raw global-manager field evidence: flowmgr [summary|writes|accessors|accesses|offset <hex>|accessor <va>|func <va>|write <offset>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        const auto parseNumber = [&c](const std::string& text, mmu::u32& value) {
            char* end = nullptr;
            const unsigned long parsed = std::strtoul(text.c_str(), &end, 0);
            if (end == nullptr || *end != '\0' || parsed > 0xFFFFFFFFul) {
                return false;
            }
            value = static_cast<mmu::u32>(parsed);
            return true;
        };
        const auto printWrite = [&c](const mmu::ManagerLiteralWrite& row) {
            char line[192];
            std::snprintf(line, sizeof(line),
                          "write func=0x%06x obj=0x%06x +0x%03x size=%u "
                          "value=0x%x line=%u",
                          row.functionVa, row.objectGlobalVa, row.offset,
                          row.size, row.value, row.lineNumber);
            c.log(line);
        };
        const auto printAccessor = [&c](const mmu::ManagerAccessor& row) {
            char forward[32] = "-";
            if (row.forwardedVa != 0) {
                std::snprintf(forward, sizeof(forward), "0x%06x",
                              row.forwardedVa);
            }
            char line[224];
            std::snprintf(line, sizeof(line),
                          "accessor func=0x%06x obj=0x%06x +0x%03x size=%u "
                          "%-5s calls=%u forward=%s line=%u",
                          row.functionVa, row.objectGlobalVa, row.offset,
                          row.size, managerFieldAccessKindName(row.kind),
                          row.callCount, forward, row.lineNumber);
            c.log(line);
        };
        const auto printAccess = [&c](const mmu::ManagerFieldAccess& row) {
            char line[192];
            std::snprintf(line, sizeof(line),
                          "access func=0x%06x obj=0x%06x +0x%03x size=%u "
                          "%-5s count=%u line=%u",
                          row.functionVa, row.objectGlobalVa, row.offset,
                          row.size, managerFieldAccessKindName(row.kind),
                          row.count, row.lineNumber);
            c.log(line);
        };
        if (what == "summary" || what == "all") {
            char line[192];
            std::snprintf(line, sizeof(line),
                          "manager=0x%06x completion_object=0x%06x writes=%zu "
                          "accessors=%zu accesses=%zu",
                          mmu::kManagerGlobalVa,
                          mmu::kManagerCompletionObjectGlobalVa,
                          mmu::kManagerLiteralWriteCount,
                          mmu::kManagerAccessorCount,
                          mmu::kManagerFieldAccessCount);
            c.log(line);
        } else if (what == "writes") {
            for (const mmu::ManagerLiteralWrite& row :
                 mmu::kManagerLiteralWrites) {
                printWrite(row);
            }
        } else if (what == "accessors") {
            for (const mmu::ManagerAccessor& row : mmu::kManagerAccessors) {
                printAccessor(row);
            }
        } else if (what == "accesses") {
            for (const mmu::ManagerFieldAccess& row :
                 mmu::kManagerFieldAccesses) {
                printAccess(row);
            }
        } else if (what == "offset" || what == "write") {
            if (args.size() < 2) {
                c.log("usage: flowmgr offset|write <hex>");
                return;
            }
            mmu::u32 value = 0;
            if (!parseNumber(args[1], value)) {
                c.log("usage: flowmgr offset|write <hex>");
                return;
            }
            bool found = false;
            for (const mmu::ManagerLiteralWrite& row :
                 mmu::kManagerLiteralWrites) {
                if (row.offset != value) continue;
                printWrite(row);
                found = true;
            }
            if (what == "offset") {
                for (const mmu::ManagerAccessor& row : mmu::kManagerAccessors) {
                    if (row.offset != value) continue;
                    printAccessor(row);
                    found = true;
                }
                for (const mmu::ManagerFieldAccess& row :
                     mmu::kManagerFieldAccesses) {
                    if (row.offset != value) continue;
                    printAccess(row);
                    found = true;
                }
            }
            if (!found) c.log("unknown manager field offset");
        } else if (what == "accessor") {
            if (args.size() < 2) {
                c.log("usage: flowmgr accessor <va>");
                return;
            }
            mmu::u32 va = 0;
            if (!parseNumber(args[1], va)) {
                c.log("usage: flowmgr accessor <va>");
                return;
            }
            bool found = false;
            for (const mmu::ManagerAccessor& row : mmu::kManagerAccessors) {
                if (row.functionVa != va) continue;
                printAccessor(row);
                found = true;
            }
            if (!found) c.log("unknown manager accessor");
        } else if (what == "func") {
            if (args.size() < 2) {
                c.log("usage: flowmgr func <va>");
                return;
            }
            mmu::u32 va = 0;
            if (!parseNumber(args[1], va)) {
                c.log("usage: flowmgr func <va>");
                return;
            }
            bool found = false;
            for (const mmu::ManagerLiteralWrite& row :
                 mmu::kManagerLiteralWrites) {
                if (row.functionVa != va) continue;
                printWrite(row);
                found = true;
            }
            for (const mmu::ManagerFieldAccess& row :
                 mmu::kManagerFieldAccesses) {
                if (row.functionVa != va) continue;
                printAccess(row);
                found = true;
            }
            if (!found) c.log("unknown manager function");
        } else {
            c.log("usage: flowmgr [summary|writes|accessors|accesses|offset <hex>|accessor <va>|func <va>|write <offset>]");
        }
    });
    state.console.registerCommand("flownames", "query registered state names: flownames [filter]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string needle = args.empty() ? std::string() : args[0];
        std::size_t shown = 0;
        for (const mmu::RegisteredStateName& row : mmu::kRegisteredStateNames) {
            const std::string name = std::string(row.name);
            if (!needle.empty() && name.find(needle) == std::string::npos)
                continue;
            char line[160];
            std::snprintf(line, sizeof(line), "0x%08x  %s", row.hash, name.c_str());
            c.log(line);
            ++shown;
        }
        char summary[160];
        std::snprintf(summary, sizeof(summary),
                      "registered names=%zu shown=%zu",
                      mmu::kRegisteredStateNameCount, shown);
        c.log(summary);
    });
    state.console.registerCommand("flowtrans", "query observed setter evidence: flowtrans [all|to <name>|class <name>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "all" : args[0];
        const auto printRow = [&](const mmu::RegisteredStateTransition& row) {
            char line[256];
            std::snprintf(line, sizeof(line),
                          "  %-22s slot=%-2d func=0x%06x call=0x%06x %-12s hash=0x%08x %s",
                          std::string(row.className).c_str(), row.vtableSlot,
                          row.functionVa, row.callVa,
                          mmu::flowTransitionKindName(row.flags), row.targetHash,
                          std::string(row.targetName).c_str());
            c.log(line);
        };
        if (what == "to") {
            if (args.size() < 2) { c.log("usage: flowtrans to <state|cue>"); return; }
            const std::size_t count = mmu::countTransitionsTo(args[1]);
            if (count == 0) { c.log("  (none)"); return; }
            for (const mmu::RegisteredStateTransition& row :
                 mmu::kRegisteredStateTransitions) {
                if (row.targetName == args[1]) printRow(row);
            }
        } else if (what == "class") {
            if (args.size() < 2) { c.log("usage: flowtrans class <name>"); return; }
            const std::size_t count = mmu::countTransitionsForClass(args[1]);
            if (count == 0) { c.log("  (none)"); return; }
            for (const mmu::RegisteredStateTransition& row :
                 mmu::kRegisteredStateTransitions) {
                if (row.className == args[1]) printRow(row);
            }
        } else if (what == "all") {
            std::size_t setStateCount = 0, cueCount = 0;
            for (const mmu::RegisteredStateTransition& row :
                 mmu::kRegisteredStateTransitions) {
                if ((row.flags & mmu::kFlowTransitionSetState) != 0) ++setStateCount;
                if ((row.flags & mmu::kFlowTransitionCue) != 0) ++cueCount;
            }
            char line[192];
            std::snprintf(line, sizeof(line),
                          "flow transitions=%zu setState=%zu cue=%zu",
                          mmu::kRegisteredStateTransitionCount,
                          setStateCount, cueCount);
            c.log(line);
        } else {
            c.log("usage: flowtrans [all|to <name>|class <name>]");
        }
    });
    state.console.registerCommand("flowgraph", "recovered ordered state graphs: flowgraph [initial|to <name>|<class>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        if (what == "initial") {
            for (std::size_t i = 0; i < mmu::kFlowScreenCount; ++i) {
                const mmu::FlowScreen screen = static_cast<mmu::FlowScreen>(i);
                const std::string_view initial = mmu::graphInitialState(screen);
                char line[192];
                std::snprintf(line, sizeof(line), "  %-30s %s",
                              mmu::flowScreenName(screen),
                              initial.empty() ? "(not recovered)" : std::string(initial).c_str());
                c.log(line);
            }
        } else if (what == "to") {
            if (args.size() < 2) { c.log("usage: flowgraph to <name>"); return; }
            for (const mmu::RegisteredStateGraphEdge& edge :
                 mmu::kRegisteredStateGraphEdges) {
                if (edge.targetName != args[1]) continue;
                char line[192];
                std::snprintf(line, sizeof(line),
                              "  %-22s slot=%-2d func=0x%06x call=0x%06x %s",
                              std::string(edge.className).c_str(), edge.vtableSlot,
                              edge.functionVa, edge.callVa,
                              std::string(edge.targetName).c_str());
                c.log(line);
            }
        } else if (what != "summary") {
            std::string states;
            std::string initial;
            for (const mmu::RegisteredStateGraphRow& row :
                 mmu::kRegisteredStateGraph) {
                if (row.className != what) continue;
                if (!states.empty()) states += ", ";
                states += row.stateName;
                if (row.isInitial != 0) initial = row.stateName;
            }
            if (states.empty()) { c.log("  (unknown class)"); return; }
            char line[512];
            std::snprintf(line, sizeof(line), "%s: initial=%s states=%s",
                          what.c_str(), initial.c_str(), states.c_str());
            c.log(line);
            for (const mmu::RegisteredStateGraphEdge& edge :
                 mmu::kRegisteredStateGraphEdges) {
                if (edge.className != what) continue;
                std::snprintf(line, sizeof(line),
                              "  slot=%-2d func=0x%06x call=0x%06x %-8s %s",
                              edge.vtableSlot, edge.functionVa, edge.callVa,
                              edge.flags == 1 ? "setState" : "cue",
                              std::string(edge.targetName).c_str());
                c.log(line);
            }
        } else {
            char line[192];
            std::snprintf(line, sizeof(line),
                          "state classes=%zu regions=%zu states=%zu edges=%zu",
                          mmu::stateGraphClassCount(),
                          mmu::stateGraphRegionCount(),
                          mmu::kRegisteredStateGraphCount,
                          mmu::kRegisteredStateGraphEdgeCount);
            c.log(line);
        }
    });
    state.console.registerCommand("enemygraph", "joined RTTI enemy state graphs: enemygraph [initial|slots|to <name>|damage|<class>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        if (what == "slots") {
            for (const mmu::EnemyStateGraphMachine& machine :
                 mmu::kEnemyStateGraphMachines) {
                for (u32 index = 0; index < machine.stateSlotCount; ++index) {
                    const mmu::EnemyStateGraphStateSlots& entry =
                        machine.stateSlots[index];
                    std::snprintf(line, sizeof(line), "  %-16s %-10s %u..%u",
                                  std::string(machine.className).c_str(),
                                  std::string(entry.stateName).c_str(),
                                  entry.firstSlot, entry.lastSlot);
                    c.log(line);
                }
            }
        } else if (what == "initial") {
            for (const mmu::EnemyStateGraphMachine& machine :
                 mmu::kEnemyStateGraphMachines) {
                std::snprintf(line, sizeof(line), "  %-16s %s (setState 0x%06x)",
                              std::string(machine.className).c_str(),
                              std::string(machine.initialState).c_str(),
                              machine.initialSetStateVa);
                c.log(line);
            }
        } else if (what == "damage") {
            for (const mmu::EnemyStateGraphMachine& machine :
                 mmu::kEnemyStateGraphMachines) {
                std::snprintf(line, sizeof(line),
                              "  %-16s call=0x%06x handler=0x%06x bits=0x%08x",
                              std::string(machine.className).c_str(),
                              machine.damageCallVa, machine.damageHandlerVa,
                              machine.damageResultCodeBits);
                c.log(line);
            }
        } else if (what == "to") {
            if (args.size() < 2) { c.log("usage: enemygraph to <name>"); return; }
            for (const mmu::EnemyStateGraphMachine& machine :
                 mmu::kEnemyStateGraphMachines) {
                for (u32 index = 0; index < machine.edgeCount; ++index) {
                    const mmu::EnemyStateGraphEdge& edge = machine.edges[index];
                    if (edge.targetName != args[1]) continue;
                    std::snprintf(line, sizeof(line),
                                  "  %-16s slot=%-2u func=0x%06x call=0x%06x "
                                  "%-8s hash=0x%08x owner=%s",
                                  std::string(machine.className).c_str(),
                                  edge.vtableSlot, edge.functionVa, edge.callVa,
                                  edge.isCue ? "cue" : "setState",
                                  edge.targetHash,
                                  std::string(edge.ownerState).c_str());
                    c.log(line);
                }
            }
        } else if (what != "summary") {
            const mmu::EnemyStateGraphMachine* machine =
                mmu::enemyStateGraphMachine(what);
            if (machine == nullptr) { c.log("  (unknown class)"); return; }
            std::string states;
            for (u32 index = 0; index < machine->stateCount; ++index) {
                if (!states.empty()) states += ", ";
                states += machine->states[index];
            }
            std::snprintf(line, sizeof(line),
                          "%s vft=0x%06x ctor=0x%06x initial=%s states=%s",
                          what.c_str(), machine->vftableVa,
                          machine->constructorVa,
                          std::string(machine->initialState).c_str(),
                          states.c_str());
            c.log(line);
            for (u32 index = 0; index < machine->stateSlotCount; ++index) {
                const mmu::EnemyStateGraphStateSlots& entry =
                    machine->stateSlots[index];
                std::snprintf(line, sizeof(line), "  slots %-10s %u..%u",
                              std::string(entry.stateName).c_str(),
                              entry.firstSlot, entry.lastSlot);
                c.log(line);
            }
            for (u32 index = 0; index < machine->edgeCount; ++index) {
                const mmu::EnemyStateGraphEdge& edge = machine->edges[index];
                std::snprintf(line, sizeof(line),
                              "  slot=%-2u func=0x%06x call=0x%06x %-8s "
                              "hash=0x%08x %s owner=%s",
                              edge.vtableSlot, edge.functionVa, edge.callVa,
                              edge.isCue ? "cue" : "setState", edge.targetHash,
                              std::string(edge.targetName).c_str(),
                              std::string(edge.ownerState).c_str());
                c.log(line);
            }
        } else {
            std::size_t states = 0, edges = 0, slots = 0;
            for (const mmu::EnemyStateGraphMachine& machine :
                 mmu::kEnemyStateGraphMachines) {
                states += machine.stateCount;
                slots += machine.stateSlotCount;
                edges += machine.edgeCount;
            }
            std::snprintf(line, sizeof(line),
                          "enemy state machines=%zu states=%zu slots=%zu edges=%zu",
                          std::size(mmu::kEnemyStateGraphMachines), states,
                          slots, edges);
            c.log(line);
        }
    });
    state.console.registerCommand("enemyfacts", "verbatim enemy edge branch facts: enemyfacts [summary|constants <class>|call <va>|<class>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        if (what == "constants") {
            if (args.size() < 2) { c.log("usage: enemyfacts constants <class>"); return; }
            for (const mmu::EnemyEdgeFact& fact : mmu::kEnemyEdgeFacts) {
                if (fact.className != args[1]) continue;
                for (u32 index = 0; index < fact.constantCount; ++index) {
                    const mmu::EnemyEdgeConstant& constant = fact.constants[index];
                    std::snprintf(line, sizeof(line),
                                  "  %-16s call=0x%06x 0x%06x = %.17g",
                                  std::string(fact.className).c_str(),
                                  fact.callVa, constant.address, constant.value);
                    c.log(line);
                }
            }
        } else if (what == "call") {
            if (args.size() < 2) { c.log("usage: enemyfacts call <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            for (const mmu::EnemyEdgeFact& fact : mmu::kEnemyEdgeFacts) {
                if (fact.callVa != va) continue;
                std::snprintf(line, sizeof(line),
                              "  %-16s slot=%-2u func=0x%06x %s -> %s "
                              "owner=%s key=0x%06x %s",
                              std::string(fact.className).c_str(),
                              fact.vtableSlot, fact.functionVa,
                              fact.isCue ? "cue" : "setState",
                              std::string(fact.targetName).c_str(),
                              std::string(fact.ownerState).c_str(),
                              fact.setStateKeyVa,
                              std::string(fact.setStateKey).c_str());
                c.log(line);
                for (u32 index = 0; index < fact.comparisonCount; ++index) {
                    const mmu::EnemyEdgeComparison& cmp = fact.comparisons[index];
                    std::snprintf(line, sizeof(line), "    0x%06x %-6s %s",
                                  cmp.instructionVa,
                                  std::string(cmp.mnemonic).c_str(),
                                  std::string(cmp.operands).c_str());
                    c.log(line);
                }
            }
        } else if (what != "summary") {
            std::size_t shown = 0;
            for (const mmu::EnemyEdgeFact& fact : mmu::kEnemyEdgeFacts) {
                if (fact.className != what) continue;
                std::snprintf(line, sizeof(line),
                              "  slot=%-2u call=0x%06x %s -> %s owner=%s "
                              "cmps=%u consts=%u key=0x%06x %s",
                              fact.vtableSlot, fact.callVa,
                              fact.isCue ? "cue" : "setState",
                              std::string(fact.targetName).c_str(),
                              std::string(fact.ownerState).c_str(),
                              fact.comparisonCount, fact.constantCount,
                              fact.setStateKeyVa,
                              std::string(fact.setStateKey).c_str());
                c.log(line);
                ++shown;
            }
            if (!shown) c.log("  (unknown class)");
        } else {
            std::size_t comparisons = 0, constants = 0;
            for (const mmu::EnemyEdgeFact& fact : mmu::kEnemyEdgeFacts) {
                comparisons += fact.comparisonCount;
                constants += fact.constantCount;
            }
            std::snprintf(line, sizeof(line),
                          "enemy edge facts=%zu comparisons=%zu constants=%zu",
                          std::size(mmu::kEnemyEdgeFacts), comparisons,
                          constants);
            c.log(line);
        }
    });
    state.console.registerCommand("flowfacts", "verbatim menu edge branch facts: flowfacts [summary|constants <class>|call <va>|<class>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        if (what == "constants") {
            if (args.size() < 2) { c.log("usage: flowfacts constants <class>"); return; }
            for (const mmu::FlowEdgeFact& fact : mmu::kFlowEdgeFacts) {
                if (fact.className != args[1]) continue;
                for (u32 index = 0; index < fact.constantCount; ++index) {
                    const mmu::FlowEdgeConstant& constant = fact.constants[index];
                    std::snprintf(line, sizeof(line),
                                  "  %-26s call=0x%06x 0x%06x = %.17g",
                                  std::string(fact.className).c_str(),
                                  fact.callVa, constant.address, constant.value);
                    c.log(line);
                }
            }
        } else if (what == "call") {
            if (args.size() < 2) { c.log("usage: flowfacts call <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            for (const mmu::FlowEdgeFact& fact : mmu::kFlowEdgeFacts) {
                if (fact.callVa != va) continue;
                std::snprintf(line, sizeof(line),
                              "  %-26s slot=%-2u func=0x%06x %s -> %s "
                              "key=0x%06x %s",
                              std::string(fact.className).c_str(),
                              fact.vtableSlot, fact.functionVa,
                              fact.isCue ? "cue" : "setState",
                              std::string(fact.targetName).c_str(),
                              fact.setStateKeyVa,
                              std::string(fact.setStateKey).c_str());
                c.log(line);
                for (u32 index = 0; index < fact.comparisonCount; ++index) {
                    const mmu::FlowEdgeComparison& cmp = fact.comparisons[index];
                    std::snprintf(line, sizeof(line), "    0x%06x %-6s %s",
                                  cmp.instructionVa,
                                  std::string(cmp.mnemonic).c_str(),
                                  std::string(cmp.operands).c_str());
                    c.log(line);
                }
            }
        } else if (what != "summary") {
            std::size_t shown = 0;
            for (const mmu::FlowEdgeFact& fact : mmu::kFlowEdgeFacts) {
                if (fact.className != what) continue;
                std::snprintf(line, sizeof(line),
                              "  slot=%-2u call=0x%06x %s -> %s "
                              "cmps=%u consts=%u key=0x%06x %s",
                              fact.vtableSlot, fact.callVa,
                              fact.isCue ? "cue" : "setState",
                              std::string(fact.targetName).c_str(),
                              fact.comparisonCount, fact.constantCount,
                              fact.setStateKeyVa,
                              std::string(fact.setStateKey).c_str());
                c.log(line);
                ++shown;
            }
            if (!shown) c.log("  (unknown class)");
        } else {
            std::size_t comparisons = 0, constants = 0;
            for (const mmu::FlowEdgeFact& fact : mmu::kFlowEdgeFacts) {
                comparisons += fact.comparisonCount;
                constants += fact.constantCount;
            }
            std::snprintf(line, sizeof(line),
                          "flow edge facts=%zu comparisons=%zu constants=%zu",
                          std::size(mmu::kFlowEdgeFacts), comparisons,
                          constants);
            c.log(line);
        }
    });
    state.console.registerCommand("pauseres", "verbatim PauseMenuStatus inline resources: pauseres [summary|list|key <hex>|path <text>|kind <Icon|Colors>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        auto emit = [&c, &line](const mmu::PauseWeaponResource& resource) {
            std::snprintf(line, sizeof(line),
                          "  seq=%-2u push=0x%06x string=0x%06x key=0x%08x %-6s %s",
                          resource.sequence, resource.pushVa, resource.stringVa,
                          resource.key, std::string(resource.kind).c_str(),
                          std::string(resource.path).c_str());
            c.log(line);
        };
        if (what == "key") {
            if (args.size() < 2) { c.log("usage: pauseres key <hex>"); return; }
            const u32 key = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::PauseWeaponResource& resource : mmu::kPauseWeaponResources) {
                if (resource.key != key) continue;
                emit(resource);
                ++shown;
            }
            if (!shown) c.log("  (unknown key)");
        } else if (what == "path") {
            if (args.size() < 2) { c.log("usage: pauseres path <text>"); return; }
            std::size_t shown = 0;
            for (const mmu::PauseWeaponResource& resource : mmu::kPauseWeaponResources) {
                if (std::string(resource.path).find(args[1]) == std::string::npos)
                    continue;
                emit(resource);
                ++shown;
            }
            if (!shown) c.log("  (unknown path)");
        } else if (what == "kind") {
            if (args.size() < 2) { c.log("usage: pauseres kind <Icon|Colors>"); return; }
            std::size_t shown = 0;
            for (const mmu::PauseWeaponResource& resource : mmu::kPauseWeaponResources) {
                if (std::string(resource.kind) != args[1]) continue;
                emit(resource);
                ++shown;
            }
            if (!shown) c.log("  (unknown kind)");
        } else if (what == "list") {
            for (const mmu::PauseWeaponResource& resource : mmu::kPauseWeaponResources)
                emit(resource);
        } else if (what != "summary") {
            c.log("usage: pauseres [summary|list|key <hex>|path <text>|kind <Icon|Colors>]");
        } else {
            std::size_t icons = 0, colors = 0;
            for (const mmu::PauseWeaponResource& resource : mmu::kPauseWeaponResources) {
                if (resource.kind == "Icon") ++icons;
                else if (resource.kind == "Colors") ++colors;
            }
            std::snprintf(line, sizeof(line),
                          "pause inline resources=%zu icons=%zu colors=%zu",
                          std::size(mmu::kPauseWeaponResources), icons, colors);
            c.log(line);
        }
    });
    state.console.registerCommand("menures", "verbatim inline menu .xspr pushes: menures [summary|list|class <name>|key <hex>|path <text>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[448];
        auto emit = [&c, &line](const mmu::InlineMenuResource& resource) {
            std::snprintf(line, sizeof(line),
                          "  %-30s seq=%-2u push=0x%06x string=0x%06x key=0x%08x %s",
                          std::string(resource.className).c_str(),
                          resource.sequence, resource.pushVa, resource.stringVa,
                          resource.key, std::string(resource.path).c_str());
            c.log(line);
        };
        if (what == "class") {
            if (args.size() < 2) { c.log("usage: menures class <name>"); return; }
            std::size_t shown = 0;
            for (const mmu::InlineMenuResource& resource : mmu::kInlineMenuResources) {
                if (std::string(resource.className) != args[1]) continue;
                emit(resource);
                ++shown;
            }
            if (!shown) c.log("  (unknown class)");
        } else if (what == "key") {
            if (args.size() < 2) { c.log("usage: menures key <hex>"); return; }
            const u32 key = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::InlineMenuResource& resource : mmu::kInlineMenuResources) {
                if (resource.key != key) continue;
                emit(resource);
                ++shown;
            }
            if (!shown) c.log("  (unknown key)");
        } else if (what == "path") {
            if (args.size() < 2) { c.log("usage: menures path <text>"); return; }
            std::size_t shown = 0;
            for (const mmu::InlineMenuResource& resource : mmu::kInlineMenuResources) {
                if (std::string(resource.path).find(args[1]) == std::string::npos)
                    continue;
                emit(resource);
                ++shown;
            }
            if (!shown) c.log("  (unknown path)");
        } else if (what == "list") {
            for (const mmu::InlineMenuResource& resource : mmu::kInlineMenuResources)
                emit(resource);
        } else if (what != "summary") {
            c.log("usage: menures [summary|list|class <name>|key <hex>|path <text>]");
        } else {
            std::size_t uniquePaths = 0;
            std::vector<std::string_view> paths;
            for (const mmu::InlineMenuResource& resource : mmu::kInlineMenuResources)
                paths.push_back(resource.path);
            std::sort(paths.begin(), paths.end());
            uniquePaths = static_cast<std::size_t>(std::distance(
                paths.begin(), std::unique(paths.begin(), paths.end())));
            std::snprintf(line, sizeof(line),
                          "inline menu resources=%zu classes=20 paths=%zu",
                          std::size(mmu::kInlineMenuResources), uniquePaths);
            c.log(line);
        }
    });
    state.console.registerCommand("rtticlasses", "raw RTTI class-name lines: rtticlasses [summary|list [filter]|name <text>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[512];
        auto emit = [&c, &line](std::size_t index, std::string_view name) {
            std::snprintf(line, sizeof(line), "  %zu %.*s", index,
                          static_cast<int>(name.size()), name.data());
            c.log(line);
        };
        if (what == "list") {
            const std::string filter = args.size() >= 2 ? args[1] : std::string();
            std::size_t shown = 0;
            for (std::size_t i = 0; i < mmu::kRttiClassNameCount; ++i) {
                if (!filter.empty() &&
                    std::string(mmu::kRttiClassNames[i]).find(filter) ==
                        std::string::npos)
                    continue;
                emit(i, mmu::kRttiClassNames[i]);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "rtti class names shown=%zu", shown);
            c.log(line);
        } else if (what == "name") {
            if (args.size() < 2) { c.log("usage: rtticlasses name <text>"); return; }
            const std::size_t index = mmu::rttiClassNameIndex(args[1]);
            if (index == mmu::kRttiClassNameCount) c.log("  (unknown name)");
            else emit(index, mmu::kRttiClassNames[index]);
        } else if (what != "summary") {
            c.log("usage: rtticlasses [summary|list [filter]|name <text>]");
        } else {
            const std::string_view first = mmu::kRttiClassNames[0];
            const std::string_view last =
                mmu::kRttiClassNames[mmu::kRttiClassNameCount - 1];
            std::snprintf(line, sizeof(line),
                          "rtti class names=%zu first=%.*s last=%.*s",
                          mmu::kRttiClassNameCount,
                          static_cast<int>(first.size()), first.data(),
                          static_cast<int>(last.size()), last.data());
            c.log(line);
        }
    });
    state.console.registerCommand("rtti", "raw RTTI/vtable evidence: rtti [summary|classes [filter]|class <name>|vftable <hex>|slot <hex>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        auto emit = [&c, &line](const mmu::RttiVtable& table) {
            std::snprintf(line, sizeof(line), "  %-28s vftable=0x%06x slots=%u",
                          std::string(table.className).c_str(), table.vftableVa,
                          table.slotCount);
            c.log(line);
        };
        auto emitSlot = [&c, &line](const mmu::RttiVtable& table, u32 index) {
            std::snprintf(line, sizeof(line), "  %-28s slot=%-2u 0x%06x",
                          std::string(table.className).c_str(), index,
                          mmu::rttiVtableSlot(table, index));
            c.log(line);
        };
        if (what == "classes") {
            const std::string filter = args.size() >= 2 ? args[1] : std::string();
            std::size_t shown = 0;
            for (const mmu::RttiVtable& table : mmu::kRttiVtables) {
                if (!filter.empty() &&
                    std::string(table.className).find(filter) == std::string::npos &&
                    std::string(table.mangledName).find(filter) == std::string::npos)
                    continue;
                emit(table);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "rtti classes shown=%zu", shown);
            c.log(line);
        } else if (what == "class") {
            if (args.size() < 2) { c.log("usage: rtti class <name>"); return; }
            const mmu::RttiVtable* table = mmu::rttiVtable(args[1]);
            if (!table) { c.log("  (unknown class)"); return; }
            emit(*table);
            std::snprintf(line, sizeof(line), "  mangled=%s",
                          std::string(table->mangledName).c_str());
            c.log(line);
            for (u32 i = 0; i < table->slotCount; ++i) emitSlot(*table, i);
        } else if (what == "vftable") {
            if (args.size() < 2) { c.log("usage: rtti vftable <hex>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            const mmu::RttiVtable* table = mmu::rttiVtableByVa(va);
            if (table) emit(*table);
            else c.log("  (unknown vftable)");
        } else if (what == "slot") {
            if (args.size() < 2) { c.log("usage: rtti slot <hex>"); return; }
            const u32 target = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::RttiVtable& table : mmu::kRttiVtables) {
                for (u32 i = 0; i < table.slotCount; ++i) {
                    if (mmu::rttiVtableSlot(table, i) != target) continue;
                    emitSlot(table, i);
                    ++shown;
                }
            }
            std::snprintf(line, sizeof(line), "rtti slot matches=%zu", shown);
            c.log(line);
        } else if (what != "summary") {
            c.log("usage: rtti [summary|classes [filter]|class <name>|vftable <hex>|slot <hex>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "rtti type_descriptors=%u cols=%u vtables=%zu slots=%zu",
                          mmu::kRttiTypeDescriptorCount,
                          mmu::kRttiCompleteObjectLocatorCount,
                          mmu::kRttiVtableCount, mmu::kRttiVtableSlotCount);
            c.log(line);
        }
    });
    state.console.registerCommand("stateslots", "raw state thunk/slot blocks: stateslots [summary|class <name>|state <name>|incomplete]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[512];
        auto emit = [&c, &line](const mmu::StateSlotBlock& block) {
            std::snprintf(line, sizeof(line),
                          "  %-28s %-24s reg=0x%06x count=%u slots=%u,%u,%u "
                          "thunks=0x%06x,0x%06x,0x%06x %s",
                          std::string(block.className).c_str(),
                          std::string(block.stateName).c_str(),
                          block.registrationVa, block.slotCount,
                          block.slots[0], block.slots[1], block.slots[2],
                          block.thunks[0], block.thunks[1], block.thunks[2],
                          block.complete ? "complete" : "incomplete");
            c.log(line);
        };
        if (what == "class" || what == "state") {
            if (args.size() < 2) {
                c.log(what == "class" ? "usage: stateslots class <name>"
                                      : "usage: stateslots state <name>");
                return;
            }
            std::size_t shown = 0;
            for (const mmu::StateSlotBlock& block : mmu::kStateSlotBlocks) {
                if (what == "class" && std::string(block.className) != args[1])
                    continue;
                if (what == "state" && std::string(block.stateName) != args[1])
                    continue;
                emit(block);
                ++shown;
            }
            if (!shown) c.log("  (unknown filter)");
        } else if (what == "incomplete") {
            for (const mmu::StateSlotBlock& block : mmu::kStateSlotBlocks)
                if (!block.complete) emit(block);
        } else if (what != "summary") {
            c.log("usage: stateslots [summary|class <name>|state <name>|incomplete]");
        } else {
            std::size_t complete = 0;
            std::vector<std::string_view> classes;
            for (const mmu::StateSlotBlock& block : mmu::kStateSlotBlocks) {
                if (block.complete) ++complete;
                classes.push_back(block.className);
            }
            std::sort(classes.begin(), classes.end());
            classes.erase(std::unique(classes.begin(), classes.end()), classes.end());
            std::snprintf(line, sizeof(line),
                          "state slot blocks=%zu classes=%zu complete=%zu incomplete=%zu",
                          std::size(mmu::kStateSlotBlocks), classes.size(),
                          complete, std::size(mmu::kStateSlotBlocks) - complete);
            c.log(line);
        }
    });
    state.console.registerCommand("outro", "raw outro/handoff evidence: outro [summary|calls|call <va>|owner <name>|strings|facts|func <va>|offset <hex>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[768];
        auto callKind = [](mmu::OutroHandoffKind kind) -> const char* {
            switch (kind) {
            case mmu::OutroHandoffKind::CueRegistrar: return "CueRegistrar";
            case mmu::OutroHandoffKind::StateSetter: return "StateSetter";
            case mmu::OutroHandoffKind::NamedStateCall: return "NamedStateCall";
            case mmu::OutroHandoffKind::CutsceneLoader: return "CutsceneLoader";
            case mmu::OutroHandoffKind::DirectHandoff: return "DirectHandoff";
            case mmu::OutroHandoffKind::EnvironmentHelper: return "EnvironmentHelper";
            case mmu::OutroHandoffKind::StateQueryCall: return "StateQueryCall";
            }
            return "?";
        };
        auto factKind = [](mmu::OutroHandoffFactKind kind) -> const char* {
            switch (kind) {
            case mmu::OutroHandoffFactKind::FieldOffset: return "FieldOffset";
            case mmu::OutroHandoffFactKind::GlobalConstant: return "GlobalConstant";
            case mmu::OutroHandoffFactKind::LiteralConstant: return "LiteralConstant";
            }
            return "?";
        };
        auto emitCall = [&c, &line, &callKind](const mmu::OutroHandoffCall& row) {
            std::snprintf(line, sizeof(line),
                          "  func=0x%06x call=0x%06x -> 0x%06x string=0x%06x "
                          "hash=0x%08x %-18s %-26s %s",
                          row.functionVa, row.callVa, row.targetVa,
                          row.stringVa, row.hash, callKind(row.kind),
                          std::string(row.owner).c_str(),
                          std::string(row.literal).c_str());
            c.log(line);
        };
        auto emitString = [&c, &line](const mmu::OutroHandoffString& row) {
            std::snprintf(line, sizeof(line), "  string=0x%06x hash=0x%08x %s",
                          row.stringVa, row.hash, std::string(row.literal).c_str());
            c.log(line);
        };
        auto emitFact = [&c, &line, &factKind](const mmu::OutroHandoffFact& fact) {
            char offset[16];
            if (fact.byteOffset == mmu::kOutroHandoffNoOffset)
                std::snprintf(offset, sizeof(offset), "none");
            else
                std::snprintf(offset, sizeof(offset), "0x%04x", fact.byteOffset);
            std::snprintf(line, sizeof(line),
                          "  func=0x%06x line=%-4u %-16s off=%-6s value=0x%08x %s",
                          fact.functionVa, fact.lineNumber, factKind(fact.kind),
                          offset, fact.value, std::string(fact.expression).c_str());
            c.log(line);
        };
        auto usage = [&c]() {
            c.log("usage: outro [summary|calls|call <va>|owner <name>|strings|facts|func <va>|offset <hex>]");
        };
        if (what == "call") {
            if (args.size() < 2) { c.log("usage: outro call <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            const mmu::OutroHandoffCall* row = mmu::findOutroHandoffCall(va);
            if (row) emitCall(*row);
            else c.log("  (unknown call)");
        } else if (what == "owner") {
            if (args.size() < 2) { c.log("usage: outro owner <name>"); return; }
            std::size_t shown = 0;
            for (const mmu::OutroHandoffCall& row : mmu::kOutroHandoffCalls) {
                if (std::string(row.owner) != args[1]) continue;
                emitCall(row);
                ++shown;
            }
            if (!shown) c.log("  (unknown owner)");
        } else if (what == "func") {
            if (args.size() < 2) { c.log("usage: outro func <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::OutroHandoffCall& row : mmu::kOutroHandoffCalls) {
                if (row.functionVa != va) continue;
                emitCall(row);
                ++shown;
            }
            for (const mmu::OutroHandoffFact& fact : mmu::kOutroHandoffFacts) {
                if (fact.functionVa != va) continue;
                emitFact(fact);
                ++shown;
            }
            if (!shown) c.log("  (unknown function)");
        } else if (what == "offset") {
            if (args.size() < 2) { c.log("usage: outro offset <hex>"); return; }
            const u32 offset = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::OutroHandoffFact& fact : mmu::kOutroHandoffFacts) {
                if (fact.byteOffset != offset) continue;
                emitFact(fact);
                ++shown;
            }
            if (!shown) c.log("  (unknown offset)");
        } else if (what == "calls") {
            for (const mmu::OutroHandoffCall& row : mmu::kOutroHandoffCalls) emitCall(row);
        } else if (what == "strings") {
            for (const mmu::OutroHandoffString& row : mmu::kOutroHandoffStrings) emitString(row);
        } else if (what == "facts") {
            for (const mmu::OutroHandoffFact& fact : mmu::kOutroHandoffFacts) emitFact(fact);
        } else if (what != "summary") {
            usage();
        } else {
            std::snprintf(line, sizeof(line),
                          "outro handoff calls=%zu strings=%zu facts=%zu",
                          std::size(mmu::kOutroHandoffCalls),
                          std::size(mmu::kOutroHandoffStrings),
                          std::size(mmu::kOutroHandoffFacts));
            c.log(line);
        }
    });
    state.console.registerCommand("keycandidates", "unverified CRC key-name candidates: keycandidates [summary|list|key <hex>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[256];
        auto emit = [&c, &line](const mmu::UnverifiedKeyNameCandidate& row) {
            std::snprintf(line, sizeof(line), "  UNVERIFIED 0x%08x %s", row.hash,
                          std::string(row.name).c_str());
            c.log(line);
        };
        if (what == "list") {
            for (const mmu::UnverifiedKeyNameCandidate& row : mmu::kUnverifiedKeyNameCandidates) emit(row);
        } else if (what == "key") {
            if (args.size() < 2) { c.log("usage: keycandidates key <hex>"); return; }
            const u32 hash = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            const std::string_view name = mmu::unverifiedKeyNameCandidate(hash);
            if (name.empty()) c.log("  (no candidate)");
            else {
                std::snprintf(line, sizeof(line), "  UNVERIFIED 0x%08x %.*s", hash,
                              static_cast<int>(name.size()), name.data());
                c.log(line);
            }
        } else if (what != "summary") {
            c.log("usage: keycandidates [summary|list|key <hex>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "unverified key name candidates=%zu (not imported as names)",
                          mmu::kUnverifiedKeyNameCandidateCount);
            c.log(line);
        }
    });
    state.console.registerCommand("gunparams", "decoded gun-object parameters: gunparams [summary|objects|object <name>|key <hex>|lookup <object> <key>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        struct GunObjectView {
            std::string_view name;
            const mmu::GunObject* object;
        };
        static constexpr GunObjectView kObjects[] = {
            {"Boss", &mmu::kBoss},
            {"MineE", &mmu::kMineE},
            {"SawPole", &mmu::kSawPole},
            {"SuperCannon", &mmu::kSuperCannon},
            {"Telly", &mmu::kTelly},
            {"WilyCapsule", &mmu::kWilyCapsule},
        };
        const std::string what = args.empty() ? "summary" : args[0];
        char line[256];
        auto emitParam = [&c, &line](const GunObjectView& view, const mmu::GunParamEntry& entry) {
            std::snprintf(line, sizeof(line), "  %-12s key=0x%08x value=%.9g",
                          std::string(view.name).c_str(), entry.key,
                          static_cast<double>(entry.value));
            c.log(line);
        };
        auto findObject = [&](const std::string& name) -> const GunObjectView* {
            for (const GunObjectView& view : kObjects)
                if (view.name == name) return &view;
            return nullptr;
        };
        if (what == "objects") {
            for (const GunObjectView& view : kObjects) {
                std::snprintf(line, sizeof(line), "  %-12s key=0x%08x params=%u",
                              std::string(view.name).c_str(), view.object->key,
                              view.object->paramCount);
                c.log(line);
            }
        } else if (what == "object") {
            if (args.size() < 2) { c.log("usage: gunparams object <name>"); return; }
            const GunObjectView* view = findObject(args[1]);
            if (!view) { c.log("  (unknown object)"); return; }
            for (u32 i = 0; i < view->object->paramCount; ++i)
                emitParam(*view, view->object->params[i]);
        } else if (what == "key") {
            if (args.size() < 2) { c.log("usage: gunparams key <hex>"); return; }
            const u32 key = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const GunObjectView& view : kObjects)
                for (u32 i = 0; i < view.object->paramCount; ++i)
                    if (view.object->params[i].key == key) {
                        emitParam(view, view.object->params[i]);
                        ++shown;
                    }
            if (!shown) c.log("  (unknown key in decoded objects)");
        } else if (what == "lookup") {
            if (args.size() < 3) { c.log("usage: gunparams lookup <object> <key>"); return; }
            const GunObjectView* view = findObject(args[1]);
            if (!view) { c.log("  (unknown object)"); return; }
            const u32 key = static_cast<u32>(std::strtoul(args[2].c_str(), nullptr, 0));
            bool found = false;
            for (u32 i = 0; i < view->object->paramCount; ++i) {
                if (view->object->params[i].key != key) continue;
                emitParam(*view, view->object->params[i]);
                found = true;
            }
            if (!found) c.log("  (key absent from object)");
        } else if (what != "summary") {
            c.log("usage: gunparams [summary|objects|object <name>|key <hex>|lookup <object> <key>]");
        } else {
            std::size_t params = 0;
            for (const GunObjectView& view : kObjects) params += view.object->paramCount;
            std::snprintf(line, sizeof(line), "decoded gun objects=%zu params=%zu",
                          std::size(kObjects), params);
            c.log(line);
        }
    });
    state.console.registerCommand("coverage", "bounded artifact coverage: coverage [summary|keys|states|reports]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[512];
        auto ratio = [&c, &line](const char* label, u32 covered, u32 total,
                                 u32 remaining) {
            std::snprintf(line, sizeof(line), "  %s=%u/%u (%.1f%%) remaining=%u",
                          label, covered, total,
                          total ? 100.0 * covered / total : 0.0, remaining);
            c.log(line);
        };
        if (what == "keys") {
            ratio("verified keys", mmu::kCoverageVerifiedKeys,
                  mmu::kCoverageCollectedKeys, mmu::kCoverageUnresolvedKeys);
            std::snprintf(line, sizeof(line),
                          "  mitm unverified=%u sweep candidates=%u sweep new_matches=%u",
                          mmu::kCoverageMitmUnverifiedKeys,
                          mmu::kCoverageSweepCandidates,
                          mmu::kCoverageSweepNewMatches);
            c.log(line);
        } else if (what == "states") {
            ratio("state slots", mmu::kCoverageSlotComplete,
                  mmu::kCoverageSlotStates, mmu::kCoverageSlotIncomplete);
        } else if (what == "reports") {
            ratio("raw reports with portable headers",
                  mmu::kCoverageHeaderBackedReports, mmu::kCoverageRawReports,
                  mmu::kCoverageRawReports -
                      mmu::kCoverageHeaderBackedReports);
            ratio("generated headers reachable from SDL",
                  mmu::kCoverageGeneratedHeadersInHarness,
                  mmu::kCoverageGeneratedHeaders,
                  mmu::kCoverageGeneratedHeaders -
                      mmu::kCoverageGeneratedHeadersInHarness);
            std::snprintf(line, sizeof(line),
                          "  direct generated header includes=%u",
                          mmu::kCoverageGeneratedHeadersDirect);
            c.log(line);
            ratio("commands documented", mmu::kCoverageDocumentedCommands,
                  mmu::kCoverageRegisteredCommands,
                  mmu::kCoverageRegisteredCommands -
                      mmu::kCoverageDocumentedCommands);
            for (std::size_t i = 0; i < mmu::kCoverageUnmappedReportCount; ++i) {
                const std::string_view name = mmu::kCoverageUnmappedReports[i];
                std::snprintf(line, sizeof(line), "  without portable header=%.*s",
                              static_cast<int>(name.size()), name.data());
                c.log(line);
            }
        } else if (what != "summary") {
            c.log("usage: coverage [summary|keys|states|reports]");
        } else {
            ratio("verified keys", mmu::kCoverageVerifiedKeys,
                  mmu::kCoverageCollectedKeys, mmu::kCoverageUnresolvedKeys);
            ratio("state slots", mmu::kCoverageSlotComplete,
                  mmu::kCoverageSlotStates, mmu::kCoverageSlotIncomplete);
            ratio("raw reports", mmu::kCoverageHeaderBackedReports,
                  mmu::kCoverageRawReports,
                  mmu::kCoverageRawReports - mmu::kCoverageHeaderBackedReports);
            ratio("generated headers reachable from SDL",
                  mmu::kCoverageGeneratedHeadersInHarness,
                  mmu::kCoverageGeneratedHeaders,
                  mmu::kCoverageGeneratedHeaders -
                      mmu::kCoverageGeneratedHeadersInHarness);
            std::snprintf(line, sizeof(line),
                          "  direct generated header includes=%u",
                          mmu::kCoverageGeneratedHeadersDirect);
            c.log(line);
            ratio("commands documented", mmu::kCoverageDocumentedCommands,
                  mmu::kCoverageRegisteredCommands,
                  mmu::kCoverageRegisteredCommands -
                      mmu::kCoverageDocumentedCommands);
            std::snprintf(line, sizeof(line),
                          "  decompiled=%u (denominator unknown) vtables=%u slots=%u "
                          "proto=%u/%u/%u scenes=%u",
                          mmu::kCoverageDecompiledFunctions,
                          mmu::kCoverageRttiVtables,
                          mmu::kCoverageRttiVtableSlots,
                          mmu::kCoverageProtoSchemas,
                          mmu::kCoverageProtoMessages,
                          mmu::kCoverageProtoFields,
                          mmu::kCoverageSceneNames);
            c.log(line);
            std::snprintf(line, sizeof(line),
                          "  ported functions full=%u partial=%u data=%u of %u; MegaMan %u/%u",
                          mmu::kCoverageFunctionsFull, mmu::kCoverageFunctionsPartial,
                          mmu::kCoverageFunctionsDataOnly, mmu::kCoverageFunctionsTotal,
                          mmu::kCoverageMegaManPorted, mmu::kCoverageMegaManFunctions);
            c.log(line);
        }
    });
    state.console.registerCommand("keysweep", "key-name sweep counters only: keysweep [summary]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.size() >= 1 && args[0] != "summary") {
            c.log("usage: keysweep [summary]");
            return;
        }
        char line[256];
        std::snprintf(line, sizeof(line),
                      "key sweep candidates=%u collected=%u previously_known=%u "
                      "new_matches=%u (candidate strings not imported)",
                      mmu::kKeySweepCandidateCount,
                      mmu::kKeySweepCollectedCount,
                      mmu::kKeySweepPreviouslyKnownCount,
                      mmu::kKeySweepNewMatchCount);
        c.log(line);
    });
    state.console.registerCommand("keynames", "verified GameParam key names: keynames [summary|list|key <hex>|name <text>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[256];
        auto emit = [&c, &line](const mmu::VerifiedKeyName& row) {
            std::snprintf(line, sizeof(line), "  0x%08x %s", row.hash,
                          std::string(row.name).c_str());
            c.log(line);
        };
        if (what == "list") {
            for (const mmu::VerifiedKeyName& row : mmu::kVerifiedKeyNames) emit(row);
        } else if (what == "key") {
            if (args.size() < 2) { c.log("usage: keynames key <hex>"); return; }
            const u32 hash = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            const std::string_view name = mmu::verifiedKeyName(hash);
            if (name.empty()) {
                c.log("  (unverified or unknown key)");
            } else {
                std::snprintf(line, sizeof(line), "  0x%08x %.*s", hash,
                              static_cast<int>(name.size()), name.data());
                c.log(line);
            }
        } else if (what == "name") {
            if (args.size() < 2) { c.log("usage: keynames name <text>"); return; }
            std::size_t shown = 0;
            for (const mmu::VerifiedKeyName& row : mmu::kVerifiedKeyNames) {
                if (row.name != args[1]) continue;
                emit(row);
                ++shown;
            }
            if (!shown) c.log("  (unknown name)");
        } else if (what != "summary") {
            c.log("usage: keynames [summary|list|key <hex>|name <text>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "verified key names=%zu candidate reports are not imported",
                          mmu::kVerifiedKeyNameCount);
            c.log(line);
        }
    });
    state.console.registerCommand("wily", "raw Wily attack selector: wily [summary|phases|phase <n>|raw]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        auto emit = [&c, &line](int phase, const mmu::WilySelectorPhase& row) {
            std::snprintf(line, sizeof(line),
                          "  phase=%d target=0x%06x state=%-12s clear_counter=%s",
                          phase, row.targetVa, std::string(row.stateName).c_str(),
                          row.clearsShotCounter ? "true" : "false");
            c.log(line);
        };
        if (what == "phases") {
            for (int phase = 0; phase < mmu::kWilyPhaseCount; ++phase)
                emit(phase, mmu::kWilySelectorPhases[phase]);
        } else if (what == "phase") {
            if (args.size() < 2) { c.log("usage: wily phase <n>"); return; }
            const int phase = std::atoi(args[1].c_str());
            const int normalized = mmu::wilySelectorPhase(phase);
            emit(phase, mmu::kWilySelectorPhases[normalized]);
            std::snprintf(line, sizeof(line), "  normalized=%d wrap=%d", normalized,
                          mmu::kWilyWrapPhase);
            c.log(line);
        } else if (what == "raw") {
            std::snprintf(line, sizeof(line),
                          "  selector=0x%06x jump_table=0x%06x caller=0x%06x "
                          "phase=+0x%03x counter=+0x%03x timer=+0x%03x "
                          "event=0x%x limit=%d",
                          mmu::kWilySelectorVa, mmu::kWilySelectorJumpTableVa,
                          mmu::kWilySelectorCallerVa, mmu::kWilyPhaseField,
                          mmu::kWilyShotCounterField, mmu::kWilyHoverTimerField,
                          mmu::kWilyEventCode0f, mmu::kWilyShotCounterLimit);
            c.log(line);
        } else if (what != "summary") {
            c.log("usage: wily [summary|phases|phase <n>|raw]");
        } else {
            std::snprintf(line, sizeof(line),
                          "wily selector=0x%06x jump_table=0x%06x phases=%d wrap=%d caller=0x%06x",
                          mmu::kWilySelectorVa, mmu::kWilySelectorJumpTableVa,
                          mmu::kWilyPhaseCount, mmu::kWilyWrapPhase,
                          mmu::kWilySelectorCallerVa);
            c.log(line);
        }
    });
    state.console.registerCommand("bossdamage", "raw boss-damage caller map: bossdamage [summary|calls|owner <name>|call <va>|bits <hex>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[512];
        auto emit = [&c, &line](const mmu::BossDamageCaller& row) {
            std::snprintf(line, sizeof(line),
                          "  owner=%-18s call=0x%06x handler=0x%06x "
                          "vftable=0x%06x slot=%-2u bits=0x%08x %s",
                          row.owner.empty() ? "(unresolved)"
                                            : std::string(row.owner).c_str(),
                          row.callVa, row.handlerVa, row.vftableVa,
                          row.vtableSlot, row.resultCodeBits,
                          std::string(row.resolution).c_str());
            c.log(line);
        };
        if (what == "call") {
            if (args.size() < 2) { c.log("usage: bossdamage call <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::BossDamageCaller& row : mmu::kBossDamageCallers) {
                if (row.callVa != va) continue;
                emit(row);
                ++shown;
            }
            if (!shown) c.log("  (unknown call)");
        } else if (what == "owner") {
            if (args.size() < 2) { c.log("usage: bossdamage owner <name>"); return; }
            std::size_t shown = 0;
            for (const mmu::BossDamageCaller& row : mmu::kBossDamageCallers) {
                if (std::string(row.owner) != args[1]) continue;
                emit(row);
                ++shown;
            }
            if (!shown) c.log("  (unknown owner)");
        } else if (what == "bits") {
            if (args.size() < 2) { c.log("usage: bossdamage bits <hex>"); return; }
            const u32 bits = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::BossDamageCaller& row : mmu::kBossDamageCallers) {
                if (row.resultCodeBits != bits) continue;
                emit(row);
                ++shown;
            }
            if (!shown) c.log("  (unknown bits)");
        } else if (what == "calls") {
            for (const mmu::BossDamageCaller& row : mmu::kBossDamageCallers) emit(row);
        } else if (what != "summary") {
            c.log("usage: bossdamage [summary|calls|owner <name>|call <va>|bits <hex>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "boss damage key=0x%08x getter=0x%06x compare=0x%06x "
                          "default=%.1f code=%d meters=0x%04x/0x%04x "
                          "helpers=0x%06x/0x%06x callers=%zu",
                          mmu::kBossDamageParamKey, mmu::kBossDamageGetterVa,
                          mmu::kBossDamageCompareVa,
                          static_cast<double>(mmu::kBossDamageParamDefault),
                          mmu::kBossDamageDepletionCode,
                          mmu::kBossDamageMeterOffset0,
                          mmu::kBossDamageMeterOffset1,
                          mmu::kBossDamageHelperVa0, mmu::kBossDamageHelperVa1,
                          std::size(mmu::kBossDamageCallers));
            c.log(line);
        }
    });
    state.console.registerCommand("camera", "raw Camera/CameraTarget evidence: camera [summary|slots|target|states|edges|operands|func <va>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[640];
        auto usage = [&c]() {
            c.log("usage: camera [summary|slots|target|states|edges|operands|func <va>]");
        };
        if (what == "slots") {
            for (const mmu::CameraVtableSlot& row : mmu::kCameraVtableSlots) {
                std::snprintf(line, sizeof(line), "  Camera slot=%-2u func=0x%06x",
                              static_cast<unsigned>(row.slot), row.functionVa);
                c.log(line);
            }
        } else if (what == "target") {
            for (const mmu::CameraVtableSlot& row : mmu::kCameraTargetVtableSlots) {
                std::snprintf(line, sizeof(line), "  CameraTarget slot=%-2u func=0x%06x",
                              static_cast<unsigned>(row.slot), row.functionVa);
                c.log(line);
            }
        } else if (what == "states") {
            for (const mmu::CameraTargetState& row : mmu::kCameraTargetStates) {
                std::snprintf(line, sizeof(line),
                              "  %-5s reg=0x%06x hash=0x%08x slots=%u,%u,%u "
                              "thunks=%s complete=%s",
                              std::string(row.name).c_str(), row.registrationVa,
                              row.hash, row.slot0, row.slot1, row.slot2,
                              std::string(row.thunks).c_str(),
                              row.complete ? "yes" : "no");
                c.log(line);
            }
        } else if (what == "edges") {
            for (const mmu::CameraTargetEdge& row : mmu::kCameraTargetEdges) {
                std::snprintf(line, sizeof(line),
                              "  slot=%-2u func=0x%06x call=0x%06x "
                              "hash=0x%08x state=%s",
                              static_cast<unsigned>(row.slot), row.functionVa,
                              row.callVa, row.hash,
                              std::string(row.stateName).c_str());
                c.log(line);
            }
        } else if (what == "operands") {
            for (const mmu::CameraOperandLine& row : mmu::kCameraOperandLines) {
                std::snprintf(line, sizeof(line), "  0x%06x:%u %s",
                              row.functionVa, row.sourceLine,
                              std::string(row.text).c_str());
                c.log(line);
            }
        } else if (what == "func") {
            if (args.size() < 2) { c.log("usage: camera func <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (std::size_t i = 0; i < mmu::kCameraOperandLineCount; ++i) {
                const mmu::CameraOperandLine* row = mmu::cameraOperandLine(va, i);
                if (!row) break;
                std::snprintf(line, sizeof(line), "  0x%06x:%u %s",
                              row->functionVa, row->sourceLine,
                              std::string(row->text).c_str());
                c.log(line);
                ++shown;
            }
            if (!shown) c.log("  (unknown function)");
        } else if (what != "summary") {
            usage();
        } else {
            std::snprintf(line, sizeof(line),
                          "Camera vftable=0x%06x slots=%zu | CameraTarget "
                          "vftable=0x%06x ctor=0x%06x slots=%zu states=%zu "
                          "edges=%zu operands=%zu",
                          mmu::kCameraVtableVa, mmu::kCameraVtableSlotCount,
                          mmu::kCameraTargetVtableVa,
                          mmu::kCameraTargetConstructorVa,
                          mmu::kCameraTargetVtableSlotCount,
                          mmu::kCameraTargetStateCount,
                          mmu::kCameraTargetEdgeCount,
                          mmu::kCameraOperandLineCount);
            c.log(line);
            c.log("No scroll speed, direction, timing, trigger or room destination is inferred.");
        }
    });
    state.console.registerCommand("playerspawn", "raw player-spawn token evidence: playerspawn [summary|checks|candidates|scene <n>|exact <token>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[640];
        auto usage = [&c]() {
            c.log("usage: playerspawn [summary|checks|candidates|scene <n>|exact <token>]");
        };
        auto emitCandidate = [&c, &line](const mmu::PlayerSpawnCandidate& row) {
            std::snprintf(line, sizeof(line),
                          "  scene=%-3u path=%s key=0x%08x spawn=%u type=%s "
                          "x=%.1f y=%.1f flags=%u attr1=%s",
                          row.sceneIndex, std::string(row.sceneName).c_str(),
                          row.sceneKey, row.spawnIndex,
                          std::string(row.type).c_str(), row.x, row.y,
                          row.flags, std::string(row.attr1).c_str());
            c.log(line);
        };
        if (what == "checks") {
            for (const mmu::PlayerSpawnTypeCount& row : mmu::kPlayerSpawnExactChecks) {
                std::snprintf(line, sizeof(line), "  %-14s %u",
                              std::string(row.type).c_str(), row.count);
                c.log(line);
            }
        } else if (what == "candidates") {
            for (const mmu::PlayerSpawnCandidate& row : mmu::kPlayerSpawnCandidates)
                emitCandidate(row);
        } else if (what == "scene") {
            if (args.size() < 2) { c.log("usage: playerspawn scene <n>"); return; }
            const u32 scene = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (const mmu::PlayerSpawnCandidate& row : mmu::kPlayerSpawnCandidates) {
                if (row.sceneIndex != scene) continue;
                emitCandidate(row);
                ++shown;
            }
            if (!shown) c.log("  (no substring candidate)");
        } else if (what == "exact") {
            if (args.size() < 2) { c.log("usage: playerspawn exact <token>"); return; }
            const mmu::PlayerSpawnTypeCount* row =
                mmu::playerSpawnExactCheck(args[1]);
            if (!row) c.log("  (unknown token)");
            else {
                std::snprintf(line, sizeof(line), "  %-14s %u",
                              std::string(row->type).c_str(), row->count);
                c.log(line);
            }
        } else if (what != "summary") {
            usage();
        } else {
            std::snprintf(line, sizeof(line),
                          "scenes=%u spawns=%u unique_types=%u "
                          "substring(megaman)=%u substring(player)=%u "
                          "candidates=%zu",
                          mmu::kPlayerSpawnSceneCount,
                          mmu::kPlayerSpawnRecordCount,
                          mmu::kPlayerSpawnUniqueTypeCount,
                          mmu::kPlayerSpawnSubstringMegaManCount,
                          mmu::kPlayerSpawnSubstringPlayerCount,
                          mmu::kPlayerSpawnCandidateCount);
            c.log(line);
            c.log("No scene is designated as the real player spawn.");
        }
    });
    state.console.registerCommand("playerctl", "raw player control evidence: playerctl [summary|states|keys|edges|chars|resources|operands|state <name>|key <name>|func <va>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[640];
        auto usage = [&c]() {
            c.log("usage: playerctl [summary|states|keys|edges|chars|resources|operands|state <name>|key <name>|func <va>]");
        };
        if (what == "states") {
            for (const mmu::PlayerControlState& row : mmu::kPlayerControlStates) {
                std::snprintf(line, sizeof(line),
                              "  %-12s hash=0x%08x first=0x%06x refs=%u",
                              std::string(row.name).c_str(), row.hash,
                              row.firstVa, row.refs);
                c.log(line);
            }
        } else if (what == "state") {
            if (args.size() < 2) { c.log("usage: playerctl state <name>"); return; }
            const mmu::PlayerControlState* row = mmu::playerControlState(args[1]);
            if (!row) c.log("  (unknown state)");
            else {
                std::snprintf(line, sizeof(line),
                              "  %-12s hash=0x%08x first=0x%06x refs=%u",
                              std::string(row->name).c_str(), row->hash,
                              row->firstVa, row->refs);
                c.log(line);
            }
        } else if (what == "keys") {
            for (const mmu::PlayerControlKey& row : mmu::kPlayerControlKeys) {
                std::snprintf(line, sizeof(line), "  %-22s hash=0x%08x",
                              std::string(row.name).c_str(), row.hash);
                c.log(line);
            }
        } else if (what == "key") {
            if (args.size() < 2) { c.log("usage: playerctl key <name>"); return; }
            const mmu::PlayerControlKey* row = mmu::playerControlKey(args[1]);
            if (!row) c.log("  (unknown key)");
            else {
                std::snprintf(line, sizeof(line), "  %-22s hash=0x%08x",
                              std::string(row->name).c_str(), row->hash);
                c.log(line);
            }
        } else if (what == "edges") {
            for (const mmu::PlayerControlEdge& row : mmu::kPlayerControlEdges) {
                std::snprintf(line, sizeof(line),
                              "  %-19s %-18s slot=%-2u func=0x%06x call=0x%06x "
                              "%-7s hash=0x%08x",
                              std::string(row.className).c_str(),
                              std::string(row.target).c_str(), row.slot,
                              row.functionVa, row.callVa,
                              row.isCue ? "cue" : "setState", row.hash);
                c.log(line);
            }
        } else if (what == "chars") {
            for (const mmu::PlayerControlCharacter& row : mmu::kPlayerControlCharacters) {
                std::snprintf(line, sizeof(line),
                              "  index=%u action=0x%02x character=%u label='%s' "
                              "labelVa=0x%06x object=0x%06x",
                              row.index, row.action, row.character,
                              std::string(row.label).c_str(), row.labelVa,
                              row.object);
                c.log(line);
            }
        } else if (what == "resources") {
            for (const mmu::PlayerControlResource& row : mmu::kPlayerControlResources) {
                std::snprintf(line, sizeof(line), "  key=0x%08x path=%s",
                              row.key, std::string(row.path).c_str());
                c.log(line);
            }
        } else if (what == "operands") {
            for (const mmu::PlayerControlOperandLine& row : mmu::kPlayerControlOperandLines) {
                std::snprintf(line, sizeof(line), "  0x%06x:%u %s",
                              row.functionVa, row.sourceLine,
                              std::string(row.text).c_str());
                c.log(line);
            }
        } else if (what == "func") {
            if (args.size() < 2) { c.log("usage: playerctl func <va>"); return; }
            const u32 va = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            std::size_t shown = 0;
            for (std::size_t i = 0; i < mmu::kPlayerControlOperandLineCount; ++i) {
                const mmu::PlayerControlOperandLine* row =
                    mmu::playerControlOperandLine(va, i);
                if (!row) break;
                std::snprintf(line, sizeof(line), "  0x%06x:%u %s",
                              row->functionVa, row->sourceLine,
                              std::string(row->text).c_str());
                c.log(line);
                ++shown;
            }
            if (!shown) c.log("  (unknown function)");
        } else if (what != "summary") {
            usage();
        } else {
            std::snprintf(line, sizeof(line),
                          "states=%zu keys=%zu edges=%zu chars=%zu "
                          "resources=%zu operands=%zu charge_field=%s",
                          mmu::kPlayerControlStateCount,
                          mmu::kPlayerControlKeyCount,
                          mmu::kPlayerControlEdgeCount,
                          mmu::kPlayerControlCharacterCount,
                          mmu::kPlayerControlResourceCount,
                          mmu::kPlayerControlOperandLineCount,
                          std::string(mmu::kPlayerControlChargeShotField).c_str());
            c.log(line);
            c.log("No portable physics constant or effect timing is derived here.");
        }
    });
    state.console.registerCommand("flowcomplete", "raw completion-cutscene branches: flowcomplete [summary|rows|flag <id>|state <hex> [0|1]|pending]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[640];
        auto emit = [&c, &line](const mmu::RecoveredCompletionCutscene& row) {
            char writes[96];
            writes[0] = '\0';
            for (mmu::u8 i = 0; i < row.writeCount && i < 2; ++i) {
                std::snprintf(writes + std::strlen(writes), sizeof(writes) - std::strlen(writes),
                              " +0x%02x=0x%04x", row.writeOffsets[i], row.writeValues[i]);
            }
            std::snprintf(line, sizeof(line),
                          "  flag=%-2u state=0x%03x sel=%u string=0x%06x pending=0x%06x "
                          "writes=%u%s %s",
                          row.rawFlagId, row.managerState, row.selector, row.stringVa,
                          row.pendingStringVa, row.writeCount, writes,
                          std::string(row.path).c_str());
            c.log(line);
        };
        auto usage = [&c]() {
            c.log("usage: flowcomplete [summary|rows|flag <id>|state <hex> [0|1]|pending]");
        };
        if (what == "rows") {
            for (const mmu::RecoveredCompletionCutscene& row : mmu::kRecoveredCompletionCutscenes) emit(row);
        } else if (what == "flag") {
            if (args.size() < 2) { c.log("usage: flowcomplete flag <id>"); return; }
            const u32 flag = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            const mmu::RecoveredCompletionCutscene* row = mmu::findCompletionCutsceneFlag(flag);
            if (row) emit(*row);
            else c.log("  (unknown flag)");
        } else if (what == "state") {
            if (args.size() < 2) { c.log("usage: flowcomplete state <hex> [0|1]"); return; }
            const u32 state = static_cast<u32>(std::strtoul(args[1].c_str(), nullptr, 0));
            const bool flag59 = args.size() >= 3 && std::strtoul(args[2].c_str(), nullptr, 0) != 0;
            const mmu::RecoveredCompletionCutscene* row = mmu::findCompletionCutsceneState(state, flag59);
            if (row) emit(*row);
            else c.log("  (unknown state)");
        } else if (what == "pending") {
            std::snprintf(line, sizeof(line),
                          "  pending call=0x%06x string=+0x%02x flag=+0x%02x=0x%02x "
                          "clear=0x%06x global=0x%06x writes=%zu",
                          mmu::kCompletionPendingCallVa,
                          mmu::kCompletionPendingStringOffset,
                          mmu::kCompletionPendingFlagOffset,
                          mmu::kCompletionPendingFlagValue,
                          mmu::kCompletionClearCallVa, mmu::kCompletionClearGlobalVa,
                          mmu::kCompletionClearWriteCount);
            c.log(line);
            for (const mmu::CompletionClearWrite& write : mmu::kCompletionClearWrites) {
                std::snprintf(line, sizeof(line), "  clear +0x%03x size=%u value=0x%04x",
                              write.offset, write.size, write.value);
                c.log(line);
            }
        } else if (what != "summary") {
            usage();
        } else {
            std::snprintf(line, sizeof(line),
                          "completion cutscene rows=%zu pending_call=0x%06x clear_call=0x%06x",
                          mmu::kRecoveredCompletionCutsceneCount,
                          mmu::kCompletionPendingCallVa, mmu::kCompletionClearCallVa);
            c.log(line);
        }
    });
    state.console.registerCommand("flowgrid", "raw stage-select grid evidence: flowgrid [summary|cells|keys|center <0|1> <count>|stage <x> <y> <0|1> <count>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[320];
        auto usage = [&c]() {
            c.log("usage: flowgrid [summary|cells|keys|center <0|1> <count>|stage <x> <y> <0|1> <count>]");
        };
        if (what == "cells") {
            for (const mmu::StageSelectGridOuterCell& cell : mmu::kStageSelectOuterCells) {
                std::snprintf(line, sizeof(line), "  (%d,%d) -> stage %d",
                              cell.x, cell.y, cell.stage);
                c.log(line);
            }
        } else if (what == "keys") {
            for (const mmu::StageSelectCursorKey& key : mmu::kStageSelectCursorKeys) {
                std::snprintf(line, sizeof(line),
                              "  key=0x%02x field=+0x%04x bound=%d delta=%+d",
                              key.key, key.fieldOffset, key.bound, key.delta);
                c.log(line);
            }
        } else if (what == "center") {
            if (args.size() < 3) { c.log("usage: flowgrid center <0|1> <count>"); return; }
            const bool flag = std::strtoul(args[1].c_str(), nullptr, 0) != 0;
            const int count = std::atoi(args[2].c_str());
            std::snprintf(line, sizeof(line), "  center flag=0xba8=%u count=%d -> stage %d",
                          flag ? 1u : 0u, count,
                          mmu::stageSelectGridCenterStage(flag, count));
            c.log(line);
        } else if (what == "stage") {
            if (args.size() < 5) {
                c.log("usage: flowgrid stage <x> <y> <0|1> <count>");
                return;
            }
            const int x = std::atoi(args[1].c_str());
            const int y = std::atoi(args[2].c_str());
            const bool flag = std::strtoul(args[3].c_str(), nullptr, 0) != 0;
            const int count = std::atoi(args[4].c_str());
            std::snprintf(line, sizeof(line),
                          "  (%d,%d) flag=0xba8=%u count=%d -> stage %d",
                          x, y, flag ? 1u : 0u, count,
                          mmu::stageSelectGridStage(x, y, flag, count));
            c.log(line);
        } else if (what != "summary") {
            usage();
        } else {
            std::snprintf(line, sizeof(line),
                          "stage grid outer=%zu flag_stage=%d count=%d..%d keys=%zu",
                          std::size(mmu::kStageSelectOuterCells),
                          mmu::kStageSelectCenterFlagStage,
                          mmu::kStageSelectCenterCountMin,
                          mmu::kStageSelectCenterCountMax,
                          std::size(mmu::kStageSelectCursorKeys));
            c.log(line);
        }
    });
    state.console.registerCommand("flowmenu", "recovered menu item/action tables: flowmenu [items [class]|actions [code|kind]]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        if (what == "items") {
            const std::string className = args.size() > 1 ? args[1] : "MainMenu";
            std::size_t shown = 0;
            for (const mmu::RecoveredMenuItem& item : mmu::kRecoveredMenuItems) {
                if (item.className != className) continue;
                std::snprintf(line, sizeof(line),
                              "  %-26s slot=%-2u supplier=0x%06x action=0x%02x %-7s '%.*s'",
                              std::string(item.className).c_str(), item.index,
                              item.supplierVa, item.actionCode,
                              menuItemBehaviorName(item.behavior),
                              static_cast<int>(item.label.size()),
                              item.label.empty() ? "" : item.label.data());
                c.log(line);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "menu items for %s=%zu",
                          className.c_str(), shown);
            c.log(line);
            return;
        }
        if (what == "actions") {
            std::string filter = args.size() > 1 ? args[1] : std::string();
            char* end = nullptr;
            const unsigned long parsed =
                filter.empty() ? 0 : std::strtoul(filter.c_str(), &end, 0);
            const bool exactCode = !filter.empty() && *end == '\0';
            std::size_t shown = 0;
            for (const mmu::RecoveredMenuAction& action :
                 mmu::kRecoveredMenuActions) {
                if (exactCode && action.actionCode != parsed) continue;
                if (!exactCode && !filter.empty() &&
                    filter != mmu::menuActionKindName(action.kind)) {
                    continue;
                }
                std::string screens;
                for (mmu::u8 i = 0; i < action.screenCount && i < 2; ++i) {
                    if (!screens.empty()) screens += ", ";
                    screens += mmu::flowScreenName(action.screens[i]);
                }
                std::string variants;
                for (mmu::u8 i = 0; i < action.variantCount && i < 2; ++i) {
                    char supplier[16];
                    std::snprintf(supplier, sizeof(supplier), "0x%06x",
                                  action.variantSuppliers[i]);
                    if (!variants.empty()) variants += ",";
                    variants += supplier;
                }
                std::snprintf(line, sizeof(line),
                              "  action=0x%02x %-12s screens=%-34s items=%-20s %s",
                              action.actionCode,
                              mmu::menuActionKindName(action.kind),
                              screens.empty() ? "-" : screens.c_str(),
                              variants.empty() ? "-" : variants.c_str(),
                              action.note);
                c.log(line);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "menu actions shown=%zu",
                          shown);
            c.log(line);
            return;
        }
        std::unordered_map<std::string, bool> classes;
        std::unordered_map<std::string, bool> suppliers;
        std::size_t screenActions = 0;
        for (const mmu::RecoveredMenuItem& item : mmu::kRecoveredMenuItems) {
            classes[std::string(item.className)] = true;
            suppliers[std::to_string(item.supplierVa)] = true;
        }
        for (const mmu::RecoveredMenuAction& action :
             mmu::kRecoveredMenuActions) {
            if (action.kind == mmu::MenuActionKind::Screen) ++screenActions;
        }
        std::snprintf(line, sizeof(line),
                      "menu item tables=%zu classes=%zu suppliers=%zu "
                      "actions=%zu withScreen=%zu",
                      mmu::kRecoveredMenuItemsCount, classes.size(),
                      suppliers.size(), mmu::kRecoveredMenuActionsCount,
                      screenActions);
        c.log(line);
        c.log("Use 'flowmenu items MainMenu' or 'flowmenu actions 0x47'.");
    });
    state.console.registerCommand("cutscene", "cutscene [list|keys [list|key <hex>|path <text>]|start <key|#index|suffix>|status|stop|step <sec>|events]", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        mmu::CutsceneRuntime& cutscene = state.cutsceneRuntime;
        cutscene.attach(state.assets);
        const std::string what = args.empty() ? "status" : args[0];
        char line[256];
        if (what == "keys") {
            const std::string sub = args.size() >= 2 ? args[1] : std::string("summary");
            auto emit = [&c, &line](const mmu::RegisteredCutscene& row) {
                std::snprintf(line, sizeof(line), "  key=%-10u hex=0x%08x path=%.*s",
                              row.key, row.key,
                              static_cast<int>(row.path.size()), row.path.data());
                c.log(line);
            };
            if (sub == "list") {
                for (const mmu::RegisteredCutscene& row : mmu::kRegisteredCutscenes)
                    emit(row);
            } else if (sub == "key") {
                if (args.size() < 3) { c.log("usage: cutscene keys key <hex>"); return; }
                const u32 key = static_cast<u32>(std::strtoul(args[2].c_str(), nullptr, 0));
                const std::string_view path = mmu::cutscenePathForKey(key);
                if (path.empty()) c.log("  (unknown key)");
                else {
                    std::snprintf(line, sizeof(line), "  key=%-10u hex=0x%08x path=%.*s",
                                  key, key, static_cast<int>(path.size()), path.data());
                    c.log(line);
                }
            } else if (sub == "path") {
                if (args.size() < 3) { c.log("usage: cutscene keys path <text>"); return; }
                std::size_t shown = 0;
                for (const mmu::RegisteredCutscene& row : mmu::kRegisteredCutscenes) {
                    if (std::string(row.path).find(args[2]) == std::string::npos) continue;
                    emit(row);
                    ++shown;
                }
                std::snprintf(line, sizeof(line), "registered cutscene path matches=%zu", shown);
                c.log(line);
            } else if (sub != "summary") {
                c.log("usage: cutscene keys [list|key <hex>|path <text>]");
            } else {
                std::snprintf(line, sizeof(line),
                              "registered cutscene keys=%zu first=0x%08x last=0x%08x",
                              mmu::kRegisteredCutsceneCount,
                              mmu::kRegisteredCutscenes[0].key,
                              mmu::kRegisteredCutscenes[
                                  mmu::kRegisteredCutsceneCount - 1].key);
                c.log(line);
            }
            return;
        }
        if (what == "list") {
            for (std::size_t i = 0; i < state.assets.cutscenes().size(); ++i) {
                const std::string path(state.assets.cutscenePaths().size() > i
                                           ? state.assets.cutscenePaths()[i]
                                           : mmu::cutscenePathForIndex(state.assets, i));
                const u32 key = state.assets.cutsceneKeys().size() > i
                                    ? state.assets.cutsceneKeys()[i] : 0;
                std::snprintf(line, sizeof(line), "#%-3zu key=%-10u %s", i, key,
                              path.c_str());
                c.log(line);
            }
            std::snprintf(line, sizeof(line), "cutscenes=%zu",
                          state.assets.cutscenes().size());
            c.log(line);
            return;
        }
        if (what == "start") {
            if (args.size() < 2) { c.log("usage: cutscene start <key|#index|suffix>"); return; }
            const std::string arg = args[1];
            const auto isNumber = [](const std::string& text) {
                char* end = nullptr;
                std::strtoul(text.c_str(), &end, 0);
                return end != text.c_str() && *end == '\0';
            };
            bool started = false;
            if (!arg.empty() && arg[0] == '#') {
                started = cutscene.startIndex(
                    static_cast<std::size_t>(std::strtoul(arg.c_str() + 1, nullptr, 0)));
            } else if (isNumber(arg)) {
                started = cutscene.startKey(
                    static_cast<u32>(std::strtoul(arg.c_str(), nullptr, 0)));
            } else {
                started = cutscene.startPathSuffix(arg);
            }
            if (started) state.cutsceneEventLog.clear();
            c.log(started ? "cutscene started" : "cutscene not found");
            return;
        }
        if (what == "stop") {
            cutscene.stop();
            state.cutsceneEventLog.clear();
            c.log("cutscene stopped");
            return;
        }
        if (what == "step") {
            const float seconds = args.size() < 2 ? 0.0f
                                                  : static_cast<float>(std::atof(args[1].c_str()));
            cutscene.update(seconds);
            dispatchCutsceneEvents(state);
            return;
        }
        if (what == "events") {
            const std::vector<mmu::CutsceneEvent> events = state.cutsceneEventLog;
            for (const mmu::CutsceneEvent& event : events) {
                std::snprintf(line, sizeof(line),
                              "t=%-5u %-16s id=%-16s resource=%-24s anim=%-20s value=%s",
                              event.timestampMs, mmu::cutsceneCommandName(event.kind),
                              event.id.c_str(), event.resource.c_str(),
                              event.anim.c_str(), event.value.c_str());
                c.log(line);
            }
            std::snprintf(line, sizeof(line), "events=%zu", events.size());
            c.log(line);
            return;
        }
        if (what != "status") {
            c.log("usage: cutscene [list|keys [list|key <hex>|path <text>]|start <key|#index|suffix>|status|stop|step <sec>|events]");
            return;
        }
        std::snprintf(line, sizeof(line),
                      "playing=%s clock=%.0fms timestamp=%zu elements=%zu texts=%zu events=%zu scene=%.*s",
                      cutscene.playing() ? "yes" : "no", cutscene.clockMs(),
                      cutscene.currentTimestamp(), cutscene.elements().size(),
                      cutscene.texts().size(), state.cutsceneEventLog.size(),
                      static_cast<int>(cutscene.sceneName().size()),
                      cutscene.sceneName().data());
        c.log(line);
    });
    state.console.registerCommand("stats", "show decoded asset counts", [&state](mmu::DebugConsole&, const mmu::DebugConsole::Arguments&) {
        logStats(state);
    });
    state.console.registerCommand("load", "load an .mmu file from the asset directory", [&state](mmu::DebugConsole&, const mmu::DebugConsole::Arguments& args) {
        if (args.empty()) { state.console.log("usage: load <file.mmu>"); return; }
        loadAssetFile(state, args[0]);
    });
    state.console.registerCommand("sprites", "list the first N sprites", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        int count = args.empty() ? 10 : std::atoi(args[0].c_str());
        if (count < 0) count = 0;
        if (state.pak.valid()) {
            const auto& sprites = state.pak.sprites();
            for (std::size_t i = 0; i < sprites.size() && static_cast<int>(i) < count; ++i) {
                c.log(std::to_string(sprites[i].key) + "  frames="
                      + std::to_string(sprites[i].frameCount) + "  "
                      + spriteLabel(state.pak, sprites[i]));
            }
            return;
        }
        const auto& sprites = state.assets.sprites();
        for (std::size_t i = 0; i < sprites.size() && static_cast<int>(i) < count; ++i) {
            const mmu::SpriteAsset& sprite = sprites[i];
            const std::string path = sprite.animation.frames.empty()
                ? sprite.resRoot : sprite.animation.frames.front().imagePath;
            c.log(std::to_string(sprite.key) + "  " + (path.empty() ? sprite.source : path));
        }
    });
    state.console.registerCommand("sprite", "select a sprite in the viewer by key", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.empty() || !state.pak.valid()) { c.log("usage: sprite <key>"); return; }
        const u32 key = static_cast<u32>(std::strtoul(args[0].c_str(), nullptr, 0));
        const int index = findSpriteIndex(state.pak, key);
        if (index < 0) { c.log("No sprite with key " + std::to_string(key)); return; }
        state.viewer.selected = index;
        state.viewer.frame = 0;
        state.viewer.elapsed = 0.0f;
        state.viewer.open = true;
        const mmu::PackedSprite& sprite = state.pak.sprites()[index];
        char line[160];
        std::snprintf(line, sizeof(line), "Selected sprite %u: %u frames, handle (%d, %d)",
                      sprite.key, sprite.frameCount, sprite.xHandle, sprite.yHandle);
        c.log(line);
    });
    state.console.registerCommand("pak", "load assets.pak and list its first entries", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string file = args.empty() ? "assets.pak" : args[0];
        if (!state.pak.valid() && !loadPackedArchive(state, file)) {
            c.log("Could not load " + file);
            return;
        }
        if (state.typeImages.empty()) state.typeImages = mmu::resolveTypeImages(state.pak);
        int count = 20;
        if (args.size() > 1) count = std::atoi(args[1].c_str());
        const auto& entries = state.pak.entries();
        for (std::size_t i = 0; i < entries.size() && static_cast<int>(i) < count; ++i) {
            const mmu::PackedEntry& entry = entries[i];
            char line[256];
            std::snprintf(line, sizeof(line), "%zu %-5s key=%u %ux%u %s",
                          i, mmu::packedEntryTypeName(entry.type), entry.key,
                          entry.width, entry.height,
                          entry.name.empty() ? mmu::packedSourceName(entry.source)
                                             : std::string(entry.name).c_str());
            c.log(line);
        }
    });
    state.console.registerCommand("pakimage", "preview a packed image by index", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.empty() || !state.pak.valid()) { c.log("usage: pakimage <index>"); return; }
        const std::size_t index = static_cast<std::size_t>(std::strtoul(args[0].c_str(), nullptr, 0));
        if (index >= state.pak.entries().size()) { c.log("Index out of range"); return; }
        uploadPreview(state, g_renderer, describe(state.pak.entries()[index]));
        c.log("Previewing packed image " + std::to_string(index));
    });
    state.console.registerCommand("audio", "show audio device status", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        const mmu::sdl2::AudioStatus s = state.audio.status();
        char line[192];
        std::snprintf(line, sizeof(line),
                      "device=%s %d Hz %d ch voices=%d/%zu master=%d music=%s",
                      s.opened ? "open" : "closed", s.frequency, s.channels,
                      s.activeVoices, s.voiceCount, s.masterVolume,
                      s.music.empty() ? "(none)" : s.music.c_str());
        c.log(line);
    });
    state.console.registerCommand("sfx", "play a sound effect by key", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.empty() || !state.pak.valid()) { c.log("usage: sfx <key>"); return; }
        const u32 key = static_cast<u32>(std::strtoul(args[0].c_str(), nullptr, 0));
        c.log(state.audio.playSfx(state.pak, key)
              ? "Playing sfx " + std::to_string(key)
              : "No sfx with key " + std::to_string(key));
    });
    state.console.registerCommand("music", "play looping music by key", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.empty() || !state.pak.valid()) { c.log("usage: music <key>"); return; }
        const u32 key = static_cast<u32>(std::strtoul(args[0].c_str(), nullptr, 0));
        c.log(state.audio.playMusic(state.pak, key)
              ? "Playing music " + std::to_string(key)
              : "No music with key " + std::to_string(key));
    });
    state.console.registerCommand("stopsound", "stop all audio", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        state.audio.stopAll();
        c.log("Audio stopped");
    });
    state.console.registerCommand("scenenames", "raw editor scene paths: scenenames [summary|list [filter]|path <text>|index <n>]", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const std::string what = args.empty() ? "summary" : args[0];
        char line[384];
        auto emit = [&c, &line](std::size_t index, std::string_view name) {
            std::snprintf(line, sizeof(line), "  %zu %.*s", index,
                          static_cast<int>(name.size()), name.data());
            c.log(line);
        };
        if (what == "list") {
            const std::string filter = args.size() >= 2 ? args[1] : std::string();
            std::size_t shown = 0;
            for (std::size_t i = 0; i < mmu::kSceneNameCount; ++i) {
                if (!filter.empty() &&
                    std::string(mmu::kSceneNames[i]).find(filter) == std::string::npos)
                    continue;
                emit(i, mmu::kSceneNames[i]);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "scene paths shown=%zu", shown);
            c.log(line);
        } else if (what == "path") {
            if (args.size() < 2) { c.log("usage: scenenames path <text>"); return; }
            std::size_t shown = 0;
            for (std::size_t i = 0; i < mmu::kSceneNameCount; ++i) {
                if (std::string(mmu::kSceneNames[i]).find(args[1]) == std::string::npos)
                    continue;
                emit(i, mmu::kSceneNames[i]);
                ++shown;
            }
            std::snprintf(line, sizeof(line), "scene path matches=%zu", shown);
            c.log(line);
        } else if (what == "index") {
            if (args.size() < 2) { c.log("usage: scenenames index <n>"); return; }
            const std::size_t index = static_cast<std::size_t>(
                std::strtoul(args[1].c_str(), nullptr, 0));
            const std::string_view name = mmu::sceneName(index);
            if (name.empty()) c.log("  (index out of range)");
            else emit(index, name);
        } else if (what != "summary") {
            c.log("usage: scenenames [summary|list [filter]|path <text>|index <n>]");
        } else {
            std::snprintf(line, sizeof(line),
                          "scene paths=%zu first=%.*s last=%.*s",
                          mmu::kSceneNameCount,
                          static_cast<int>(mmu::kSceneNames[0].size()),
                          mmu::kSceneNames[0].data(),
                          static_cast<int>(
                              mmu::kSceneNames[mmu::kSceneNameCount - 1].size()),
                          mmu::kSceneNames[mmu::kSceneNameCount - 1].data());
            c.log(line);
        }
    });
    state.console.registerCommand("levels", "list scenes baked into assets.pak", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (!state.pak.valid() || state.pak.scenes().empty()) {
            c.log("No scene data in assets.pak (re-bake with MMUPAK v3).");
            return;
        }
        int count = args.empty() ? 10 : std::atoi(args[0].c_str());
        if (count < 0) count = 0;
        for (std::size_t i = 0; i < state.pak.scenes().size() && static_cast<int>(i) < count; ++i) {
            const mmu::PackedScene& scene = state.pak.scenes()[i];
            char line[192];
            if (!scene.name.empty()) {
                std::snprintf(line, sizeof(line), "#%zu  %.*s  spawns=%u",
                              i, static_cast<int>(scene.name.size()), scene.name.data(),
                              scene.spawnCount);
            } else {
                std::snprintf(line, sizeof(line), "#%zu  key=%u  spawns=%u",
                              i, scene.key, scene.spawnCount);
            }
            c.log(line);
        }
    });
    state.console.registerCommand("level", "select a scene by key or #index", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.empty() || !state.pak.valid()) { c.log("usage: level <key|#index>"); return; }
        const int index = findSceneIndex(state.pak, args[0]);
        if (index < 0 || static_cast<std::size_t>(index) >= state.pak.scenes().size()) {
            c.log("No scene matching " + args[0]);
            return;
        }
        state.levelViewer.selected = index;
        state.levelViewer.hoveredSpawn = -1;
        state.levelViewer.open = true;
        c.log("Selected scene #" + std::to_string(index) + " key="
              + std::to_string(state.pak.scenes()[index].key));
    });
    state.console.registerCommand("play", "instantiate a scene in the live world", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.empty() || !state.pak.valid()) { c.log("usage: play <key|#index>"); return; }
        const int index = findSceneIndex(state.pak, args[0]);
        if (index < 0 || static_cast<std::size_t>(index) >= state.pak.scenes().size()) {
            c.log("No scene matching " + args[0]);
            return;
        }
        if (state.typeImages.empty()) state.typeImages = mmu::resolveTypeImages(state.pak);
        state.world.loadScene(state.pak, state.pak.scenes()[index], state.typeImages);
        state.gameView.sceneIndex = index;
        state.gameView.running = true;
        state.gameView.open = true;
        c.log("Playing scene #" + std::to_string(index) + " key="
              + std::to_string(state.pak.scenes()[index].key) + " objects="
              + std::to_string(state.world.objects().size()));
    });
    state.console.registerCommand("types", "report spawn-type to sprite resolution", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        if (!state.pak.valid()) { c.log("No packed archive loaded."); return; }
        if (state.typeImages.empty()) state.typeImages = mmu::resolveTypeImages(state.pak);
        char line[192];
        std::snprintf(line, sizeof(line), "resolved %zu spawn types to packed images",
                      state.typeImages.size());
        c.log(line);
        int shown = 0;
        for (const auto& resolved : state.typeImages) {
            if (shown++ >= 8) break;
            const mmu::PackedEntry& image = state.pak.entries()[resolved.second];
            std::snprintf(line, sizeof(line), "  %-24s -> %u %.*s", resolved.first.c_str(),
                          resolved.second, static_cast<int>(image.name.size()), image.name.data());
            c.log(line);
        }
    });
    state.console.registerCommand("state", "show or change the player state", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        mmu::PlayerController& pc = state.world.controller();
        if (args.empty()) {
            char line[192];
            std::snprintf(line, sizeof(line),
                          "player %s (prev %s, t=%.2f) hp=%.0f weapon=%d energy=%.0f pos=(%.0f,%.0f) %s",
                          mmu::playerStateName(pc.state), mmu::playerStateName(pc.previous),
                          pc.stateTime, pc.health, pc.weapon + 1, pc.weaponEnergy,
                          pc.x, pc.y, pc.facingRight ? "right" : "left");
            c.log(line);
            return;
        }
        mmu::PlayerState parsed;
        if (mmu::parsePlayerState(args[0], parsed)) {
            pc.setState(parsed);
            c.log(std::string("player state -> ") + mmu::playerStateName(parsed));
        } else {
            c.log("Unknown state: " + args[0] + " (try: states)");
        }
    });
    state.console.registerCommand("states", "list recovered MegaMan states", [](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        std::string line;
        for (int i = 0; i <= static_cast<int>(mmu::PlayerState::Land); ++i) {
            line += mmu::playerStateName(static_cast<mmu::PlayerState>(i));
            line += (i == static_cast<int>(mmu::PlayerState::Land)) ? "\n" : " ";
        }
        c.log(line);
    });
    state.console.registerCommand("event", "inject a player event id (0..38)", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        if (args.empty()) { c.log("usage: event <id> [value]"); return; }
        const int id = std::atoi(args[0].c_str());
        if (id < 0 || id > 0x26) { c.log("event id out of range"); return; }
        const float value = args.size() > 1 ? static_cast<float>(std::atof(args[1].c_str())) : 0.0f;
        state.world.controller().handleEvent(static_cast<mmu::PlayerEvent>(static_cast<mmu::u8>(id)), value);
        char line[128];
        std::snprintf(line, sizeof(line), "event 0x%02x -> state %s", id,
                      mmu::playerStateName(state.world.controller().state));
        c.log(line);
    });
    state.console.registerCommand("hurt", "damage the player", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        mmu::PlayerController& pc = state.world.controller();
        const float sourceX = args.empty() ? pc.x + 24.0f : static_cast<float>(std::atof(args[0].c_str()));
        pc.handleEvent(mmu::PlayerEvent::Damage, sourceX);
        char line[128];
        std::snprintf(line, sizeof(line), "damage -> %s hp=%.0f", mmu::playerStateName(pc.state), pc.health);
        c.log(line);
    });
    state.console.registerCommand("heal", "restore health", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const float amount = args.empty() ? 100.0f : static_cast<float>(std::atof(args[0].c_str()));
        state.world.controller().handleEvent(mmu::PlayerEvent::Heal, amount);
        c.log("health=" + std::to_string(state.world.controller().health));
    });
    state.console.registerCommand("collision", "report the scene collision map at the player", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        const mmu::CollisionMap& col = state.world.collision();
        const mmu::PlayerController& pc = state.world.controller();
        if (!col.valid()) { c.log("no packed collision map loaded"); return; }
        char line[192];
        std::snprintf(line, sizeof(line),
                      "collision image=%u size=%ux%u player=(%.0f,%.0f) solid=%s grounded=%s",
                      col.imageIndex, col.width, col.height, pc.x, pc.y,
                      col.solidAt(pc.x, pc.y) ? "yes" : "no",
                      pc.grounded ? "yes" : "no");
        c.log(line);
    });
    state.console.registerCommand("enemies", "list live enemy state machines", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        const auto& enemies = state.world.enemies();
        if (enemies.empty()) { c.log("no enemy controllers in this scene"); return; }
        int limit = static_cast<int>(enemies.size());
        if (!args.empty()) limit = std::max(0, std::min(limit, std::atoi(args[0].c_str())));
        char line[192];
        for (int i = 0; i < limit; ++i) {
            const mmu::EnemyController& enemy = enemies[i];
            const std::string type = enemy.objectIndex < state.world.objects().size()
                ? state.world.objects()[enemy.objectIndex].type : std::string("?");
            std::snprintf(line, sizeof(line),
                          "  #%zu %-12s %-7s t=%.2f pos=(%.0f,%.0f) home=(%.0f,%.0f) spd=%.0f rng=%.0f obj=%u",
                          static_cast<std::size_t>(i), type.c_str(),
                          mmu::enemyStateName(enemy.state), enemy.stateTime,
                          enemy.x, enemy.y, enemy.homeX, enemy.homeY, enemy.speed,
                          enemy.range, enemy.objectIndex);
            c.log(line);
        }
        char summary[64];
        std::snprintf(summary, sizeof(summary), "enemy controllers=%zu", enemies.size());
        c.log(summary);
    });
    state.console.registerCommand("projectiles", "list live runtime-spawned projectiles", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments&) {
        char line[192];
        std::size_t count = 0;
        for (const mmu::WorldObject& object : state.world.objects()) {
            if (!object.active || object.projectile == mmu::SpawnedObjectKind::None) continue;
            ++count;
            std::snprintf(line, sizeof(line),
                          "  #%u %-20s age=%.2f delay=%.2f pos=(%.0f,%.0f) vel=(%.0f,%.0f) dmg=%.0f",
                          object.id, object.type.c_str(), object.projectileAge,
                          object.delay, object.x, object.y, object.vx, object.vy,
                          object.damage);
            c.log(line);
        }
        std::snprintf(line, sizeof(line), "projectiles=%zu", count);
        c.log(line);
    });
    state.console.registerCommand("enemy", "show or force an enemy state", [&state](mmu::DebugConsole& c, const mmu::DebugConsole::Arguments& args) {
        auto& enemies = state.world.enemies();
        if (args.empty() || enemies.empty()) { c.log("usage: enemy <index> [state|hurt N]; try 'enemy states' for the full list"); return; }
        if (args[0] == "states") {
            c.log("Idle MoveH MoveV Wait Death Scan Explode Stand Turn Attack Intro Pose "
                  "Hurt Patrol Hover PillarDrop Lightning Ram Attack3 GotoAttack2 "
                  "Fly FlyDown BigExplosion MidBossExplosion Move Shoot Init RamReturn "
                  "Spawn FlyUp GotoAttack1 GotoAttack3 GotoAttack4 Crush EnergyFill "
                  "WilyBossExplosion Up Down Flash Appear Hidden Pop Fire SwimUp FallDown "
                  "Hiding Exposed MoveBack Charge ShootUp ShootDown TurnUp TurnDown");
            return;
        }
        const int index = std::atoi(args[0].c_str());
        if (index < 0 || static_cast<std::size_t>(index) >= enemies.size()) { c.log("enemy index out of range"); return; }
        mmu::EnemyController& enemy = enemies[static_cast<std::size_t>(index)];
        if (args.size() > 1) {
            if (args[1] == "hurt") {
                enemy.hurt(args.size() > 2 ? static_cast<float>(std::atof(args[2].c_str())) : 10.0f);
            } else {
                mmu::EnemyState parsed;
                if (!mmu::parseEnemyState(args[1], parsed)) { c.log("unknown enemy state (try: help enemy)"); return; }
                enemy.setState(parsed);
            }
        }
        char line[192];
        std::snprintf(line, sizeof(line),
                      "enemy #%d kind=%s state=%s previous=%s pos=(%.0f,%.0f) sign=%.0f hp=%.0f shots=%u",
                      index, mmu::enemyKindName(enemy.kind), mmu::enemyStateName(enemy.state),
                      mmu::enemyStateName(enemy.previous), enemy.x, enemy.y, enemy.moveSign,
                      enemy.hp, enemy.shotsFired);
        c.log(line);
    });
    state.console.registerCommand("quit", "exit the application", [&state](mmu::DebugConsole&, const mmu::DebugConsole::Arguments&) {
        state.quit = true;
    });
}

int findSpriteIndex(const mmu::PackedArchive& pak, u32 key) {
    for (std::size_t i = 0; i < pak.sprites().size(); ++i)
        if (pak.sprites()[i].key == key) return static_cast<int>(i);
    return -1;
}

void drawSpriteViewer(AppState& state, TextureCache& cache, float delta) {
    SpriteViewerState& viewer = state.viewer;
    if (!viewer.open) return;
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowSpriteViewer,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Sprite Viewer", &viewer.open);
    if (!state.pak.valid()) {
        ImGui::TextWrapped("No packed archive loaded. Run mmu_bake and place assets.pak in the asset directory, or type 'pak' in the console.");
        ImGui::End();
        return;
    }

    ImGui::Columns(2, "sprite-columns", true);
    ImGui::InputText("Filter", viewer.filter, IM_ARRAYSIZE(viewer.filter));
    ImGui::BeginChild("sprite list", ImVec2(0, -8), ImGuiChildFlags_Borders);
    const std::string filter = viewer.filter;
    for (std::size_t i = 0; i < state.pak.sprites().size(); ++i) {
        const mmu::PackedSprite& sprite = state.pak.sprites()[i];
        std::string label = std::to_string(sprite.key) + "  " + spriteLabel(state.pak, sprite);
        if (!filter.empty() && label.find(filter) == std::string::npos) continue;
        const bool selected = viewer.selected == static_cast<int>(i);
        if (ImGui::Selectable(label.c_str(), selected)) {
            viewer.selected = static_cast<int>(i);
            viewer.frame = 0;
            viewer.elapsed = 0.0f;
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();
    if (viewer.selected < 0 || static_cast<std::size_t>(viewer.selected) >= state.pak.sprites().size()) {
        ImGui::TextWrapped("Select a sprite on the left.");
        ImGui::End();
        return;
    }

    const mmu::PackedSprite& sprite = state.pak.sprites()[viewer.selected];
    ImGui::Text("key=%u  frames=%u  source=%s", sprite.key, sprite.frameCount,
                mmu::packedSourceName(sprite.source));
    ImGui::Text("handle=(%d, %d)  %s", sprite.xHandle, sprite.yHandle,
                spriteLabel(state.pak, sprite).c_str());
    if (!sprite.frameCount || !sprite.frames) {
        ImGui::Text("This sprite has no frames.");
        ImGui::End();
        return;
    }
    if (viewer.frame < 0) viewer.frame = 0;
    if (static_cast<u32>(viewer.frame) >= sprite.frameCount) viewer.frame = sprite.frameCount - 1;

    if (viewer.playing && sprite.frameCount > 1) {
        viewer.elapsed += delta;
        // Re-read each frame's duration so mixed-length animations stay in sync.
        for (u32 guard = 0; guard < sprite.frameCount; ++guard) {
            const float duration = sprite.frames[viewer.frame].duration > 0.0f
                ? sprite.frames[viewer.frame].duration : 1.0f / 12.0f;
            if (viewer.elapsed < duration) break;
            viewer.elapsed -= duration;
            viewer.frame = (viewer.frame + 1) % sprite.frameCount;
        }
    }
    const mmu::PackedFrame& frame = sprite.frames[viewer.frame];

    ImGui::Checkbox("Play", &viewer.playing);
    ImGui::SameLine();
    ImGui::Checkbox("Collision", &viewer.showCollision);
    ImGui::SliderInt("Frame", &viewer.frame, 0, static_cast<int>(sprite.frameCount) - 1);
    ImGui::SliderFloat("Scale", &viewer.scale, 0.25f, 8.0f);
    ImGui::Text("offset=(%d, %d)  duration=%.3fs  image=%u",
                frame.xOffset, frame.yOffset, frame.duration, frame.imageIndex);

    SDL_Texture* texture = cache.get(state.pak, frame.imageIndex);
    if (texture) {
        const mmu::PackedEntry& image = state.pak.entries()[frame.imageIndex];
        const ImVec2 size(static_cast<float>(image.width) * viewer.scale,
                          static_cast<float>(image.height) * viewer.scale);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::Image(reinterpret_cast<ImTextureID>(texture), size);
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        if (viewer.showCollision && frame.collisionIndex < state.pak.entries().size()) {
            if (SDL_Texture* collision = cache.get(state.pak, frame.collisionIndex)) {
                const mmu::PackedEntry& mask = state.pak.entries()[frame.collisionIndex];
                const ImVec2 end(origin.x + static_cast<float>(mask.width) * viewer.scale,
                                 origin.y + static_cast<float>(mask.height) * viewer.scale);
                drawList->AddImage(reinterpret_cast<ImTextureID>(collision), origin, end,
                                   ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 60, 60, 96));
            }
        }

        const ImVec2 handle(origin.x + static_cast<float>(sprite.xHandle - frame.xOffset) * viewer.scale,
                            origin.y + static_cast<float>(sprite.yHandle - frame.yOffset) * viewer.scale);
        const float arm = 10.0f;
        drawList->AddLine(ImVec2(handle.x - arm, handle.y), ImVec2(handle.x + arm, handle.y),
                          IM_COL32(80, 255, 120, 255), 1.0f);
        drawList->AddLine(ImVec2(handle.x, handle.y - arm), ImVec2(handle.x, handle.y + arm),
                          IM_COL32(80, 255, 120, 255), 1.0f);
    } else {
        ImGui::Text("Image entry %u is unavailable.", frame.imageIndex);
    }
    ImGui::End();
}

const mmu::SceneArt& sceneArtFor(AppState& state, u32 sceneIndex) {
    auto it = state.sceneArt.find(sceneIndex);
    if (it != state.sceneArt.end()) return it->second;
    const mmu::SceneArt& art = state.sceneArt[sceneIndex] =
        mmu::resolveSceneArt(state.pak, state.pak.scenes()[sceneIndex]);
    return art;
}

// Background frames are tiled horizontally so the long sections stay covered.
template <typename ToScreen>
void drawSceneBackground(TextureCache& cache, AppState& state, u32 sceneIndex,
                         ImDrawList* list, ToScreen toScreen, float worldLeft,
                         float worldRight, float scale, float time, bool animate) {
    const mmu::SceneArt& art = sceneArtFor(state, sceneIndex);
    if (art.empty()) return;
    const std::size_t frame = animate
        ? static_cast<std::size_t>(time / art.frameDuration) % art.frames.size() : 0;
    SDL_Texture* texture = cache.get(state.pak, art.frames[frame]);
    if (!texture) return;
    const float w = static_cast<float>(art.width) * scale;
    const float h = static_cast<float>(art.height) * scale;
    if (w < 1.0f) return;
    const int first = static_cast<int>(std::floor(worldLeft / art.width)) - 1;
    const int last = static_cast<int>(std::ceil(worldRight / art.width)) + 1;
    for (int tile = first; tile <= last; ++tile) {
        const ImVec2 p = toScreen(static_cast<float>(tile) * art.width, 0.0f);
        list->AddImage(reinterpret_cast<ImTextureID>(texture), p,
                       ImVec2(p.x + w, p.y + h), ImVec2(0, 0), ImVec2(1, 1),
                       IM_COL32(220, 225, 240, 255));
    }
}

// ---------------------------------------------------------------------------
// Debug harness (NOT the original game)
//
// This is a deterministic test bench for the reconstructed world/controller
// code: a crude title card, fixed 60 Hz stepping, global input, a 256x224
// viewport and a primitive HUD. It is deliberately labelled as a harness;
// reconstructing the original MMU.exe game loop remains the project goal.
// ---------------------------------------------------------------------------

enum class GameMode : std::uint8_t { Title, Playing, Paused, GameOver };

struct GameRuntime {
    GameMode mode = GameMode::Title;
    int sceneIndex = 0;
    float accumulator = 0.0f;
    float gameOverTime = 0.0f;
    SDL_GameController* pad = nullptr;
    bool prevJump = false;
    bool prevPause = false;
    bool prevConfirm = false;
    bool prevUp = false;
    bool prevDown = false;
    bool prevLeft = false;
    bool prevRight = false;

    ~GameRuntime() {
        if (pad) SDL_GameControllerClose(pad);
    }

    void discoverPad() {
        if (pad || SDL_NumJoysticks() <= 0 || !SDL_IsGameController(0)) return;
        pad = SDL_GameControllerOpen(0);
    }

    bool startScene(AppState& state, int index) {
        if (state.pak.scenes().empty()) return false;
        if (index < 0 || static_cast<std::size_t>(index) >= state.pak.scenes().size())
            index = 0;
        sceneIndex = index;
        state.gameView.sceneIndex = index;
        state.gameView.running = true; // console compatibility: runtime owns stepping
        state.world.loadScene(state.pak, state.pak.scenes()[index], state.typeImages);
        accumulator = 0.0f;
        gameOverTime = 0.0f;
        mode = GameMode::Playing;
        char line[128];
        std::snprintf(line, sizeof(line), "harness: scene #%d objects=%zu", index,
                      state.world.objects().size());
        state.console.log(line);
        return true;
    }

    // Consume the flow events that need a real scene load. GameFlowRuntime
    // stays event-only; this harness chooses the baked scene from exact
    // recovered names and reports any stage without a scene candidate.
    bool consumeFlowEvent(AppState& state, const mmu::FlowEvent& event) {
        char line[192];
        if (event.type == mmu::FlowEventType::StageSelected) {
            const mmu::u32 stageId = static_cast<mmu::u32>(event.value);
            if (stageId != 0 && state.flowRuntime.applyLevelSetup(stageId)) {
                std::snprintf(line, sizeof(line),
                              "harness: stage %u selected by stage grid", stageId);
            } else {
                std::snprintf(line, sizeof(line),
                              "harness: stage %u has no recovered level setup",
                              stageId);
            }
            state.console.log(line);
            return false;
        }
        if (event.type == mmu::FlowEventType::MenuAction &&
            event.actionCode == 0x0au && event.continueStageId != 0) {
            bool applied = state.flowRuntime.applyLevelSetup(event.continueStageId);
            if (!applied && event.continueConstructorVa != 0)
                applied = state.flowRuntime.applyLevelSetupConstructor(
                    event.continueConstructorVa);
            std::snprintf(line, sizeof(line),
                          applied
                              ? "harness: continue stage %u applied"
                              : "harness: continue stage %u has no recovered level setup",
                          event.continueStageId);
            state.console.log(line);
            return false;
        }
        if (event.type == mmu::FlowEventType::LevelSetupApplied &&
            event.levelSetup != nullptr) {
            const mmu::u32 stageId = event.levelSetup->stageId;
            const int index = stageSceneIndex(state.pak, stageId);
            if (index < 0) {
                std::snprintf(line, sizeof(line),
                              "harness: stage %u has no matching packed scene",
                              stageId);
                state.console.log(line);
                return false;
            }
            startScene(state, index);
            std::snprintf(line, sizeof(line),
                          "harness: stage %u -> scene #%d '%.*s'", stageId, index,
                          static_cast<int>(state.pak.scenes()[index].name.size()),
                          state.pak.scenes()[index].name.empty()
                              ? ""
                              : state.pak.scenes()[index].name.data());
            state.console.log(line);
            state.flowRuntime.clearEvents();
            return true;
        }
        return false;
    }

    void nextScene(AppState& state) {
        const int count = static_cast<int>(state.pak.scenes().size());
        if (count <= 1) {
            startScene(state, sceneIndex);
            return;
        }
        int next = sceneIndex + 1;
        if (next >= count) next = 0;
        startScene(state, next);
    }

    void tick(AppState& state, float delta) {
        discoverPad();
        // The quit warning owns Dpad/A/B/left stick. Freeze harness stepping and
        // sample the same held buttons into edge history so closing the warning
        // never turns a still-held confirm/cancel into a harness menu action.
        if (state.uiState.quitConfirmOpen) {
            const Uint8* blockedKeys = SDL_GetKeyboardState(nullptr);
            const bool blockedJump = blockedKeys &&
                (blockedKeys[SDL_SCANCODE_SPACE] || blockedKeys[SDL_SCANCODE_Z]);
            const bool blockedPause = blockedKeys &&
                (blockedKeys[SDL_SCANCODE_ESCAPE] || blockedKeys[SDL_SCANCODE_P]);
            bool blockedConfirm = blockedKeys &&
                (blockedKeys[SDL_SCANCODE_RETURN] || blockedKeys[SDL_SCANCODE_KP_ENTER]);
            bool blockedUp = blockedKeys &&
                (blockedKeys[SDL_SCANCODE_UP] || blockedKeys[SDL_SCANCODE_W]);
            bool blockedDown = blockedKeys &&
                (blockedKeys[SDL_SCANCODE_DOWN] || blockedKeys[SDL_SCANCODE_S]);
            bool blockedLeft = blockedKeys &&
                (blockedKeys[SDL_SCANCODE_LEFT] || blockedKeys[SDL_SCANCODE_A]);
            bool blockedRight = blockedKeys &&
                (blockedKeys[SDL_SCANCODE_RIGHT] || blockedKeys[SDL_SCANCODE_D]);
            bool blockedStart = false;
            if (pad) {
                blockedStart = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START) != 0;
                blockedConfirm = blockedConfirm ||
                    SDL_GameControllerGetButton(pad, kUiConfirmButton) != 0;
                blockedUp = blockedUp ||
                    SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
                blockedDown = blockedDown ||
                    SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
                blockedLeft = blockedLeft ||
                    SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
                blockedRight = blockedRight ||
                    SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
            }
            prevJump = blockedJump;
            prevPause = blockedPause || blockedStart;
            prevConfirm = blockedConfirm;
            prevUp = blockedUp;
            prevDown = blockedDown;
            prevLeft = blockedLeft;
            prevRight = blockedRight;
            return;
        }
        // Console commands and the old debug flag still load scenes. Adopt
        // them so `play <scene>` starts the harness even from the title card.
        if (mode == GameMode::Title && state.gameView.running && state.world.hasScene()) {
            mode = GameMode::Playing;
            sceneIndex = state.gameView.sceneIndex;
            accumulator = 0.0f;
        }
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        const bool jumpNow = keys && (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_Z]);
        const bool pauseNow = keys && (keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_P]);
        bool confirmNow = keys && (keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_KP_ENTER]);
        bool upNow = keys && (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]);
        bool downNow = keys && (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]);
        bool leftNow = keys && (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]);
        bool rightNow = keys && (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]);
        bool padStart = false, padConfirm = false;
        if (pad) {
            padStart = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START) != 0;
            padConfirm = SDL_GameControllerGetButton(pad, kUiConfirmButton) != 0;
            upNow |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
            downNow |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
            leftNow |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
            rightNow |= SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
        }
        confirmNow = confirmNow || padConfirm || (jumpNow && !prevJump);
        const bool pauseEdge = (pauseNow || padStart) && !prevPause;
        const bool confirmEdge = confirmNow && !prevConfirm;
        mmu::FlowInput flowInput;
        flowInput.confirm = confirmEdge;
        flowInput.cancel = pauseEdge;
        flowInput.up = upNow && !prevUp;
        flowInput.down = downNow && !prevDown;
        flowInput.left = leftNow && !prevLeft;
        flowInput.right = rightNow && !prevRight;
        prevJump = jumpNow;
        prevPause = pauseNow || padStart;
        prevConfirm = confirmNow;
        prevUp = upNow;
        prevDown = downNow;
        prevLeft = leftNow;
        prevRight = rightNow;

        // Title and pause now run the evidence-backed flow runtime and render
        // rows from the recovered menu supplier instead of a hardcoded card.
        // `play <scene>` still bypasses them by adopting the loaded world.
        if (mode == GameMode::Title) {
            state.flowRuntime.step(delta, flowInput);
            while (state.flowRuntime.pendingEventCount() != 0) {
                const mmu::FlowEvent event = state.flowRuntime.pollEvent();
                if (event.type == mmu::FlowEventType::ScreenChangeRequested) {
                    // Exit states emit a request instead of choosing a screen;
                    // the harness maps the observed back-out chain explicitly.
                    if (event.screen == mmu::FlowScreen::TitleMenu ||
                        event.screen == mmu::FlowScreen::MenuStageSelect ||
                        event.screen == mmu::FlowScreen::OptionsMenu) {
                        state.flowRuntime.setScreen(mmu::FlowScreen::MainMenu);
                        // The initial action 0x02 table has no stage-select row.
                        // Use the recovered unflagged action 0x03 MainMenu
                        // supplier, whose slot 0 is STAGE SELECT (0x06).
                        state.flowRuntime.setMenuSupplier(0x508820u);
                    } else {
                        state.flowRuntime.setScreen(mmu::FlowScreen::TitleMenu);
                    }
                } else if (!event.cutscenePath.empty()) {
                    // Action dispatcher cutscenes (0x06/0x1e/0x1f) carry a
                    // literal path and no destination screen.
                    startFlowCutscene(state, event.cutscenePath);
                } else if (consumeFlowEvent(state, event)) {
                    return;
                } else if (event.type == mmu::FlowEventType::MenuAction &&
                           event.hasDestination) {
                    state.flowRuntime.setScreen(event.destination);
                }
            }
            return;
        }
        if (mode == GameMode::GameOver) {
            gameOverTime += delta;
            if (confirmEdge || gameOverTime > 2.0f) startScene(state, sceneIndex);
            return;
        }
        if (pauseEdge) {
            // The harness keeps the four-row pause overlay (including EXIT
            // STAGE); the raw stage predicate from 0x511350 is not modelled by
            // the debug scene, so set the supplier explicitly.
            mmu::FlowMenuFlags pauseFlags = state.flowRuntime.menuFlags();
            pauseFlags.pauseExitStageVariant = true;
            state.flowRuntime.setMenuFlags(pauseFlags);
            if (mode == GameMode::Playing) {
                state.flowRuntime.setScreen(mmu::FlowScreen::MenuInGame);
                mode = GameMode::Paused;
            } else {
                state.flowRuntime.setScreen(mmu::FlowScreen::MenuInGame);
                mode = GameMode::Playing;
            }
            state.flowRuntime.clearEvents();
            return;
        }
        if (mode == GameMode::Paused) {
            mmu::FlowInput menuInput = flowInput;
            menuInput.cancel = false; // ESC/START edge above handles resume
            state.flowRuntime.step(delta, menuInput);
            bool resume = false;
            while (state.flowRuntime.pendingEventCount() != 0) {
                const mmu::FlowEvent event = state.flowRuntime.pollEvent();
                if (event.type == mmu::FlowEventType::ResumeRequested ||
                    (event.type == mmu::FlowEventType::MenuAction &&
                     event.actionCode == 0x0eu)) {
                    resume = true;
                } else if (event.type == mmu::FlowEventType::MenuAction &&
                           (event.actionCode == 0x01u ||
                            event.actionCode == 0x0fu)) {
                    state.flowRuntime.setScreen(mmu::FlowScreen::TitleMenu);
                    mode = GameMode::Title;
                } else if (event.type == mmu::FlowEventType::MenuAction &&
                           event.actionCode == 0x10u) {
                    state.quit = true;
                } else if (!event.cutscenePath.empty()) {
                    startFlowCutscene(state, event.cutscenePath);
                } else if (event.type == mmu::FlowEventType::MenuAction &&
                           event.hasDestination) {
                    state.flowRuntime.setScreen(event.destination);
                }
            }
            if (resume) {
                state.flowRuntime.setScreen(mmu::FlowScreen::MenuInGame);
                state.flowRuntime.clearEvents();
                mode = GameMode::Playing;
            }
            return;
        }

        mmu::PlayerInput input;
        if (keys) {
            if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) input.moveX -= 1.0f;
            if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) input.moveX += 1.0f;
            if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) input.moveY -= 1.0f;
            if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) input.moveY += 1.0f;
            if (jumpNow) input.jump = true;
            // Original defaults (options.xopts): Z jump, LShift fire, LAlt
            // slide. X/C remain as harness aliases for slide/fire.
            if (keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_LALT]) input.dash = true;
            if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_C])
                input.shoot = true;
        }
        if (pad) {
            const Sint16 axisX = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
            const Sint16 axisY = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
            if (std::abs(axisX) > 8000) input.moveX += axisX / 32767.0f;
            if (std::abs(axisY) > 8000) input.moveY += axisY / 32767.0f;
            // Jump/slide follow the platform's confirm/cancel layout.
            if (SDL_GameControllerGetButton(pad, kUiConfirmButton)) input.jump = true;
            if (SDL_GameControllerGetButton(pad, kUiCancelButton)) input.dash = true;
            if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_X)) input.shoot = true;
            if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) input.moveX -= 1.0f;
            if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) input.moveX += 1.0f;
            if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP)) input.moveY -= 1.0f;
            if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) input.moveY += 1.0f;
        }
        if (input.moveX != 0.0f && input.moveY != 0.0f) {
            input.moveX *= 0.70710678f;
            input.moveY *= 0.70710678f;
        }
        input.moveX = std::clamp(input.moveX, -1.0f, 1.0f);
        input.moveY = std::clamp(input.moveY, -1.0f, 1.0f);

        accumulator += delta < 0.1f ? delta : 0.1f;
        constexpr float kStep = 1.0f / 60.0f;
        for (int guard = 0; accumulator >= kStep && guard < 8; ++guard) {
            // Inputs are held state; the controller derives press/release
            // edges itself, like the original input layer (0x441820).
            state.world.step(kStep, input);
            accumulator -= kStep;
        }

        const mmu::PlayerController& pc = state.world.controller();
        if (pc.dead || pc.health <= 0.0f) {
            mode = GameMode::GameOver;
            gameOverTime = 0.0f;
        } else if (pc.x >= state.world.worldWidth() - 48.0f) {
            nextScene(state);
        }
    }
};

SDL_Rect gameViewport(SDL_Renderer* renderer) {
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    int scale = std::min(w / 256, h / 224);
    if (scale < 1) scale = 1;
    const int vw = 256 * scale, vh = 224 * scale;
    return SDL_Rect{(w - vw) / 2, (h - vh) / 2, vw, vh};
}

// Debug Tools: right-click-drag the player. SDL reports mouse positions in
// window points; the game viewport is laid out in renderer output pixels, so
// scale between them before undoing the viewport/camera transform.
void handleDebugDrag(AppState& state, SDL_Window* window, SDL_Renderer* renderer,
                     const SDL_Event& event,
                     bool imguiWantsMouse) {
    UiState& ui = state.uiState;
    if (!ui.debugDragPlayer || !state.world.hasScene()) {
        ui.draggingPlayer = false;
        return;
    }
    int mouseX = 0, mouseY = 0;
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
        if (imguiWantsMouse) return;
        ui.draggingPlayer = true;
        mouseX = event.button.x;
        mouseY = event.button.y;
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
        ui.draggingPlayer = false;
        return;
    } else if (event.type == SDL_MOUSEMOTION && ui.draggingPlayer) {
        mouseX = event.motion.x;
        mouseY = event.motion.y;
    } else {
        return;
    }
    int windowW = 1, windowH = 1;
    SDL_GetWindowSize(window, &windowW, &windowH);
    const float pixelX = mouseX * static_cast<float>(state.outputWidth) / std::max(1, windowW);
    const float pixelY = mouseY * static_cast<float>(state.outputHeight) / std::max(1, windowH);
    const SDL_Rect vp = gameViewport(renderer);
    const float scale = static_cast<float>(vp.w) / 256.0f;
    const mmu::Camera& camera = state.world.camera();
    ui.dragWorldX = camera.x + (pixelX - vp.x) / scale;
    ui.dragWorldY = camera.y + (pixelY - vp.y) / scale;
    state.world.debugPlacePlayer(ui.dragWorldX, ui.dragWorldY);
}

void renderGame(SDL_Renderer* renderer, AppState& state, TextureCache& cache) {
    const SDL_Rect vp = gameViewport(renderer);
    SDL_SetRenderDrawColor(renderer, 8, 10, 16, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 18, 20, 28, 255);
    SDL_RenderFillRect(renderer, &vp);
    if (!state.world.hasScene() || state.pak.scenes().empty()) return;

    const float scale = static_cast<float>(vp.w) / 256.0f;
    const mmu::Camera& camera = state.world.camera();
    const auto sx = [&](float wx) {
        return vp.x + static_cast<int>(std::lround((wx - camera.x) * scale));
    };
    const auto sy = [&](float wy) {
        return vp.y + static_cast<int>(std::lround((wy - camera.y) * scale));
    };

    const u32 sceneIndex = static_cast<u32>(std::clamp(
        state.gameView.sceneIndex, 0, static_cast<int>(state.pak.scenes().size()) - 1));
    const mmu::SceneArt& art = sceneArtFor(state, sceneIndex);
    if (!art.empty()) {
        const std::size_t frame = static_cast<std::size_t>(state.world.time() / art.frameDuration)
                                  % art.frames.size();
        SDL_Texture* bg = cache.get(state.pak, art.frames[frame]);
        if (bg) {
            const int tw = std::max(1, static_cast<int>(std::lround(art.width * scale)));
            const int th = std::max(1, static_cast<int>(std::lround(art.height * scale)));
            const int first = static_cast<int>(std::floor(camera.x / art.width)) - 1;
            const int last = static_cast<int>(std::ceil((camera.x + 256.0f) / art.width)) + 1;
            for (int tile = first; tile <= last; ++tile) {
                const int x = sx(static_cast<float>(tile) * art.width);
                if (x + tw < vp.x || x > vp.x + vp.w) continue;
                const SDL_Rect dst{x, sy(0.0f), tw, th};
                SDL_RenderCopy(renderer, bg, nullptr, &dst);
            }
        }
    }

    const mmu::WorldObject* playerObject = state.world.player();
    for (const mmu::WorldObject& object : state.world.objects()) {
        if (!object.active || &object == playerObject) continue; // drawn below
        if (object.imageIndex == 0xFFFFFFFFu) {
            if (object.projectile == mmu::SpawnedObjectKind::None) continue;
            // Runtime projectiles whose art path is not in the packed image
            // table still need to be visible in the harness viewport.
            if (object.projectile == mmu::SpawnedObjectKind::WilyMachineLightning)
                SDL_SetRenderDrawColor(renderer, 120, 220, 255, 255);
            else if (object.projectile == mmu::SpawnedObjectKind::WilyMachinePillar)
                SDL_SetRenderDrawColor(renderer, 255, 150, 60, 255);
            else
                SDL_SetRenderDrawColor(renderer, 255, 230, 90, 255);
            const int w = std::max(2, static_cast<int>(std::lround(6.0f * scale)));
            const int h = object.projectile == mmu::SpawnedObjectKind::WilyMachineLightning
                              ? std::max(8, static_cast<int>(std::lround(48.0f * scale)))
                              : std::max(4, static_cast<int>(std::lround(12.0f * scale)));
            const SDL_Rect marker{sx(object.x) - w / 2, sy(object.y) - h, w, h};
            SDL_RenderFillRect(renderer, &marker);
            continue;
        }
        SDL_Texture* texture = cache.get(state.pak, object.imageIndex);
        if (!texture) continue;
        const mmu::PackedEntry& image = state.pak.entries()[object.imageIndex];
        const int w = std::max(1, static_cast<int>(std::lround(image.width * scale)));
        const int h = std::max(1, static_cast<int>(std::lround(image.height * scale)));
        SDL_Rect dst{sx(object.x) - w / 2, sy(object.y) - h, w, h};
        if (dst.x + w < vp.x || dst.x > vp.x + vp.w ||
            dst.y + h < vp.y || dst.y > vp.y + vp.h) continue;
        SDL_RenderCopyEx(renderer, texture, nullptr, &dst, 0.0, nullptr,
                         object.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
    }

    const mmu::PlayerController& pc = state.world.controller();
    const int px = sx(pc.x), py = sy(pc.y);
    const mmu::PackedFrame* playerFrame = pc.currentFrame();
    SDL_Texture* playerTexture = playerFrame ? cache.get(state.pak, playerFrame->imageIndex) : nullptr;
    // Post-hurt invincibility blinks the sprite (harness approximation).
    const bool blinkHidden = pc.invincible && static_cast<int>(state.world.time() * 30.0f) % 2;
    if (playerTexture && pc.sprite) {
        if (!blinkHidden) {
            const mmu::PackedEntry& image = state.pak.entries()[playerFrame->imageIndex];
            const int w = std::max(1, static_cast<int>(std::lround(image.width * scale)));
            const int h = std::max(1, static_cast<int>(std::lround(image.height * scale)));
            // The handle (feet) sits at the controller position; mirrored
            // sprites mirror the handle too.
            const int handleX = pc.facingRight ? pc.sprite->xHandle
                                               : static_cast<int>(image.width) - pc.sprite->xHandle;
            const SDL_Rect dst{sx(std::floor(pc.x) - handleX + playerFrame->xOffset),
                               sy(std::floor(pc.y) - pc.sprite->yHandle + playerFrame->yOffset), w, h};
            SDL_RenderCopyEx(renderer, playerTexture, nullptr, &dst, 0.0, nullptr,
                             pc.facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 120, 255, 140, 255);
        const SDL_Rect marker{px - 2, py - 6, 4, 6};
        SDL_RenderFillRect(renderer, &marker);
    }

    // Harness-only diagnostic bars, drawn with primitive rects so they do not
    // depend on ImGui. These are not the original game's HUD.
    const int hudY = vp.y + 6;
    SDL_SetRenderDrawColor(renderer, 40, 44, 56, 255);
    const SDL_Rect barBack{vp.x + 6, hudY, 120, 6};
    SDL_Rect barFill = barBack;
    SDL_RenderFillRect(renderer, &barBack);
    SDL_SetRenderDrawColor(renderer, 240, 80, 80, 255);
    barFill.w = static_cast<int>(120.0f * std::clamp(pc.health / 100.0f, 0.0f, 1.0f));
    SDL_RenderFillRect(renderer, &barFill);
    SDL_SetRenderDrawColor(renderer, 40, 44, 56, 255);
    const SDL_Rect energyBack{vp.x + 6, hudY + 8, 120, 4};
    SDL_Rect energyFill = energyBack;
    SDL_RenderFillRect(renderer, &energyBack);
    SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
    energyFill.w = static_cast<int>(120.0f * std::clamp(pc.weaponEnergy / 100.0f, 0.0f, 1.0f));
    SDL_RenderFillRect(renderer, &energyFill);
}

void drawFlowMenuOverlay(const GameRuntime& runtime, const AppState& state) {
    const mmu::GameFlowRuntime& flow = state.flowRuntime;
    const SDL_Rect vp = gameViewport(g_renderer);
    const float width = static_cast<float>(vp.w);
    const auto center = [&](const char* text, float y, ImVec4 color) {
        const float x = (width - ImGui::CalcTextSize(text).x) * 0.5f;
        ImGui::SetCursorPos(ImVec2(x, y));
        ImGui::TextColored(color, "%s", text);
    };

    char header[160];
    std::snprintf(header, sizeof(header), "%s / %s",
                  flowScreenName(flow.screen()), flowStateName(flow.state()));
    center(header, static_cast<float>(vp.h) * 0.22f, ImVec4(0.68f, 0.80f, 1.0f, 1.0f));

    int rows = 0;
    const std::string className = flowScreenName(flow.screen());
    for (int i = 0; i < 10; ++i) {
        const mmu::RecoveredMenuItem* item =
            mmu::findMenuItem(className, static_cast<mmu::u16>(i), flow.menuSupplier());
        if (!item || item->label.empty()) continue;
        const bool selected = i == flow.selection();
        char line[192];
        std::snprintf(line, sizeof(line), "%s %s", selected ? ">" : " ",
                      std::string(item->label).c_str());
        center(line, static_cast<float>(vp.h) * 0.32f + rows * 14.0f,
               selected ? ImVec4(1.0f, 0.88f, 0.35f, 1.0f)
                        : ImVec4(0.92f, 0.94f, 1.0f, 1.0f));
        ++rows;
    }
    if (rows == 0) {
        if (flow.screen() == mmu::FlowScreen::TitleMenu)
            center("PRESS ENTER / A", static_cast<float>(vp.h) * 0.40f,
                   ImVec4(1.0f, 0.88f, 0.35f, 1.0f));
        else if (flow.screen() == mmu::FlowScreen::MenuStageSelect) {
            char line[96];
            const int stage = flow.stageSelectStage();
            if (stage >= 0)
                std::snprintf(line, sizeof(line), "STAGE SELECT  STAGE %d", stage);
            else
                std::snprintf(line, sizeof(line), "STAGE SELECT  NO STAGE");
            center(line, static_cast<float>(vp.h) * 0.40f,
                   ImVec4(1.0f, 0.88f, 0.35f, 1.0f));
        }
    }

    if (flow.screen() != mmu::FlowScreen::TitleMenu) {
        const mmu::RecoveredMenuItem* selectedItem = mmu::findMenuItem(
            className, static_cast<mmu::u16>(flow.selection()), flow.menuSupplier());
        if (selectedItem && selectedItem->actionCode != 0) {
            const mmu::RecoveredMenuAction* action =
                mmu::findMenuAction(selectedItem->actionCode);
            char line[192];
            std::snprintf(line, sizeof(line), "action=0x%02x %s",
                          selectedItem->actionCode,
                          action ? mmu::menuActionKindName(action->kind) : "unknown");
            center(line, static_cast<float>(vp.h) * 0.82f,
                   ImVec4(0.55f, 0.62f, 0.72f, 1.0f));
        }
    }

    center(runtime.mode == GameMode::Paused ? "PAUSED - ESC / START TO RESUME"
                                            : "EVIDENCE-BACKED FLOW (harness)",
           static_cast<float>(vp.h) * 0.92f,
           ImVec4(0.55f, 0.62f, 0.72f, 1.0f));
}

void drawGameOverlay(const GameRuntime& runtime, const AppState& state) {
    const SDL_Rect vp = gameViewport(g_renderer);
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(vp.x), static_cast<float>(vp.y)));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(vp.w), static_cast<float>(vp.h)));
    ImGui::Begin("game-overlay", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);
    const mmu::PlayerController& pc = state.world.controller();
    ImGui::SetCursorPos(ImVec2(8.0f, 18.0f));
    ImGui::Text("HP %3.0f  W%02d  %s", pc.health, pc.weapon + 1,
                mmu::playerStateName(pc.state));
    ImGui::SetCursorPos(ImVec2(8.0f, 36.0f));
    ImGui::TextWrapped("DEBUG HARNESS - NOT THE GAME");
    if (runtime.mode == GameMode::Title || runtime.mode == GameMode::Paused) {
        drawFlowMenuOverlay(runtime, state);
    } else if (runtime.mode == GameMode::GameOver) {
        ImGui::SetCursorPos(ImVec2(0.0f, static_cast<float>(vp.h) * 0.46f));
        ImGui::SetCursorPosX((vp.w - ImGui::CalcTextSize("GAME OVER").x) * 0.5f);
        ImGui::TextUnformatted("GAME OVER\nretrying...");
    }
    ImGui::SetCursorPos(ImVec2(8.0f, static_cast<float>(vp.h) - 18.0f));
    ImGui::TextDisabled("ZL+R3: %s menu", state.uiState.menuBarVisible ? "hide" : "show");
    ImGui::End();
}

void drawLevelViewer(AppState& state, TextureCache& cache) {
    LevelViewerState& view = state.levelViewer;
    if (!view.open) return;
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowLevelViewer,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Level Viewer", &view.open);
    if (!state.pak.valid() || state.pak.scenes().empty()) {
        ImGui::TextWrapped("No level data in assets.pak. Re-bake with mmu_bake "
                           "(MMUPAK v3+ includes scene names and spawn tables).");
        ImGui::End();
        return;
    }

    ImGui::Columns(2, "level-columns", true);
    ImGui::InputText("Filter", view.filter, IM_ARRAYSIZE(view.filter));
    ImGui::BeginChild("scene list", ImVec2(0, -8), ImGuiChildFlags_Borders);
    const std::string filter = view.filter;
    for (std::size_t i = 0; i < state.pak.scenes().size(); ++i) {
        const mmu::PackedScene& scene = state.pak.scenes()[i];
        char label[160];
        if (!scene.name.empty()) {
            std::string name(scene.name);
            const std::size_t slash = name.find_last_of("\\/");
            if (slash != std::string::npos) name = name.substr(slash + 1);
            std::snprintf(label, sizeof(label), "#%zu  %-28s %u spawns",
                          i, name.c_str(), scene.spawnCount);
        } else {
            std::snprintf(label, sizeof(label), "#%zu  key=%u  %u spawns",
                          i, scene.key, scene.spawnCount);
        }
        if (!filter.empty() && std::string(label).find(filter) == std::string::npos) continue;
        if (ImGui::Selectable(label, view.selected == static_cast<int>(i))) {
            view.selected = static_cast<int>(i);
            view.hoveredSpawn = -1;
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();
    if (view.selected < 0 || static_cast<std::size_t>(view.selected) >= state.pak.scenes().size()) {
        ImGui::TextWrapped("Select a scene on the left, or use 'level <key>' in the console.");
        ImGui::End();
        return;
    }
    const mmu::PackedScene& scene = state.pak.scenes()[view.selected];
    ImGui::Text("scene #%d  key=%u  spawns=%u", view.selected, scene.key, scene.spawnCount);
    if (!scene.name.empty()) ImGui::TextWrapped("%.*s",
        static_cast<int>(scene.name.size()), scene.name.data());
    if (!sceneArtFor(state, static_cast<u32>(view.selected)).empty())
        ImGui::Text("background: %u frame(s) %ux%u",
                    static_cast<u32>(sceneArtFor(state, static_cast<u32>(view.selected)).frames.size()),
                    sceneArtFor(state, static_cast<u32>(view.selected)).width,
                    sceneArtFor(state, static_cast<u32>(view.selected)).height);
    ImGui::Checkbox("Grid", &view.showGrid); ImGui::SameLine();
    ImGui::Checkbox("Labels", &view.showLabels); ImGui::SameLine();
    ImGui::Checkbox("Images", &view.showImages); ImGui::SameLine();
    ImGui::Checkbox("Background", &view.showBackground); ImGui::SameLine();
    ImGui::Checkbox("Bottom anchor", &view.anchorBottom);
    ImGui::SliderFloat("Zoom", &view.scale, 0.25f, 8.0f, "%.2fx");
    ImGui::SameLine();
    if (ImGui::Button("Reset view")) {
        view.scale = 2.0f;
        view.pan = ImVec2(0.0f, 0.0f);
    }
    ImGui::Text("Drag to pan, wheel to zoom. Yellow box = 256x224 screen. pan=(%.0f, %.0f)",
                view.pan.x, view.pan.y);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 canvasSize(avail.x > 32.0f ? avail.x : 32.0f,
                            avail.y > 152.0f ? avail.y - 140.0f : 60.0f);
    const ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("level canvas", canvasSize);
    const bool canvasHovered = ImGui::IsItemHovered();
    ImGuiIO& io = ImGui::GetIO();
    if (canvasHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        view.pan.x -= io.MouseDelta.x / view.scale;
        view.pan.y -= io.MouseDelta.y / view.scale;
    }
    if (canvasHovered && io.MouseWheel != 0.0f) {
        const float mouseX = io.MousePos.x - canvasPos.x;
        const float mouseY = io.MousePos.y - canvasPos.y;
        const float worldX = view.pan.x + mouseX / view.scale;
        const float worldY = view.pan.y + mouseY / view.scale;
        float scale = view.scale * (1.0f + io.MouseWheel * 0.15f);
        if (scale < 0.25f) scale = 0.25f;
        if (scale > 8.0f) scale = 8.0f;
        view.scale = scale;
        view.pan.x = worldX - mouseX / view.scale;
        view.pan.y = worldY - mouseY / view.scale;
    }

    const ImVec2 canvasEnd(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(canvasPos, canvasEnd, true);
    drawList->AddRectFilled(canvasPos, canvasEnd, IM_COL32(22, 24, 32, 255));
    const auto toScreen = [&](float wx, float wy) -> ImVec2 {
        return ImVec2(canvasPos.x + (wx - view.pan.x) * view.scale,
                      canvasPos.y + (wy - view.pan.y) * view.scale);
    };

    const float worldLeft = view.pan.x;
    const float worldTop = view.pan.y;
    const float worldRight = view.pan.x + canvasSize.x / view.scale;
    const float worldBottom = view.pan.y + canvasSize.y / view.scale;
    if (view.showBackground)
        drawSceneBackground(cache, state, static_cast<u32>(view.selected), drawList,
                            toScreen, worldLeft, worldRight, view.scale, 0.0f, false);
    if (view.showGrid) {
        const int x0 = static_cast<int>(std::floor(worldLeft / 8.0f)) * 8;
        const int y0 = static_cast<int>(std::floor(worldTop / 8.0f)) * 8;
        for (int gx = x0; gx <= worldRight; gx += 8) {
            const ImU32 color = gx % 16 == 0 ? IM_COL32(70, 74, 92, 255)
                                             : IM_COL32(42, 45, 58, 255);
            drawList->AddLine(toScreen(static_cast<float>(gx), worldTop),
                              toScreen(static_cast<float>(gx), worldBottom), color, 1.0f);
        }
        for (int gy = y0; gy <= worldBottom; gy += 8) {
            const ImU32 color = gy % 16 == 0 ? IM_COL32(70, 74, 92, 255)
                                             : IM_COL32(42, 45, 58, 255);
            drawList->AddLine(toScreen(worldLeft, static_cast<float>(gy)),
                              toScreen(worldRight, static_cast<float>(gy)), color, 1.0f);
        }
        drawList->AddRect(toScreen(0, 0), toScreen(256, 224),
                          IM_COL32(255, 214, 80, 255), 1.0f, 0, 2.0f);
    }

    int hovered = -1;
    for (u32 i = 0; i < scene.spawnCount; ++i) {
        const mmu::PackedSpawn& spawn = scene.spawns[i];
        const ImVec2 p = toScreen(spawn.x, spawn.y);
        const auto found = state.typeImages.find(mmu::toLowerAscii(spawn.type));
        if (view.showImages && found != state.typeImages.end()) {
            if (SDL_Texture* texture = cache.get(state.pak, found->second)) {
                const mmu::PackedEntry& image = state.pak.entries()[found->second];
                const float w = static_cast<float>(image.width) * view.scale;
                const float h = static_cast<float>(image.height) * view.scale;
                const ImVec2 pos(p.x - w * 0.5f,
                                 p.y - (view.anchorBottom ? h : h * 0.5f));
                drawList->AddImage(reinterpret_cast<ImTextureID>(texture), pos,
                                   ImVec2(pos.x + w, pos.y + h));
            }
        }
        const ImU32 marker = spawn.collectible ? IM_COL32(255, 214, 80, 255)
                           : spawn.facingRight ? IM_COL32(120, 220, 255, 255)
                                               : IM_COL32(120, 255, 160, 255);
        drawList->AddCircleFilled(p, 3.0f, marker, 8);
        if (view.showLabels && view.scale >= 0.75f) {
            const std::string label(spawn.type);
            drawList->AddText(ImVec2(p.x + 5.0f, p.y - 7.0f),
                              IM_COL32(230, 230, 235, 255), label.c_str());
        }
        if (canvasHovered && ImGui::IsMouseHoveringRect(
                ImVec2(p.x - 6.0f, p.y - 6.0f), ImVec2(p.x + 6.0f, p.y + 6.0f)))
            hovered = static_cast<int>(i);
    }
    drawList->PopClipRect();
    if (hovered >= 0) view.hoveredSpawn = hovered;

    ImGui::BeginChild("spawn list", ImVec2(0, 132), ImGuiChildFlags_Borders);
    for (u32 i = 0; i < scene.spawnCount; ++i) {
        const mmu::PackedSpawn& spawn = scene.spawns[i];
        char line[200];
        std::snprintf(line, sizeof(line), "%-20s (%6.0f,%5.0f)%s%s",
                      std::string(spawn.type).c_str(), spawn.x, spawn.y,
                      spawn.collectible ? " pickup" : "",
                      spawn.facingRight ? " right" : "");
        if (ImGui::Selectable(line, view.hoveredSpawn == static_cast<int>(i)))
            view.hoveredSpawn = static_cast<int>(i);
    }
    ImGui::EndChild();

    if (hovered >= 0) {
        const mmu::PackedSpawn& spawn = scene.spawns[hovered];
        ImGui::SetTooltip("%s\npos=(%.1f, %.1f) auto=%.1f\nattr1=%s  attr2=%s\n"
                          "respawn death=%d screen=%d  delete offscreen=%d  collectible=%d",
                          std::string(spawn.type).c_str(), spawn.x, spawn.y, spawn.autoRespawn,
                          std::string(spawn.attr1).c_str(), std::string(spawn.attr2).c_str(),
                          spawn.respawnOnDeath, spawn.respawnOnScreen,
                          spawn.deleteOffScreen, spawn.collectible);
    }
    ImGui::End();
}

void drawGameView(AppState& state, TextureCache& cache, float delta) {
    GameViewState& view = state.gameView;
    if (!view.open) return;
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowHarnessView,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Harness View", &view.open);
    if (!state.pak.valid() || state.pak.scenes().empty()) {
        ImGui::TextWrapped("No level data in assets.pak. Re-bake with mmu_bake (MMUPAK v3+).");
        ImGui::End();
        return;
    }

    ImGui::InputInt("Scene #", &view.sceneIndex);
    if (view.sceneIndex < 0) view.sceneIndex = 0;
    if (view.sceneIndex >= static_cast<int>(state.pak.scenes().size()))
        view.sceneIndex = static_cast<int>(state.pak.scenes().size()) - 1;
    ImGui::SameLine();
    if (ImGui::Button("Load Scene")) {
        state.world.loadScene(state.pak, state.pak.scenes()[view.sceneIndex], state.typeImages);
        state.levelViewer.selected = view.sceneIndex;
        view.accumulator = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Use Level Viewer selection") && state.levelViewer.selected >= 0) {
        view.sceneIndex = state.levelViewer.selected;
        state.world.loadScene(state.pak, state.pak.scenes()[view.sceneIndex], state.typeImages);
        view.accumulator = 0.0f;
    }

    if (!state.world.hasScene()) {
        ImGui::TextWrapped("Load a scene to instantiate its live objects.");
        ImGui::End();
        return;
    }
    const mmu::WorldObject* player = state.world.player();
    ImGui::Text("scene key=%u  objects=%zu  time=%.2fs",
                state.world.sceneKey(), state.world.objects().size(), state.world.time());
    const mmu::PlayerController& pc = state.world.controller();
    ImGui::Text("player: %s  hp=%.0f  weapon=%d  energy=%.0f  %s",
                mmu::playerStateName(pc.state), pc.health, pc.weapon + 1,
                pc.weaponEnergy, pc.facingRight ? "facing right" : "facing left");
    const mmu::CollisionMap& col = state.world.collision();
    ImGui::Text("collision: %s  player solid=%s  grounded=%s",
                col.valid() ? "packed image" : "spawn floor fallback",
                col.valid() && col.solidAt(pc.x, pc.y) ? "yes" : "no",
                pc.grounded ? "yes" : "no");
    const auto& enemies = state.world.enemies();
    ImGui::Text("enemies=%zu  (use 'enemies' in the console to inspect states)",
                enemies.size());
    const std::string_view sceneName = state.pak.scenes()[view.sceneIndex].name;
    if (!sceneName.empty()) ImGui::TextWrapped("%.*s",
        static_cast<int>(sceneName.size()), sceneName.data());
    ImGui::TextWrapped("This is the debug harness, not the reconstructed game. "
                       "It steps the world at 60 Hz for testing. Use the harness card, "
                       "the console 'play' command, or Load Scene; F1 toggles the debug UI.");

    ImGui::Separator();
    ImGui::Text("Recovered flow core (not the harness card)");
    const mmu::GameFlowRuntime& flow = state.flowRuntime;
    ImGui::Text("screen=%s state=%s selection=%d",
                mmu::flowScreenName(flow.screen()),
                mmu::flowStateName(flow.state()), flow.selection());
    const std::string dispatcherName(flow.dispatcher().currentName());
    const std::string dispatcherPrevious(flow.dispatcher().previousName());
    ImGui::Text("dispatcher=%s/0x%08x previous=%s",
                dispatcherName.c_str(), flow.dispatcher().currentHash(),
                dispatcherPrevious.c_str());
    ImGui::Text("level phase=%s cue=%s level cue=%s events=%zu running=%s",
                mmu::flowLevelPhaseName(flow.levelPhase()),
                mmu::flowCueName(flow.lastCue()),
                mmu::flowLevelCueName(flow.lastLevelCue()),
                flow.pendingEventCount(), flow.running() ? "yes" : "no");
    if (ImGui::Button("Confirm")) {
        mmu::FlowInput input{};
        input.confirm = true;
        state.flowRuntime.step(1.0f / 60.0f, input);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        mmu::FlowInput input{};
        input.cancel = true;
        state.flowRuntime.step(1.0f / 60.0f, input);
    }
    ImGui::SameLine();
    if (ImGui::Button("Step 0.5s")) state.flowRuntime.step(0.5f, mmu::FlowInput{});
    ImGui::SameLine();
    if (ImGui::Button("Drain events")) state.flowRuntime.clearEvents();
    ImGui::TextWrapped("This executes the evidence-driven menu core. Terminal states emit "
                       "events; unresolved screen/level destinations are not guessed.");

    ImGui::Checkbox("Follow player", &view.followPlayer);
    ImGui::SameLine(); ImGui::Checkbox("Labels", &view.showLabels);
    ImGui::SameLine(); ImGui::Checkbox("Background", &view.showBackground);
    ImGui::SliderFloat("Zoom", &view.zoom, 0.5f, 6.0f, "%.2fx");

    mmu::Camera& camera = state.world.camera();
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 canvasSize(avail.x > 32.0f ? avail.x : 32.0f,
                            avail.y > 32.0f ? avail.y : 32.0f);
    const ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("game canvas", canvasSize);
    if (!view.followPlayer && ImGui::IsItemHovered() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        camera.x -= ImGui::GetIO().MouseDelta.x / view.zoom;
        camera.y -= ImGui::GetIO().MouseDelta.y / view.zoom;
        camera.clamp(state.world.worldWidth(), state.world.worldHeight());
    }
    const ImVec2 canvasEnd(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(canvasPos, canvasEnd, true);
    drawList->AddRectFilled(canvasPos, canvasEnd, IM_COL32(18, 20, 28, 255));
    const auto toScreen = [&](float wx, float wy) -> ImVec2 {
        return ImVec2(canvasPos.x + (wx - camera.x) * view.zoom,
                      canvasPos.y + (wy - camera.y) * view.zoom);
    };

    const float worldLeft = camera.x;
    const float worldTop = camera.y;
    const float worldRight = camera.x + canvasSize.x / view.zoom;
    const float worldBottom = camera.y + canvasSize.y / view.zoom;
    if (view.showBackground)
        drawSceneBackground(cache, state, static_cast<u32>(view.sceneIndex), drawList,
                            toScreen, worldLeft, worldRight, view.zoom, state.world.time(), true);
    const int gx0 = static_cast<int>(std::floor(worldLeft / 8.0f)) * 8;
    const int gy0 = static_cast<int>(std::floor(worldTop / 8.0f)) * 8;
    for (int gx = gx0; gx <= worldRight; gx += 8) {
        drawList->AddLine(toScreen(static_cast<float>(gx), worldTop),
                          toScreen(static_cast<float>(gx), worldBottom),
                          gx % 16 == 0 ? IM_COL32(58, 62, 78, 255) : IM_COL32(34, 37, 48, 255), 1.0f);
    }
    for (int gy = gy0; gy <= worldBottom; gy += 8) {
        drawList->AddLine(toScreen(worldLeft, static_cast<float>(gy)),
                          toScreen(worldRight, static_cast<float>(gy)),
                          gy % 16 == 0 ? IM_COL32(58, 62, 78, 255) : IM_COL32(34, 37, 48, 255), 1.0f);
    }
    drawList->AddRect(toScreen(0, 0),
                      toScreen(state.world.worldWidth(), state.world.worldHeight()),
                      IM_COL32(90, 100, 130, 255), 1.0f, 0, 1.0f);

    std::vector<const mmu::EnemyController*> enemyByObject(state.world.objects().size(), nullptr);
    for (const mmu::EnemyController& enemy : enemies)
        if (enemy.objectIndex < enemyByObject.size())
            enemyByObject[enemy.objectIndex] = &enemy;

    for (std::size_t objectIndex = 0; objectIndex < state.world.objects().size(); ++objectIndex) {
        const mmu::WorldObject& object = state.world.objects()[objectIndex];
        const ImVec2 p = toScreen(object.x, object.y);
        const bool isPlayer = &object == player;
        if (object.imageIndex != 0xFFFFFFFFu) {
            if (SDL_Texture* texture = cache.get(state.pak, object.imageIndex)) {
                const mmu::PackedEntry& image = state.pak.entries()[object.imageIndex];
                const float w = static_cast<float>(image.width) * view.zoom;
                const float h = static_cast<float>(image.height) * view.zoom;
                const ImVec2 pos(p.x - w * 0.5f, p.y - h);
                drawList->AddImage(reinterpret_cast<ImTextureID>(texture), pos,
                                   ImVec2(pos.x + w, pos.y + h), ImVec2(0, 0), ImVec2(1, 1),
                                   object.facingRight ? IM_COL32(255, 255, 255, 255)
                                                      : IM_COL32(190, 215, 255, 255));
            }
        } else {
            drawList->AddRectFilled(ImVec2(p.x - 6.0f, p.y - 14.0f),
                                    ImVec2(p.x + 6.0f, p.y - 2.0f),
                                    object.collectible ? IM_COL32(255, 214, 80, 255)
                                                       : IM_COL32(110, 150, 210, 255));
        }
        drawList->AddCircleFilled(p, isPlayer ? 4.0f : 2.0f,
                                  isPlayer ? IM_COL32(120, 255, 140, 255)
                                           : IM_COL32(220, 120, 120, 255), 8);
        if (view.showLabels) {
            std::string label(object.type);
            if (const mmu::EnemyController* enemy = enemyByObject[objectIndex])
                label += std::string(" [") + mmu::enemyStateName(enemy->state) + "]";
            drawList->AddText(ImVec2(p.x + 5.0f, p.y - 8.0f),
                              IM_COL32(225, 225, 230, 255), label.c_str());
        }
    }
    drawList->PopClipRect();
    ImGui::Text("Runtime: arrows/A/D move, Space/Z jump, S/Down climb, X/Shift dash, "
                "LCTRL/C fire, ESC/P pause. camera=(%.0f,%.0f) player=%s (%.0f,%.0f)",
                camera.x, camera.y, player ? std::string(player->type).c_str() : "(none)",
                player ? player->x : 0.0f, player ? player->y : 0.0f);
    ImGui::End();
}

void drawAudio(AppState& state) {
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowAudio,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Audio");
    const mmu::sdl2::AudioStatus status = state.audio.status();
    ImGui::Text("device: %s   %d Hz, %d channels",
                status.opened ? "open" : "closed", status.frequency, status.channels);
    if (!status.opened) {
        if (ImGui::Button("Open Audio")) state.audio.open();
        ImGui::End();
        return;
    }

    int volume = status.masterVolume;
    if (ImGui::SliderInt("Master volume", &volume, 0, 128))
        state.audio.setMasterVolume(volume);
    ImGui::Text("active voices: %d", status.activeVoices);
    ImGui::Text("music: %s", status.music.empty() ? "(none)" : status.music.c_str());
    if (ImGui::Button("Stop all")) state.audio.stopAll();
    ImGui::SameLine();
    if (ImGui::Button("Stop music")) state.audio.stopMusic();

    if (!state.pak.valid()) {
        ImGui::TextWrapped("No packed archive loaded; cannot list sounds.");
        ImGui::End();
        return;
    }

    static char filter[128] = {};
    ImGui::InputText("Filter", filter, IM_ARRAYSIZE(filter));
    ImGui::BeginChild("audio list", ImVec2(0, -8), ImGuiChildFlags_Borders);
    const std::string filterText = filter;
    for (std::size_t i = 0; i < state.pak.entries().size(); ++i) {
        const mmu::PackedEntry& entry = state.pak.entries()[i];
        const bool isSfx = entry.type == mmu::PackedEntryType::Audio;
        const bool isMusic = entry.type == mmu::PackedEntryType::Music;
        if (!isSfx && !isMusic) continue;
        const std::string label = std::string(isSfx ? "SFX  " : "MUS  ")
            + std::to_string(entry.key) + "  " + std::string(entry.name);
        if (!filterText.empty() && label.find(filterText) == std::string::npos) continue;
        if (ImGui::Selectable(label.c_str(), false)) {
            if (isSfx) state.audio.playSfx(state.pak, entry.key);
            else state.audio.playMusic(state.pak, entry.key);
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void drawConsole(AppState& state) {
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowConsole,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Debug Console");
    ImGui::BeginChild("log", ImVec2(0, -32), ImGuiChildFlags_Borders);
    for (const std::string& line : state.console.logLines())
        ImGui::TextUnformatted(line.c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 8.0f)
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();

    static char input[512] = {};
    const ImGuiID commandId = ImGui::GetID("##console-input");
    switchui::noteCommandField(commandId);

    // A command submitted with the Switch keyboard's Send button is handed over
    // verbatim. Put it in the field for this frame so the log shows exactly the
    // text that was executed, then clear it after execution.
    char keyboardCommand[256] = {};
    const bool keyboardSubmitted =
        switchui::takeSubmittedCommand(keyboardCommand, sizeof(keyboardCommand));
    if (keyboardSubmitted)
        std::snprintf(input, sizeof(input), "%s", keyboardCommand);

    ImGui::SetNextItemWidth(-90.0f);
    const bool enterSubmitted = ImGui::InputText(
        "##console-input", input, IM_ARRAYSIZE(input),
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    const bool sendPressed = ImGui::Button("Send");

    if (enterSubmitted || sendPressed || keyboardSubmitted) {
        if (input[0] != '\0') state.console.execute(input);
        input[0] = '\0';
#if !defined(__SWITCH__) && !defined(__SWITCH2__)
        // Keep desktop typing flowing. On Switch this would refocus the field
        // and immediately reopen the applet, so leave it blurred.
        ImGui::SetKeyboardFocusHere(-1);
#endif
    }
    ImGui::End();
}

void drawFlowEvidence(AppState& state) {
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowFlowEvidence,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Flow Evidence");
    static char filter[128] = {};
    ImGui::InputText("Filter", filter, IM_ARRAYSIZE(filter));
    const std::string needle = filter;
    std::size_t setStateCount = 0;
    std::size_t cueCount = 0;
    for (const mmu::RegisteredStateTransition& row :
         mmu::kRegisteredStateTransitions) {
        if ((row.flags & mmu::kFlowTransitionSetState) != 0) ++setStateCount;
        if ((row.flags & mmu::kFlowTransitionCue) != 0) ++cueCount;
    }
    ImGui::Text("observed setter calls: %zu (setState %zu, cue %zu)",
                mmu::kRegisteredStateTransitionCount, setStateCount, cueCount);
    ImGui::TextDisabled("Evidence only; no timing or destination is inferred.");
    ImGui::Separator();

    if (ImGui::BeginTable("flow-transitions", 6,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Class");
        ImGui::TableSetupColumn("Slot");
        ImGui::TableSetupColumn("Callback");
        ImGui::TableSetupColumn("Call");
        ImGui::TableSetupColumn("Kind");
        ImGui::TableSetupColumn("Target");
        ImGui::TableHeadersRow();

        for (const mmu::RegisteredStateTransition& row :
             mmu::kRegisteredStateTransitions) {
            const std::string className = std::string(row.className);
            const std::string targetName = std::string(row.targetName);
            if (!needle.empty() &&
                className.find(needle) == std::string::npos &&
                targetName.find(needle) == std::string::npos)
                continue;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(className.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%u", row.vtableSlot);
            ImGui::TableNextColumn();
            ImGui::Text("0x%06x", row.functionVa);
            ImGui::TableNextColumn();
            ImGui::Text("0x%06x", row.callVa);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(mmu::flowTransitionKindName(row.flags));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(targetName.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void drawCutscene(AppState& state, TextureCache& cache) {
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowCutsceneRuntime,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Cutscene Runtime");
    mmu::CutsceneRuntime& cutscene = state.cutsceneRuntime;
    cutscene.attach(state.assets);

    static std::vector<std::string> labels;
    static std::vector<const char*> labelPtrs;
    if (labels.empty()) {
        for (const mmu::RegisteredCutscene& row : mmu::kRegisteredCutscenes)
            labels.push_back(std::string(row.path));
        for (const std::string& label : labels) labelPtrs.push_back(label.c_str());
    }
    static int selected = 0;
    ImGui::Combo("Scene", &selected, labelPtrs.data(),
                 static_cast<int>(labelPtrs.size()));
    if (ImGui::Button("Start", ImVec2(100.0f, 0.0f)))
        cutscene.startKey(mmu::kRegisteredCutscenes[selected].key);
    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(100.0f, 0.0f))) cutscene.stop();
    ImGui::SameLine();
    if (ImGui::Button("Step 0.1s", ImVec2(110.0f, 0.0f))) cutscene.update(0.1f);

    ImGui::Separator();
    ImGui::Text("playing=%s  clock=%.0f ms  timestamp=%zu",
                cutscene.playing() ? "yes" : "no", cutscene.clockMs(),
                cutscene.currentTimestamp());
    ImGui::Text("scene: %.*s", static_cast<int>(cutscene.sceneName().size()),
                cutscene.sceneName().data());
    ImGui::Text("elements=%zu  texts=%zu  queued events=%zu",
                cutscene.elements().size(), cutscene.texts().size(),
                state.cutsceneEventLog.size());
    ImGui::TextDisabled("Use `cutscene events` in the console to drain decoded commands.");
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Elements")) {
        for (const mmu::CutsceneElement& element : cutscene.elements()) {
            const mmu::CutsceneSpriteRef ref = mmu::resolveCutsceneSprite(
                state.pak, cutscene.sceneName(), element.resource, element.anim,
                element.id);
            SDL_Texture* texture = ref.valid() ? cache.get(state.pak, ref.imageIndex)
                                               : nullptr;
            if (texture != nullptr) {
                const mmu::PackedEntry& image = state.pak.entries()[ref.imageIndex];
                const float longest = static_cast<float>(
                    std::max(image.width, image.height));
                const float scale = longest > 0.0f ? 72.0f / longest : 1.0f;
                ImGui::Image(reinterpret_cast<ImTextureID>(texture),
                             ImVec2(static_cast<float>(image.width) * scale,
                                    static_cast<float>(image.height) * scale));
                ImGui::SameLine();
            }
            ImGui::Text("%s  res=%s  anim=%s  pos=(%.0f,%.0f)",
                        element.id.c_str(), element.resource.c_str(),
                        element.anim.c_str(), element.x, element.y);
            if (ref.valid()) {
                ImGui::Text("  obj=%08x sprite=%08x image=%u: %.*s",
                            ref.gameObjectKey, ref.spriteKey, ref.imageIndex,
                            static_cast<int>(ref.imageName.size()),
                            ref.imageName.data());
            } else if (ref.gameObjectKey != 0) {
                ImGui::TextDisabled("  obj=%08x resolved; no sprite/anim yet",
                                    ref.gameObjectKey);
            }
        }
    }
    if (ImGui::CollapsingHeader("Text")) {
        for (const mmu::CutsceneText& text : cutscene.texts())
            ImGui::Text("%s: %s", text.id.c_str(), text.value.c_str());
    }
    ImGui::End();
}

void drawInspector(AppState& state) {
    applyWindowLayout(state.uiState.windowLayoutPending, kWindowInspector,
                      state.ui, state.outputWidth, state.outputHeight);
    ImGui::Begin("Asset Inspector");
    const mmu::AssetStats& s = state.assets.stats();
    ImGui::Text("sprites: %zu   frames: %zu", s.sprites, s.animationFrames);
    ImGui::Text("game objects: %zu   sounds: %zu", s.gameObjects, s.sounds);
    ImGui::Text("scenes: %zu   spawns: %zu", s.scenes, s.spawns);
    ImGui::Text("cutscenes: %zu   commands: %zu", s.cutscenes, s.commands);
    if (state.pak.valid())
        ImGui::Text("packed entries: %zu   images: %zu",
                    state.pak.entries().size(), state.pak.imageCount());

    if (state.preview) {
        ImGui::Separator();
        ImGui::Text("Preview (%dx%d)", state.previewWidth, state.previewHeight);
        const float scale = 256.0f / static_cast<float>(state.previewWidth > state.previewHeight
                                                            ? state.previewWidth : state.previewHeight);
        ImGui::Image(reinterpret_cast<ImTextureID>(state.preview),
                     ImVec2(static_cast<float>(state.previewWidth) * scale,
                            static_cast<float>(state.previewHeight) * scale));
    }
    ImGui::End();
}

// Full-screen boot splash drawn between ImGui init and the first harness frame.
// Each stage presents a frame so the bar is on screen while the blocking asset
// load runs; no gameplay state is touched here.
void renderSplash(SDL_Renderer* renderer, float progress, const char* status) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();

    const ImVec2 view = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(view);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##BootSplash", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetWindowPos();
    const ImVec2 far(origin.x + view.x, origin.y + view.y);
    dl->AddRectFilled(origin, far, IM_COL32(9, 12, 20, 255));
    dl->AddRectFilled(origin, ImVec2(far.x, origin.y + 5.0f),
                      IM_COL32(52, 132, 214, 255));
    dl->AddRectFilled(ImVec2(origin.x, far.y - 5.0f), far,
                      IM_COL32(52, 132, 214, 255));

    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImFont* font = ImGui::GetFont();
    const auto centeredText = [&](const char* text, float size, float y, ImU32 color) {
        const ImVec2 dim = font->CalcTextSizeA(size, 10000.0f, 0.0f, text);
        dl->AddText(font, size, ImVec2(center.x - dim.x * 0.5f, center.y + y),
                    color, text);
    };

    centeredText("MMU RECOMP", 46.0f, -140.0f, IM_COL32(238, 244, 255, 255));
    centeredText("Reconstruction Debug Harness", 19.0f, -82.0f,
                 IM_COL32(102, 168, 235, 255));
    dl->AddLine(ImVec2(center.x - 220.0f, center.y - 50.0f),
                ImVec2(center.x + 220.0f, center.y - 50.0f),
                IM_COL32(52, 132, 214, 180), 2.0f);

    char pct[16];
    std::snprintf(pct, sizeof(pct), "%.0f%%", progress * 100.0f);
    centeredText(pct, 17.0f, -34.0f, IM_COL32(191, 212, 240, 255));
    centeredText(status, 17.0f, -6.0f, IM_COL32(214, 224, 240, 255));

    const float barW = 480.0f, barH = 16.0f;
    const ImVec2 barMin(center.x - barW * 0.5f, center.y + 28.0f);
    const ImVec2 barMax(barMin.x + barW, barMin.y + barH);
    dl->AddRectFilled(barMin, barMax, IM_COL32(24, 32, 48, 255), 6.0f);
    dl->AddRectFilled(barMin, ImVec2(barMin.x + barW * progress, barMax.y),
                      IM_COL32(52, 132, 214, 255), 6.0f);
    dl->AddRect(barMin, barMax, IM_COL32(70, 100, 140, 255), 6.0f, 1.0f);

    centeredText("Reconstructed from MMU.exe - debug harness, not the game", 12.0f,
                 96.0f, IM_COL32(120, 140, 168, 255));

    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

// The warning is controller-owned: Dpad/left stick choose, A confirms, B
// cancels. Mouse clicks still work. ImGui keyboard/gamepad nav is disabled for
// the frame so those same presses cannot activate a button a second time.
void drawQuitConfirm(AppState& state) {
    UiState& ui = state.uiState;
    if (!ui.quitConfirmOpen) {
        ui.quitPopupRequested = false;
        return;
    }
    if (!ui.quitPopupRequested) {
        ImGui::OpenPopup("Quit Game?");
        ui.quitPopupRequested = true;
    }

    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Quit Game?", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoMove)) {
        // Twice the normal size on console outputs, a touch larger on desktop;
        // the smaller console scale keeps the prompt inside a 1280px viewport.
        ImGui::SetWindowFontScale(ImGui::GetMainViewport()->Size.x < 1400.0f
                                      ? 2.0f : 2.4f);
        ImGui::TextUnformatted("Quit Game?");
        ImGui::Separator();
        ImGui::TextWrapped("Unsaved harness state will be lost. Exit the application now?");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.68f, 0.80f, 1.0f, 1.0f),
                           "D-Pad / Left Stick: Choose    A: Confirm    B: Cancel");
        ImGui::Spacing();

        const auto choiceButton = [&](const char* label, int choice) {
            const bool selected = ui.quitConfirmSelection == choice;
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.42f, 0.70f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.52f, 0.84f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.34f, 0.60f, 1.0f));
            }
            const bool clicked = ImGui::Button(label, ImVec2(160.0f, 0.0f));
            if (selected) ImGui::PopStyleColor(3);
            if (choice == 1) ImGui::SetItemDefaultFocus();
            if (clicked) {
                if (choice == 0) state.quit = true;
                ui.quitConfirmOpen = false;
                ImGui::CloseCurrentPopup();
            }
        };

        choiceButton("Yes", 0);
        ImGui::SameLine();
        choiceButton("No", 1);
        ImGui::EndPopup();
    }
}

} // namespace

SDL_Renderer* g_renderer = nullptr;

int main(int argc, char** argv) {
#if defined(__SWITCH__) || defined(__SWITCH2__)
    romfsMountSelf("romfs"); // makes romfs:/ visible to std::ifstream
#endif
    AppState state;
    bool selftest = false;
    float autoQuitSeconds = 0.0f;
    std::string screenshotPath; // --screenshot=<file.bmp>, saved on auto-quit
    bool presetSprite = false, presetSfx = false, presetMusic = false, presetLevel = false, presetPlay = false;
    bool forceTouch = false, forceDesktop = false;
    std::vector<std::string> execCommands;
    if (const char* env = std::getenv("MMU_TOUCH_UI")) forceTouch = std::atoi(env) != 0;
    if (const char* env = std::getenv("MMU_DESKTOP_UI")) forceDesktop = std::atoi(env) != 0;
    u32 presetKey = 0, presetSfxKey = 0, presetMusicKey = 0, presetLevelKey = 0, presetPlayKey = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--selftest") {
            selftest = true;
        } else if (arg == "--touch-ui") {
            forceTouch = true;
        } else if (arg == "--desktop-ui") {
            forceDesktop = true;
        } else if (arg.rfind("--exec=", 0) == 0) {
            execCommands.push_back(arg.substr(7));
        } else if (arg.rfind("--screenshot=", 0) == 0) {
            screenshotPath = arg.substr(13);
        } else if (arg.rfind("--quit-after=", 0) == 0) {
            autoQuitSeconds = std::strtof(arg.c_str() + 13, nullptr);
        } else if (arg.rfind("--sprite=", 0) == 0) {
            presetSprite = true;
            presetKey = static_cast<u32>(std::strtoul(arg.c_str() + 9, nullptr, 0));
        } else if (arg.rfind("--sound=", 0) == 0) {
            presetSfx = true;
            presetSfxKey = static_cast<u32>(std::strtoul(arg.c_str() + 8, nullptr, 0));
        } else if (arg.rfind("--music=", 0) == 0) {
            presetMusic = true;
            presetMusicKey = static_cast<u32>(std::strtoul(arg.c_str() + 8, nullptr, 0));
        } else if (arg.rfind("--level=", 0) == 0) {
            presetLevel = true;
            presetLevelKey = static_cast<u32>(std::strtoul(arg.c_str() + 8, nullptr, 0));
        } else if (arg.rfind("--play=", 0) == 0) {
            presetPlay = true;
            presetPlayKey = static_cast<u32>(std::strtoul(arg.c_str() + 7, nullptr, 0));
        } else {
            state.assetDir = arg;
        }
    }

    registerCommands(state);

    if (selftest) {
        char line[160];
        int selftestChecks = 0;
        int selftestFailures = 0;
        const auto check = [&](const char* name, bool ok) {
            ++selftestChecks;
            if (!ok) ++selftestFailures;
            char checkLine[96];
            std::snprintf(checkLine, sizeof(checkLine), "  check %-24s %s", name,
                          ok ? "PASS" : "FAIL");
            state.console.log(checkLine);
        };
        check("flow menu catalogue",
              mmu::kFlowScreenCount == 20 && mmu::kFlowLevelClassCount == 39 &&
              mmu::kFlowLevelCueCount == 6);
        check("flow menu states",
              mmu::flowScreenHasState(mmu::FlowScreen::TitleMenu, mmu::FlowState::Wait) &&
              mmu::flowScreenHasState(mmu::FlowScreen::MenuStageSelect,
                                      mmu::FlowState::SelectLevel) &&
              mmu::flowScreenHasState(mmu::FlowScreen::ShopMenu,
                                      mmu::FlowState::TooManyItems));
        check("flow menu cues",
              mmu::flowScreenHasCue(mmu::FlowScreen::TitleMenu, mmu::FlowCue::TitleScreenA) &&
              mmu::flowScreenHasCue(mmu::FlowScreen::PauseMenuStatus,
                                    mmu::FlowCue::PauseMenu) &&
              mmu::flowScreenHasCue(mmu::FlowScreen::ShopMenu, mmu::FlowCue::Purchase));
        check("flow level cues",
              std::string(mmu::flowLevelCueName(mmu::FlowLevelCue::Victory)) == "Victory" &&
              std::string(mmu::flowLevelCueName(mmu::FlowLevelCue::BigExplosion)) ==
                  "BigExplosion");
        check("boss damage key",
              mmu::kBossDamageParamKey == 0xa96bff8du &&
                  mmu::kBossDamageGetterVa == 0x43c800u &&
                  mmu::kBossDamageCompareVa == 0x697ce8u &&
                  mmu::kBossDamageParamDefault == 1.0f &&
                  mmu::kBossDamageDepletionCode == 4 &&
                  mmu::kBossDamageMeterOffset0 == 0x150u &&
                  mmu::kBossDamageMeterOffset1 == 0x160u &&
                  mmu::kBossDamageHelperVa0 == 0x420380u &&
                  mmu::kBossDamageHelperVa1 == 0x420690u &&
                  mmu::bossDamageDepletionMatches(1.0f) &&
                  !mmu::bossDamageDepletionMatches(0.5f) &&
                  std::size(mmu::kBossDamageCallers) == 15 &&
                  mmu::kBossDamageCallers[1].owner == "LuggageMan" &&
                  mmu::kBossDamageCallers[1].callVa == 0x580407u &&
                  mmu::kBossDamageCallers[1].vftableVa == 0x691cecu &&
                  mmu::kBossDamageCallers[10].owner == "MineE" &&
                  mmu::kBossDamageCallers[10].callVa == 0x58f4a7u &&
                  mmu::kBossDamageCallers[10].resultCodeBits == 0x0du);
        check("wily attack selector",
              mmu::kWilyPhaseField == 0x368u &&
                  mmu::kWilyShotCounterField == 0x374u &&
                  mmu::kWilyHoverTimerField == 0x364u &&
                  mmu::kWilySelectorVa == 0x56d580u &&
                  mmu::kWilySelectorCallerVa == 0x56da4au &&
                  mmu::kWilyEventCode0f == 0xfu &&
                  mmu::kWilyPhaseCount == 4 &&
                  mmu::kWilyWrapPhase == 3 &&
                  mmu::kWilyShotCounterLimit == 3 &&
                  std::size(mmu::kWilySelectorPhases) == 4 &&
                  mmu::kWilySelectorPhases[0].stateName == "GotoAttack1" &&
                  !mmu::kWilySelectorPhases[0].clearsShotCounter &&
                  mmu::kWilySelectorPhases[1].clearsShotCounter &&
                  mmu::kWilySelectorPhases[2].clearsShotCounter &&
                  !mmu::kWilySelectorPhases[3].clearsShotCounter &&
                  mmu::wilySelectorPhase(4) == 0 &&
                  mmu::wilySelectorPhase(-1) == 3 &&
                  mmu::wilySelectorPhaseState(2) == "GotoAttack3");
        const mmu::EnemyStateGraphMachine* sniperMachine =
            mmu::enemyStateGraphMachine("SniperJoe");
        mmu::EnemyController graphEnemy;
        graphEnemy.reset(mmu::EnemyKind::SniperJoe, 0.0f, 0.0f);
        const mmu::EnemyState sniperInitial = graphEnemy.state;
        graphEnemy.reset(mmu::EnemyKind::ChickenBomb, 0.0f, 0.0f);
        const mmu::EnemyState chickenInitial = graphEnemy.state;
        graphEnemy.reset(mmu::EnemyKind::MineP, 0.0f, 0.0f);
        const mmu::EnemyState minePInitial = graphEnemy.state;
        graphEnemy.reset(mmu::EnemyKind::GrenadeSoldier, 0.0f, 0.0f);
        const mmu::EnemyState grenadeInitial = graphEnemy.state;
        check("enemy state machines",
              std::size(mmu::kEnemyStateGraphMachines) == 13 &&
                  sniperMachine != nullptr &&
                  sniperMachine->vftableVa == 0x691304u &&
                  sniperMachine->constructorVa == 0x5896a0u &&
                  sniperMachine->damageCallVa == 0x5895e1u &&
                  sniperMachine->damageResultCodeBits == 0x0du &&
                  sniperMachine->initialState == "Stand" &&
                  sniperMachine->initialSetStateVa == 0x589abfu &&
                  sniperMachine->stateCount == 3 &&
                  sniperMachine->edgeCount == 5 &&
                  mmu::enemyStateGraphInitialState("MineP") == "Scan" &&
                  mmu::enemyStateGraphInitialState("Skoripion") == "Walk" &&
                  sniperInitial == mmu::EnemyState::Stand &&
                  chickenInitial == mmu::EnemyState::Patrol &&
                  minePInitial == mmu::EnemyState::Scan &&
                  grenadeInitial == mmu::EnemyState::Hidden &&
                  mmu::inferEnemyKind("BulletTrap") == mmu::EnemyKind::BulletTrap &&
                  mmu::inferEnemyKind("LaserSpreader") ==
                      mmu::EnemyKind::LaserSpreader &&
                  mmu::inferEnemyKind("SniperJoeShot") == mmu::EnemyKind::None &&
                  mmu::enemyStateGraphInitialState("MetTrois") == "Hidden" &&
                  mmu::enemyStateGraphInitialState("SpreadMine") == "Idle" &&
                  mmu::inferEnemyKind("MetTrois") == mmu::EnemyKind::MetTrois &&
                  mmu::inferEnemyKind("SpreadMine") ==
                      mmu::EnemyKind::SpreadMine);
        std::size_t graphSlotCount = 0;
        for (const mmu::EnemyStateGraphMachine& machine :
             mmu::kEnemyStateGraphMachines)
            graphSlotCount += machine.stateSlotCount;
        const mmu::EnemyStateGraphMachine* minePMachine =
            mmu::enemyStateGraphMachine("MineP");
        const mmu::EnemyStateGraphMachine* metTroisMachine =
            mmu::enemyStateGraphMachine("MetTrois");
        const mmu::EnemyStateGraphMachine* spreadMineMachine =
            mmu::enemyStateGraphMachine("SpreadMine");
        check("enemy state slots",
              graphSlotCount == 28 &&
                  sniperMachine->stateSlotCount == 3 &&
                  sniperMachine->stateSlots[0].stateName == "Stand" &&
                  sniperMachine->stateSlots[0].firstSlot == 29 &&
                  sniperMachine->stateSlots[0].lastSlot == 31 &&
                  sniperMachine->stateSlots[1].stateName == "Fire" &&
                  sniperMachine->stateSlots[2].stateName == "Jump" &&
                  sniperMachine->edges[0].ownerState == "Fire" &&
                  sniperMachine->edges[4].ownerState == "Stand" &&
                  minePMachine != nullptr &&
                  minePMachine->stateSlots[1].stateName == "Burst" &&
                  minePMachine->stateSlots[1].firstSlot == 29 &&
                  minePMachine->stateSlots[2].stateName == "Flash" &&
                  minePMachine->stateSlots[2].firstSlot == 26 &&
                  mmu::enemyStateGraphSlotOwner("SniperJoe", 24) == "Fire" &&
                  mmu::enemyStateGraphSlotOwner("SniperJoe", 27) == "Jump" &&
                  mmu::enemyStateGraphSlotOwner("SniperJoe", 30) == "Stand" &&
                  mmu::enemyStateGraphSlotOwner("SniperJoe", 3).empty() &&
                  mmu::enemyStateGraphSlotOwner("MineP", 29) == "Burst" &&
                  mmu::enemyStateGraphSlotOwner("MineP", 26) == "Flash" &&
                  metTroisMachine != nullptr &&
                  metTroisMachine->stateCount == 6 &&
                  metTroisMachine->edgeCount == 6 &&
                  metTroisMachine->vftableVa == 0x69115cu &&
                  metTroisMachine->damageVtableSlot == 3 &&
                  metTroisMachine->damageResolution ==
                      "unique-vftable-member" &&
                  spreadMineMachine != nullptr &&
                  spreadMineMachine->stateCount == 2 &&
                  spreadMineMachine->vftableVa == 0x68ef6cu &&
                  spreadMineMachine->damageCallVa == 0x5da267u &&
                  spreadMineMachine->damageResolution ==
                      "unique-vftable-member" &&
                  mmu::enemyStateGraphSlotOwner("MetTrois", 27) == "Popping" &&
                  mmu::enemyStateGraphSlotOwner("SpreadMine", 23) == "Explode");

        std::size_t edgeFactCount = 0;
        std::size_t edgeComparisonCount = 0;
        std::size_t edgeConstantCount = 0;
        for (const mmu::EnemyEdgeFact& fact : mmu::kEnemyEdgeFacts) {
            ++edgeFactCount;
            edgeComparisonCount += fact.comparisonCount;
            edgeConstantCount += fact.constantCount;
        }
        const mmu::EnemyEdgeFact* gluedFact =
            mmu::enemyEdgeFact("LuggageMan", 3, 0x5a6f4bu);
        const mmu::EnemyEdgeFact* skoripionShotFact =
            mmu::enemyEdgeFact("Skoripion", 24, 0x583144u);
        const mmu::EnemyEdgeFact* metHidingFact =
            mmu::enemyEdgeFact("MetTrois", 33, 0x58b49eu);
        const mmu::EnemyEdgeFact* spreadExplodeFact =
            mmu::enemyEdgeFact("SpreadMine", 3, 0x5da294u);
        check("enemy edge facts",
              edgeFactCount == 35 && edgeComparisonCount == 126 &&
                  edgeConstantCount == 15 && gluedFact != nullptr &&
                  gluedFact->setStateKeyVa == 0x67d508u &&
                  gluedFact->setStateKey == "Glued" &&
                  gluedFact->comparisonCount == 3 &&
                  gluedFact->comparisons[0].instructionVa == 0x5a6effu &&
                  gluedFact->comparisons[0].operands ==
                      "dword ptr [ebp - 8], 0x11" &&
                  skoripionShotFact != nullptr &&
                  skoripionShotFact->isCue &&
                  skoripionShotFact->targetName == "SkoripionShot" &&
                  skoripionShotFact->setStateKeyVa == 0x67ae1cu &&
                  skoripionShotFact->comparisonCount == 6 &&
                  skoripionShotFact->constantCount == 1 &&
                  skoripionShotFact->constants[0].address == 0x697ce0u &&
                  skoripionShotFact->constants[0].value == 0.0 &&
                  metHidingFact != nullptr &&
                  metHidingFact->setStateKey == "Hiding" &&
                  metHidingFact->constantCount == 1 &&
                  metHidingFact->constants[0].address == 0x698a20u &&
                  metHidingFact->constants[0].value == 0.800000011920929 &&
                  spreadExplodeFact != nullptr &&
                  spreadExplodeFact->setStateKeyVa == 0x681ea4u &&
                  spreadExplodeFact->setStateKey == "Explode" &&
                  mmu::enemyEdgeFact("Missing", 0, 0) == nullptr);

        std::size_t flowEdgeFactCount = 0;
        std::size_t flowComparisonCount = 0;
        std::size_t flowConstantCount = 0;
        for (const mmu::FlowEdgeFact& fact : mmu::kFlowEdgeFacts) {
            ++flowEdgeFactCount;
            flowComparisonCount += fact.comparisonCount;
            flowConstantCount += fact.constantCount;
        }
        const mmu::FlowEdgeFact* busterSelectFact =
            mmu::flowEdgeFact("BusterMenu", 2, 0x618e28u);
        const mmu::FlowEdgeFact* titleExitFact =
            mmu::flowEdgeFact("TitleMenu", 8, 0x4f3032u);
        const mmu::FlowEdgeFact* mainConfirmFact =
            mmu::flowEdgeFact("MainMenu", 5, 0x5036eeu);
        const mmu::FlowEdgeFact* stageSelectFact =
            mmu::flowEdgeFact("MenuStageSelect", 2, 0x5000f4u);
        check("flow edge facts",
              flowEdgeFactCount == 181 && flowComparisonCount == 1390 &&
                  flowConstantCount == 144 && busterSelectFact != nullptr &&
                  busterSelectFact->isCue &&
                  busterSelectFact->setStateKeyVa == 0x68da0cu &&
                  busterSelectFact->setStateKey == "Select" &&
                  busterSelectFact->comparisonCount == 5 &&
                  busterSelectFact->constantCount == 2 &&
                  busterSelectFact->constants[0].address == 0x6987b8u &&
                  busterSelectFact->constants[0].value == -1.0 &&
                  busterSelectFact->constants[1].address == 0x697ce0u &&
                  busterSelectFact->constants[1].value == 0.0 &&
                  titleExitFact != nullptr &&
                  titleExitFact->targetName == "IntroFadeOut" &&
                  titleExitFact->setStateKeyVa == 0x65daa4u &&
                  mainConfirmFact != nullptr &&
                  mainConfirmFact->setStateKeyVa == 0x65fdacu &&
                  mainConfirmFact->setStateKey == "Confirm" &&
                  mainConfirmFact->comparisonCount == 11 &&
                  stageSelectFact != nullptr &&
                  stageSelectFact->targetName == "SelectLevel" &&
                  stageSelectFact->constants[0].address == 0x697ce0u &&
                  stageSelectFact->constants[1].address == 0x698aa0u &&
                  stageSelectFact->constants[1].value == 400.0 &&
                  mmu::flowEdgeFact("Missing", 0, 0) == nullptr);

        std::size_t pauseIconCount = 0, pauseColorCount = 0;
        std::size_t rushColorKeyCount = 0;
        u32 rushColorFirstSequence = 0xffffu, rushColorSecondSequence = 0xffffu;
        for (const mmu::PauseWeaponResource& resource : mmu::kPauseWeaponResources) {
            if (resource.kind == "Icon") ++pauseIconCount;
            else if (resource.kind == "Colors") ++pauseColorCount;
            if (resource.key == 0x7c728d9fu) {
                if (rushColorFirstSequence == 0xffffu)
                    rushColorFirstSequence = resource.sequence;
                else
                    rushColorSecondSequence = resource.sequence;
                ++rushColorKeyCount;
            }
        }
        const mmu::PauseWeaponResource* megaBusterResource =
            mmu::pauseWeaponResource(0x73a4ca16u);
        const mmu::PauseWeaponResource* waterColorsResource =
            mmu::pauseWeaponResource("..\\Menus\\Pause\\WaterCannonColors.xspr");
        check("pause inline resources",
              std::size(mmu::kPauseWeaponResources) == 44 &&
                  pauseIconCount == 31 && pauseColorCount == 13 &&
                  megaBusterResource != nullptr &&
                  megaBusterResource->sequence == 13 &&
                  megaBusterResource->pushVa == 0x4f9d16u &&
                  megaBusterResource->stringVa == 0x65e564u &&
                  megaBusterResource->path ==
                      "..\\Menus\\Pause\\MegaBusterIcon.xspr" &&
                  waterColorsResource != nullptr &&
                  waterColorsResource->sequence == 37 &&
                  waterColorsResource->key == 0x424b7cbcu &&
                  rushColorKeyCount == 2 && rushColorFirstSequence == 35 &&
                  rushColorSecondSequence == 36 &&
                  mmu::pauseWeaponResource(0u) == nullptr);

        std::vector<std::string_view> inlineResourceClasses;
        std::vector<std::string_view> inlineResourcePaths;
        std::size_t pauseInlineCount = 0, loadInlineCount = 0,
                    stageSelectInlineCount = 0;
        for (const mmu::InlineMenuResource& resource : mmu::kInlineMenuResources) {
            inlineResourceClasses.push_back(resource.className);
            inlineResourcePaths.push_back(resource.path);
            if (resource.className == "PauseMenuStatus") ++pauseInlineCount;
            else if (resource.className == "LoadMenu") ++loadInlineCount;
            else if (resource.className == "MenuStageSelect")
                ++stageSelectInlineCount;
        }
        std::sort(inlineResourceClasses.begin(), inlineResourceClasses.end());
        inlineResourceClasses.erase(
            std::unique(inlineResourceClasses.begin(), inlineResourceClasses.end()),
            inlineResourceClasses.end());
        std::sort(inlineResourcePaths.begin(), inlineResourcePaths.end());
        inlineResourcePaths.erase(
            std::unique(inlineResourcePaths.begin(), inlineResourcePaths.end()),
            inlineResourcePaths.end());
        const mmu::InlineMenuResource* pauseZPunchResource =
            mmu::firstInlineMenuResource("PauseMenuStatus", 3u);
        const mmu::InlineMenuResource* stageSelectZeroResource =
            mmu::firstInlineMenuResource(0x991fbc7au);
        const mmu::InlineMenuResource* menuCursorResource =
            mmu::firstInlineMenuResource("..\\Menus\\MenuCursor.xspr");
        check("inline menu resources",
              std::size(mmu::kInlineMenuResources) == 231 &&
                  inlineResourceClasses.size() == 20 &&
                  inlineResourcePaths.size() == 153 &&
                  pauseInlineCount == 89 && loadInlineCount == 47 &&
                  stageSelectInlineCount == 18 &&
                  pauseZPunchResource != nullptr &&
                  pauseZPunchResource->pushVa == 0x4f988au &&
                  pauseZPunchResource->stringVa == 0x65e1ecu &&
                  pauseZPunchResource->key == 0xb3928de7u &&
                  pauseZPunchResource->path ==
                      "..\\Menus\\Pause\\ZPunchIcon.xspr" &&
                  stageSelectZeroResource != nullptr &&
                  stageSelectZeroResource->className == "MenuStageSelect" &&
                  stageSelectZeroResource->sequence == 0 &&
                  stageSelectZeroResource->pushVa == 0x5004eau &&
                  menuCursorResource != nullptr &&
                  menuCursorResource->className == "BusterMenu" &&
                  menuCursorResource->sequence == 1 &&
                  menuCursorResource->key == 0x39bf406bu &&
                  mmu::firstInlineMenuResource("Missing", 0u) == nullptr);

        std::size_t completeSlotBlocks = 0;
        std::vector<std::string_view> slotBlockClasses;
        for (const mmu::StateSlotBlock& block : mmu::kStateSlotBlocks) {
            if (block.complete) ++completeSlotBlocks;
            slotBlockClasses.push_back(block.className);
        }
        std::sort(slotBlockClasses.begin(), slotBlockClasses.end());
        slotBlockClasses.erase(
            std::unique(slotBlockClasses.begin(), slotBlockClasses.end()),
            slotBlockClasses.end());
        const mmu::StateSlotBlock* metHiddenBlock =
            mmu::stateSlotBlock("MetTrois", "Hidden");
        const mmu::StateSlotBlock* busterSelectBlock =
            mmu::stateSlotBlock("BusterMenu", "Select");
        check("state slot blocks",
              std::size(mmu::kStateSlotBlocks) == 671 &&
                  slotBlockClasses.size() == 237 &&
                  completeSlotBlocks == 671 &&
                  std::size(mmu::kStateSlotBlocks) - completeSlotBlocks == 0 &&
                  metHiddenBlock != nullptr && metHiddenBlock->complete &&
                  metHiddenBlock->slotCount == 3 &&
                  metHiddenBlock->slots[0] == 25 &&
                  metHiddenBlock->slots[1] == 24 &&
                  metHiddenBlock->slots[2] == 23 &&
                  metHiddenBlock->thunks[0] == 0x4242f0u &&
                  metHiddenBlock->registrationVa == 0x58b9d5u &&
                  busterSelectBlock != nullptr &&
                  busterSelectBlock->complete &&
                  busterSelectBlock->slotCount == 3 &&
                  busterSelectBlock->slots[0] == 3 &&
                  busterSelectBlock->slots[1] == 2 &&
                  busterSelectBlock->slots[2] == 1 &&
                  busterSelectBlock->thunks[0] == 0x403290u &&
                  busterSelectBlock->thunks[1] == 0x431930u &&
                  busterSelectBlock->thunks[2] == 0x433070u &&
                  mmu::firstStateSlotBlock("Missing") == nullptr);

        const mmu::OutroHandoffCall* outroCutsceneCall =
            mmu::findOutroHandoffCall(0x526410u);
        check("outro handoff table",
              std::size(mmu::kOutroHandoffCalls) == 19 &&
                  std::size(mmu::kOutroHandoffStrings) == 16 &&
                  std::size(mmu::kOutroHandoffFacts) == 43 &&
                  mmu::kOutroHandoffNoOffset == 0xffffffffu &&
                  outroCutsceneCall != nullptr &&
                  outroCutsceneCall->targetVa == 0x621890u &&
                  outroCutsceneCall->stringVa == 0x66f044u &&
                  outroCutsceneCall->hash == 0x60bb8d06u &&
                  outroCutsceneCall->owner == "LevelWilyFive" &&
                  outroCutsceneCall->literal ==
                      "..\\Cutscenes\\CSNZero\\csnWily5Outro.xcut" &&
                  mmu::hasOutroHandoffOwner(0x5b02f0u) &&
                  mmu::hasOutroHandoffFactOffset(0x18cu) &&
                  !mmu::hasOutroHandoffFactOffset(0xffffffffu) &&
                  mmu::findOutroHandoffCall(0u) == nullptr);

        std::size_t unresolvedBossDamageCallers = 0;
        for (const mmu::BossDamageCaller& row : mmu::kBossDamageCallers)
            if (row.owner.empty()) ++unresolvedBossDamageCallers;
        bool foundMetTroisDamage = false, foundSpreadMineDamage = false;
        for (const mmu::BossDamageCaller& row : mmu::kBossDamageCallers) {
            if (row.owner == "MetTrois" && row.callVa == 0x58b737u &&
                row.vftableVa == 0x69115cu && row.vtableSlot == 3 &&
                row.resultCodeBits == 0x9u &&
                row.resolution == "unique-vftable-member")
                foundMetTroisDamage = true;
            if (row.owner == "SpreadMine" && row.callVa == 0x5da267u &&
                row.vftableVa == 0x68ef6cu && row.vtableSlot == 3 &&
                row.resultCodeBits == 0xdu &&
                row.resolution == "unique-vftable-member")
                foundSpreadMineDamage = true;
        }
        check("boss damage caller map",
              std::size(mmu::kBossDamageCallers) == 15 &&
                  unresolvedBossDamageCallers == 2 &&
                  mmu::kBossDamageParamKey == 0xa96bff8du &&
                  mmu::kBossDamageGetterVa == 0x43c800u &&
                  mmu::kBossDamageCompareVa == 0x697ce8u &&
                  mmu::kBossDamageParamDefault == 1.0f &&
                  mmu::kBossDamageDepletionCode == 4 &&
                  mmu::kBossDamageMeterOffset0 == 0x150u &&
                  mmu::kBossDamageMeterOffset1 == 0x160u &&
                  mmu::kBossDamageHelperVa0 == 0x420380u &&
                  mmu::kBossDamageHelperVa1 == 0x420690u &&
                  mmu::bossDamageDepletionMatches(1.0f) &&
                  !mmu::bossDamageDepletionMatches(0.0f) &&
                  foundMetTroisDamage && foundSpreadMineDamage);

        int resolvedDamageWrapperCallers = 0;
        bool foundSweeperWrapper = false, foundBulletWrapper = false;
        for (const mmu::DamageWrapperCaller& row :
             mmu::kDamageWrapperCallers) {
            if (!row.owner.empty()) ++resolvedDamageWrapperCallers;
            if (row.owner == "Sweeper" && row.callVa == 0x5858e5u &&
                row.wrapperVa == 0x420450u &&
                row.handlerVa == 0x5858d0u &&
                row.vftableVa == 0x691664u && row.vtableSlot == 3u &&
                row.resolution == "unique-vftable-member")
                foundSweeperWrapper = true;
            if (row.owner == "Bullet" && row.callVa == 0x56438fu &&
                row.wrapperVa == 0x420570u &&
                row.handlerVa == 0x564320u &&
                row.vftableVa == 0x693414u &&
                row.resolution == "vftable-store")
                foundBulletWrapper = true;
        }
        check("damage wrapper caller map",
              mmu::kDamageWrapperCallerCount == 4 &&
                  resolvedDamageWrapperCallers == 3 &&
                  mmu::kDamageWrapperVa0 == 0x420450u &&
                  mmu::kDamageWrapperHelperVa0 == 0x420380u &&
                  mmu::kDamageWrapperMeter0 == 0x150u &&
                  mmu::kDamageWrapperVa1 == 0x420570u &&
                  mmu::kDamageWrapperHelperVa1 == 0x420690u &&
                  mmu::kDamageWrapperMeter1 == 0x160u &&
                  foundSweeperWrapper && foundBulletWrapper &&
                  mmu::damageWrapperCaller("Sweeper") != nullptr &&
                  mmu::damageWrapperCaller("MissingOwner") == nullptr);

        bool foundCameraUpdateSlot = false, foundCameraTargetIdle = false;
        bool foundCameraTargetMove = false, foundCameraGluedEdge = false;
        for (const mmu::CameraVtableSlot& row : mmu::kCameraVtableSlots)
            foundCameraUpdateSlot |= row.slot == 15u && row.functionVa == 0x444d90u;
        for (const mmu::CameraTargetState& row : mmu::kCameraTargetStates) {
            foundCameraTargetIdle |= row.name == "Idle" &&
                                     row.registrationVa == 0x578befu &&
                                     row.hash == 0x49646c65u && row.complete;
            foundCameraTargetMove |= row.name == "Move" &&
                                     row.registrationVa == 0x578cd9u &&
                                     row.hash == 0x4d6f7665u && row.complete;
        }
        for (const mmu::CameraTargetEdge& row : mmu::kCameraTargetEdges)
            foundCameraGluedEdge |= row.slot == 3u && row.functionVa == 0x5a6ef0u &&
                                    row.callVa == 0x5a6f4bu &&
                                    row.hash == 0x46b44516u && row.stateName == "Glued";
        const mmu::CameraOperandLine* cameraOperand =
            mmu::cameraOperandLine(0x444d90u, 0);
        check("camera raw evidence table",
              mmu::kCameraVtableVa == 0x69775cu &&
                  mmu::kCameraVtableSlotCount == 18 &&
                  mmu::kCameraTargetVtableVa == 0x6924ecu &&
                  mmu::kCameraTargetConstructorVa == 0x578aa0u &&
                  mmu::kCameraTargetVtableSlotCount == 29 &&
                  mmu::kCameraTargetStateCount == 2 &&
                  mmu::kCameraTargetEdgeCount == 2 &&
                  mmu::kCameraOperandLineCount == 49 &&
                  mmu::kCameraDat006987d0 == 0x006987d0u &&
                  mmu::kCameraDat00698458 == 0x00698458u &&
                  foundCameraUpdateSlot && foundCameraTargetIdle &&
                  foundCameraTargetMove && foundCameraGluedEdge &&
                  cameraOperand != nullptr && cameraOperand->sourceLine == 20);

        const mmu::PlayerSpawnTypeCount* exactMegaManSpawn =
            mmu::playerSpawnExactCheck("MegaMan");
        const mmu::PlayerSpawnTypeCount* exactShadowSpawn =
            mmu::playerSpawnExactCheck("ShadowMegaMan");
        const mmu::PlayerSpawnCandidate* shadowCandidate =
            mmu::playerSpawnCandidate(247u, 5u);
        check("player spawn token evidence",
              mmu::kPlayerSpawnSceneCount == 791 &&
                  mmu::kPlayerSpawnRecordCount == 4926 &&
                  mmu::kPlayerSpawnUniqueTypeCount == 82 &&
                  mmu::kPlayerSpawnSubstringMegaManCount == 77 &&
                  mmu::kPlayerSpawnSubstringPlayerCount == 0 &&
                  mmu::kPlayerSpawnCandidateCount == 77 &&
                  exactMegaManSpawn != nullptr && exactMegaManSpawn->count == 0 &&
                  exactShadowSpawn != nullptr && exactShadowSpawn->count == 77 &&
                  shadowCandidate != nullptr &&
                  shadowCandidate->sceneKey == 0xf8bdc84bu &&
                  shadowCandidate->x == 465.0f &&
                  shadowCandidate->y == 192.0f &&
                  shadowCandidate->flags == 15u &&
                  shadowCandidate->type == "ShadowMegaMan");

        bool foundSlideState = false, foundJumpState = false;
        bool foundShootState = false, foundChargeState = false;
        bool foundChargeKey = false, foundMegaManSlide = false;
        bool foundProtoManCharge = false, foundBassBuster = false;
        bool foundMegaManCharacter = false, foundPrototypeCharacter = false;
        bool foundChargeField = false, foundDoubleJumpResource = false;
        for (const mmu::PlayerControlState& row : mmu::kPlayerControlStates) {
            foundSlideState |= row.name == "Slide" && row.hash == 0x19bde9bbu;
            foundJumpState |= row.name == "Jump" && row.hash == 0x4a756d70u;
            foundShootState |= row.name == "Shoot" && row.hash == 0x1dbbe2aau;
            foundChargeState |= row.name == "Charge" && row.hash == 0xd8727fd5u;
        }
        for (const mmu::PlayerControlKey& row : mmu::kPlayerControlKeys)
            foundChargeKey |= row.name == "ChargeTime" && row.hash == 0xb40a8909u;
        for (const mmu::PlayerControlEdge& row : mmu::kPlayerControlEdges) {
            foundMegaManSlide |= row.className == "MegaMan" &&
                                 row.target == "Slide" && row.slot == 37u &&
                                 row.functionVa == 0x5a8df0u &&
                                 row.callVa == 0x5a8e30u &&
                                 row.hash == 0x19bde9bbu;
            foundProtoManCharge |= row.className == "ProtoMan" &&
                                   row.target == "Charge" && row.slot == 32u &&
                                   row.callVa == 0x571fa8u &&
                                   row.hash == 0xd8727fd5u;
            foundBassBuster |= row.className == "Bass" &&
                               row.target == "Buster" && row.isCue &&
                               row.callVa == 0x570376u &&
                               row.hash == 0x65b1e1bdu;
        }
        for (const mmu::PlayerControlCharacter& row : mmu::kPlayerControlCharacters) {
            foundMegaManCharacter |= row.action == 0x48u && row.character == 0u &&
                                     row.label == "MEGA MAN" &&
                                     row.labelVa == 0x68d794u;
            foundPrototypeCharacter |= row.action == 0x49u &&
                                       row.character == 1u &&
                                       row.label == "Z-PROTOTYPE" &&
                                       row.labelVa == 0x68d7a0u;
        }
        foundChargeField = mmu::kPlayerControlChargeShotField.find(
                               "use_charge_shot") != std::string_view::npos;
        for (const mmu::PlayerControlResource& row : mmu::kPlayerControlResources)
            foundDoubleJumpResource |= row.key == 0xeea22b3cu &&
                                       row.path.find("ZDoubleJumpIcon.xspr") !=
                                           std::string_view::npos;
        check("player control raw evidence table",
              mmu::kPlayerControlStateCount == 16 &&
                  mmu::kPlayerControlKeyCount == 17 &&
                  mmu::kPlayerControlEdgeCount == 33 &&
                  mmu::kPlayerControlCharacterCount == 2 &&
                  mmu::kPlayerControlResourceCount == 4 &&
                  mmu::kPlayerControlOperandLineCount == 82 &&
                  foundSlideState && foundJumpState && foundShootState &&
                  foundChargeState && foundChargeKey && foundMegaManSlide &&
                  foundProtoManCharge && foundBassBuster &&
                  foundMegaManCharacter && foundPrototypeCharacter &&
                  foundChargeField && foundDoubleJumpResource &&
                  mmu::playerControlState("Slide") != nullptr &&
                  mmu::playerControlKey("ShootDelay")->hash == 0xe1e66eb0u &&
                  mmu::playerControlOperandLine(0x5b02f0u, 0) != nullptr);

        check("stage select grid table",
              std::size(mmu::kStageSelectOuterCells) == 8 &&
                  std::size(mmu::kStageSelectCursorKeys) == 4 &&
                  mmu::kStageSelectCenterFlagStage == 9 &&
                  mmu::kStageSelectCenterCountMin == 1 &&
                  mmu::kStageSelectCenterCountMax == 5 &&
                  mmu::stageSelectGridOuterStage(0, 0) == 8 &&
                  mmu::stageSelectGridOuterStage(2, 2) == 5 &&
                  mmu::stageSelectGridOuterStage(1, 1) == -1 &&
                  mmu::stageSelectGridCenterStage(true, 0) == 9 &&
                  mmu::stageSelectGridCenterStage(false, 5) == 14 &&
                  mmu::stageSelectGridCenterStage(false, 0) == -1 &&
                  mmu::stageSelectGridCenterStage(false, 6) == -1 &&
                  mmu::stageSelectGridStage(1, 0, false, 0) == 1 &&
                  mmu::kStageSelectCursorKeys[0].key == 0x1cu &&
                  mmu::kStageSelectCursorKeys[0].fieldOffset == 0xba4u &&
                  mmu::kStageSelectCursorKeys[2].key == 0x19u &&
                  mmu::kStageSelectCursorKeys[2].fieldOffset == 0xba0u);

        const mmu::RecoveredCompletionCutscene* completionFlag9 =
            mmu::findCompletionCutsceneFlag(9u);
        const mmu::RecoveredCompletionCutscene* completionState100 =
            mmu::findCompletionCutsceneState(0x100u, false);
        check("completion cutscene table",
              mmu::kRecoveredCompletionCutsceneCount == 12 &&
                  completionFlag9 != nullptr &&
                  completionFlag9->stringVa == 0x6608c0u &&
                  completionFlag9->path ==
                      "..\\Cutscenes\\CSN03EightBosses\\TCNGETSpeedster.xcut" &&
                  completionState100 != nullptr &&
                  completionState100->selector == 1u &&
                  completionState100->pendingStringVa == 0x6c6964u &&
                  completionState100->writeCount == 1u &&
                  completionState100->writeOffsets[0] == 0x08u &&
                  completionState100->writeValues[0] == 0x0100u &&
                  mmu::kCompletionPendingCallVa == 0x4308c0u &&
                  mmu::kCompletionPendingStringOffset == 0x40u &&
                  mmu::kCompletionPendingFlagOffset == 0x60u &&
                  mmu::kCompletionPendingFlagValue == 0x01u &&
                  mmu::kCompletionClearCallVa == 0x4308a0u &&
                  mmu::kCompletionClearGlobalVa == 0x6c55f8u &&
                  mmu::kCompletionClearWriteCount == 2 &&
                  mmu::kCompletionClearWrites[0].offset == 0x178u &&
                  mmu::kCompletionClearWrites[1].offset == 0x17cu &&
                  mmu::findCompletionCutsceneFlag(999u) == nullptr &&
                  mmu::findCompletionCutsceneState(0x300u, true) == nullptr);

        check("verified key names",
              mmu::kVerifiedKeyNameCount == 137 &&
                  mmu::verifiedKeyName(0x0166019au) == "LiftSpeed" &&
                  mmu::verifiedKeyName(0xfc3e6048u) == "LightningDuration" &&
                  mmu::hasVerifiedKeyName(0xfc3e6048u) &&
                  !mmu::hasVerifiedKeyName(0xa96bff8du) &&
                  !mmu::hasVerifiedKeyName(0x081f4e28u) &&
                  mmu::verifiedKeyName(0u).empty());

        check("decoded gun parameter objects",
              mmu::kBoss.key == 0x2e038bd9u && mmu::kBoss.paramCount == 35 &&
                  mmu::gunParam(mmu::kBoss, mmu::kHitPointsKey, -1.0f) == 6.0f &&
                  mmu::gunParam(mmu::kBoss, mmu::kTargetHotSpotYKey, -1.0f) == -34.0f &&
                  mmu::kMineE.key == 0x37967481u && mmu::kMineE.paramCount == 50 &&
                  mmu::kSawPole.key == 0x650b698bu && mmu::kSawPole.paramCount == 39 &&
                  mmu::kSuperCannon.key == 0x50ac415du &&
                  mmu::kSuperCannon.paramCount == 38 &&
                  mmu::kTelly.key == 0xaabc064du && mmu::kTelly.paramCount == 34 &&
                  mmu::kWilyCapsule.key == 0x22d4960bu &&
                  mmu::kWilyCapsule.paramCount == 3 &&
                  mmu::kMoveSpeedKey == 0x57469074u &&
                  mmu::gunParam(mmu::kWilyCapsule, mmu::kMoveSpeedKey, -1.0f) == 60.0f &&
                  mmu::gunParam(mmu::kBoss, mmu::kMoveSpeedKey, -1.0f) == -1.0f);

        const mmu::RttiVtable* metTroisVtable = mmu::rttiVtable("MetTrois");
        check("rtti vtable table",
              mmu::kRttiTypeDescriptorCount == 348 &&
                  mmu::kRttiCompleteObjectLocatorCount == 347 &&
                  mmu::kRttiVtableCount == 347 &&
                  mmu::kRttiVtableSlotCount == 9127 &&
                  metTroisVtable != nullptr &&
                  metTroisVtable->mangledName == ".?AVMetTrois@@" &&
                  metTroisVtable->vftableVa == 0x69115cu &&
                  metTroisVtable->slotOffset == 1126u &&
                  metTroisVtable->slotCount == 41 &&
                  mmu::rttiVtableSlot(*metTroisVtable, 3) == 0x58b710u &&
                  mmu::rttiVtableSlot(*metTroisVtable, 25) == 0x58b1f0u &&
                  mmu::rttiVtableSlot(*metTroisVtable, 41) == 0u &&
                  mmu::rttiVtableByVa(0x69115cu) == metTroisVtable &&
                  mmu::rttiVtable("Missing") == nullptr);

        check("unverified key name candidates",
              mmu::kUnverifiedKeyNameCandidateCount == 5 &&
                  mmu::unverifiedKeyNameCandidate(0x081f4e28u) == "RatioCountframe" &&
                  mmu::unverifiedKeyNameCandidate(0xccf2a219u) ==
                      "ExplosiondistanceLaserresurrect" &&
                  mmu::unverifiedKeyNameCandidate(0xa96bff8du).empty() &&
                  mmu::verifiedKeyName(0x081f4e28u).empty());

        check("save flag raw table",
              mmu::kSaveFlagManagerVa == 0x6c55fcu &&
                  mmu::kKeyedFlagKeyboardLimit == 0x143u &&
                  mmu::kKeyedFlagManagerByteBias == 0x0f9u &&
                  mmu::kSaveFlagWriterCount == 138 &&
                  mmu::kSaveFlagMask8KeyCount == 33 &&
                  mmu::kSaveFlagMask4KeyCount == 9 &&
                  mmu::kDirectSaveFlagReaderCount == 3 &&
                  mmu::kSaveFlagWriters[0].sourceVa == 0x4fbb1bu &&
                  mmu::kSaveFlagWriters[0].category == 1u &&
                  mmu::kSaveFlagWriters[0].key == 21u &&
                  mmu::kSaveFlagWriters[0].aux == 275u &&
                  mmu::kSaveFlagWriters[0].maxRank == 323u &&
                  mmu::kDirectSaveFlagReaders[0].sourceVa == 0x4457abu &&
                  mmu::kDirectSaveFlagReaders[0].key == 1u &&
                  mmu::kDirectSaveFlagReaders[0].mask == 0x08u &&
                  mmu::kSaveFlagMask8Keys[0] == 2u &&
                  mmu::kSaveFlagMask4Keys[0] == 21u);

        const mmu::DecompStringXref* tellyXref = mmu::decompStringXref("Telly");
        check("decomp index",
              mmu::kDecompiledFunctionVasCount == 404 &&
                  mmu::kDecompStringXrefCount == 12 &&
                  mmu::kDecompTargetsTargetVasCount == 293 &&
                  mmu::kDecompFlowTargetVasCount == 100 &&
                  mmu::kDecompDispatcherTargetVasCount == 19 &&
                  mmu::kDecompUniqueTargetVasCount == 384 &&
                  mmu::hasDecompiledFunction(0x4012f0u) &&
                  mmu::hasDecompiledFunction(0x446c20u) &&
                  !mmu::hasDecompiledFunction(0xdeadbeefu) &&
                  tellyXref != nullptr && tellyXref->stringVa == 0x67b160u &&
                  tellyXref->refCount == 1 && tellyXref->refVas[0] == 0x586470u &&
                  mmu::decompStringXref("Missing") == nullptr);

        const mmu::ProtoSchema* gunSchema = mmu::protoSchema("gun");
        const mmu::ProtoMessage* gunGameObject = mmu::protoMessage("gun", "GameObject");
        const mmu::ProtoField* protoKeyField = mmu::protoField("key");
        check("proto schema index",
              mmu::kProtoSchemaCount == 6 && mmu::kProtoMessageCount == 24 &&
                  mmu::kProtoFieldCount == 166 && gunSchema != nullptr &&
                  gunSchema->syntax == "proto2" && gunSchema->package == "mmu" &&
                  gunSchema->messageCount == 4 && gunSchema->fieldCount == 16 &&
                  gunSchema->source.find("message Guns {") != std::string_view::npos &&
                  gunGameObject != nullptr && gunGameObject->fieldCount == 5 &&
                  gunGameObject->fieldOffset >= gunSchema->fieldOffset &&
                  mmu::kProtoFields[gunGameObject->fieldOffset].schema == "gun" &&
                  mmu::kProtoFields[gunGameObject->fieldOffset].message ==
                      "GameObject" &&
                  protoKeyField != nullptr &&
                  protoKeyField->schema == "animation" &&
                  protoKeyField->message == "Sprite" &&
                  protoKeyField->number == 4 &&
                  protoKeyField->sourceLine.find("required uint32 key = 4;") !=
                      std::string_view::npos);

        check("boot cutscene raw table",
              mmu::kBootActivityMask == 0x80u &&
                  mmu::kBootConstructorVa == 0x50bbb0u &&
                  mmu::kBootManagerGlobalVa == 0x6c55a0u &&
                  mmu::kBootManagerWriteCount == 3 &&
                  mmu::kBootManagerWrites[0].offset == 0x08u &&
                  mmu::kBootManagerWrites[0].value == 0x0080u &&
                  mmu::kBootPlayCutscene ==
                      "..\\Cutscenes\\CSN00Logos\\CSN00Logos.xcut" &&
                  mmu::kBootPendingCutscene ==
                      "..\\Cutscenes\\CSN01Intro\\csn01Intro.xcut");

        check("scene path raw table",
              mmu::kSceneNameCount == 791 &&
                  mmu::sceneName(0) == "\\Levels\\YoyoMan_Section_1.xscn" &&
                  mmu::sceneName(mmu::kSceneNameCount - 1) ==
                      "\\Levels\\EndlessNew1VUP5H_001B.xscn" &&
                  mmu::sceneNameIndex(mmu::sceneName(0)) == 0 &&
                  mmu::sceneNameIndex("Missing") == mmu::kSceneNameCount &&
                  mmu::sceneName(mmu::kSceneNameCount).empty());

        const mmu::RecoveredContinueStage* yoyoContinue =
            mmu::findContinueStage(5u);
        check("continue stage raw table",
              mmu::kRecoveredContinueStageCount == 38 &&
                  mmu::kRecoveredContinueStages[0].stageId == 1u &&
                  mmu::kRecoveredContinueStages[0].constructorVa == 0x528420u &&
                  mmu::kRecoveredContinueStages[0].className == "LevelTankMan" &&
                  yoyoContinue != nullptr &&
                  yoyoContinue->constructorVa == 0x512a90u &&
                  yoyoContinue->className == "LevelYoYoMan" &&
                  mmu::kRecoveredContinueStages[
                      mmu::kRecoveredContinueStageCount - 1].stageId == 39u &&
                  mmu::findContinueStage(15u) == nullptr);

        check("registered cutscene key table",
              mmu::kRegisteredCutsceneCount == 80 &&
                  mmu::kRegisteredCutscenes[0].key == 1548880663u &&
                  mmu::kRegisteredCutscenes[0].path ==
                      "..\\Cutscenes\\CSN00Logos\\CSN00Logos.xcut" &&
                  mmu::kRegisteredCutscenes[
                      mmu::kRegisteredCutsceneCount - 1].key == 3480670622u &&
                  mmu::cutscenePathForKey(1548880663u) ==
                      mmu::kRegisteredCutscenes[0].path &&
                  mmu::cutscenePathForKey(0u).empty());

        check("mask raw site tables",
              mmu::kRecoveredActivityMaskCallCount == 24 &&
                  mmu::kRecoveredActivityMaskCalls[0].functionVa == 0x4f3040u &&
                  mmu::kRecoveredActivityMaskCalls[0].callVa == 0x4f3099u &&
                  mmu::kRecoveredActivityMaskCalls[0].mask == 0x200u &&
                  mmu::kRecoveredActivityMaskCalls[0].owner == "TitleMenu" &&
                  mmu::kRecoveredActivityMaskCalls[0].slot == 5u &&
                  mmu::kRecoveredActivityMaskReadCount == 36 &&
                  mmu::kRecoveredActivityMaskReads[0].functionVa == 0x43cc00u &&
                  mmu::kRecoveredActivityMaskReads[0].callVa == 0x43d061u &&
                  mmu::kRecoveredActivityMaskReads[0].kind ==
                      mmu::ActivityMaskReadKind::Intersect &&
                  mmu::kRecoveredActivityMaskReads[0].owner ==
                      "?$MetJackDebris@$00" &&
                  mmu::kRecoveredActivityMaskReads[0].slot == 14u);

        check("key sweep counter summary",
              mmu::kKeySweepCandidateCount == 84651u &&
                  mmu::kKeySweepCollectedCount == 490u &&
                  mmu::kKeySweepPreviouslyKnownCount == 137u &&
                  mmu::kKeySweepNewMatchCount == 0u);

        check("coverage artifact summary",
              mmu::kCoverageVerifiedKeys == 137u &&
                  mmu::kCoverageCollectedKeys == 490u &&
                  mmu::kCoverageUnresolvedKeys == 353u &&
                  mmu::kCoverageMitmUnverifiedKeys == 5u &&
                  mmu::kCoverageSlotStates == 671u &&
                  mmu::kCoverageSlotComplete == 671u &&
                  mmu::kCoverageSlotIncomplete == 0u &&
                  mmu::kCoverageRawReports == 38u &&
                  mmu::kCoverageHeaderBackedReports == 38u &&
                  mmu::kCoverageFunctionsTotal == 16957u &&
                  mmu::kCoverageMegaManFunctions == 150u &&
                  mmu::kCoverageGeneratedHeaders == 39u &&
                  mmu::kCoverageGeneratedHeadersDirect == 37u &&
                  mmu::kCoverageGeneratedHeadersInHarness == 39u &&
                  mmu::kCoverageRegisteredCommands == 64u &&
                  mmu::kCoverageDocumentedCommands == 64u &&
                  mmu::kCoverageDecompiledFunctions == 404u &&
                  mmu::kCoverageRttiVtables == 347u &&
                  mmu::kCoverageRttiVtableSlots == 9127u &&
                  mmu::kCoverageProtoSchemas == 6u &&
                  mmu::kCoverageProtoMessages == 24u &&
                  mmu::kCoverageProtoFields == 166u &&
                  mmu::kCoverageSceneNames == 791u &&
                  mmu::kCoverageUnmappedReportCount == 0u);

        check("flow raw evidence table",
              mmu::kGameFlowEvidenceLineCount == 2643u &&
                  mmu::gameFlowEvidenceLine(1) ==
                      "MMU top-level flow evidence (generated by tools/analyze_game_flow.py)" &&
                  mmu::gameFlowEvidenceLine(mmu::kGameFlowEvidenceLineCount) ==
                      "   1  \\Levels\\CometWoman\\Sprites\\CometWoman_Section04.xspr" &&
                  mmu::gameFlowEvidenceLine(0).empty() &&
                  mmu::gameFlowEvidenceLine(
                      mmu::kGameFlowEvidenceLineCount + 1).empty());

        check("rtti class name raw table",
              mmu::kRttiClassNameCount == 348u &&
                  mmu::kRttiClassNames[0] == "type_info" &&
                  mmu::rttiClassNameIndex("BaseObject") == 1u &&
                  mmu::rttiClassNameIndex("Missing") ==
                      mmu::kRttiClassNameCount);

        bool menuVisible = true;
        bool menuToggleHeld = false;
        updateMenuToggle(true, menuVisible, menuToggleHeld);
        const bool menuHiddenOnce = !menuVisible;
        updateMenuToggle(true, menuVisible, menuToggleHeld);
        const bool menuStaysHiddenWhileHeld = !menuVisible;
        updateMenuToggle(false, menuVisible, menuToggleHeld);
        updateMenuToggle(true, menuVisible, menuToggleHeld);
        check("menu edge toggle",
              menuHiddenOnce && menuStaysHiddenWhileHeld && menuVisible);

#if defined(__SWITCH__) || defined(__SWITCH2__)
        check("switch face buttons",
              kUiConfirmButton == SDL_CONTROLLER_BUTTON_B &&
                  kUiCancelButton == SDL_CONTROLLER_BUTTON_A);
#else
        check("desktop face buttons",
              kUiConfirmButton == SDL_CONTROLLER_BUTTON_A &&
                  kUiCancelButton == SDL_CONTROLLER_BUTTON_B);
#endif

        const UiProfile desktopProfile;
        const WindowLayoutDefaults consoleLayout =
            computeWindowDefaults(kWindowConsole, desktopProfile, 1280, 720);
        const WindowLayoutDefaults harnessLayout =
            computeWindowDefaults(kWindowHarnessView, desktopProfile, 1280, 720);
        check("window defaults fit output",
              consoleLayout.size.x > 400.0f && consoleLayout.size.y > 240.0f &&
                  consoleLayout.pos.x >= 0.0f && consoleLayout.pos.y >= 0.0f &&
                  consoleLayout.pos.x + consoleLayout.size.x <= 1280.0f &&
                  consoleLayout.pos.y + consoleLayout.size.y <= 720.0f &&
                  harnessLayout.size.x > 600.0f && harnessLayout.size.y > 480.0f &&
                  harnessLayout.pos.x + harnessLayout.size.x <= 1280.0f &&
                  harnessLayout.pos.y + harnessLayout.size.y <= 720.0f);

        const bool selftestLoaded = loadPackedArchive(state, "assets.pak");
        if (selftestLoaded) {
            std::size_t sfx = 0, music = 0;
            for (const mmu::PackedEntry& entry : state.pak.entries()) {
                if (entry.type == mmu::PackedEntryType::Audio) ++sfx;
                else if (entry.type == mmu::PackedEntryType::Music) ++music;
            }
            std::snprintf(line, sizeof(line),
                          "packed entries=%zu images=%zu sprites=%zu frames=%zu sfx=%zu music=%zu scenes=%zu spawns=%zu",
                          state.pak.entries().size(), state.pak.imageCount(),
                          state.pak.sprites().size(), state.pak.frames().size(), sfx, music,
                          state.pak.scenes().size(), state.pak.spawns().size());
            state.console.log(line);

            check("packed cutscene table",
                  state.assets.cutscenes().size() == 80 &&
                      state.assets.cutsceneKeys().size() == 80 &&
                      state.assets.cutscenePaths().size() == 80 &&
                      state.assets.stats().commands > 7000);

            GameRuntime handoffRuntime;
            mmu::GameFlowRuntime& handoffFlow = state.flowRuntime;
            handoffFlow.reset(mmu::FlowScreen::TitleMenu);
            mmu::FlowInput confirm{};
            confirm.confirm = true;
            handoffFlow.step(0.6f, mmu::FlowInput{});
            handoffFlow.step(0.6f, mmu::FlowInput{});
            handoffFlow.step(1.0f / 60.0f, confirm);
            handoffFlow.step(0.6f, mmu::FlowInput{});
            handoffFlow.step(0.8f, mmu::FlowInput{});
            while (handoffFlow.pendingEventCount() != 0) {
                const mmu::FlowEvent event = handoffFlow.pollEvent();
                if (event.type == mmu::FlowEventType::ScreenChangeRequested &&
                    event.screen == mmu::FlowScreen::TitleMenu) {
                    handoffFlow.setScreen(mmu::FlowScreen::MainMenu);
                    handoffFlow.setMenuSupplier(0x508820u);
                }
            }
            handoffFlow.step(0.8f, mmu::FlowInput{});
            handoffFlow.step(1.0f / 60.0f, confirm);
            handoffFlow.step(0.6f, mmu::FlowInput{});
            bool stageSelectActionClean = false;
            while (handoffFlow.pendingEventCount() != 0) {
                const mmu::FlowEvent event = handoffFlow.pollEvent();
                if (event.type == mmu::FlowEventType::MenuAction &&
                    event.actionCode == 0x06u && event.hasDestination) {
                    stageSelectActionClean = event.cutscenePath.empty();
                    handoffFlow.setScreen(event.destination);
                }
            }
            handoffFlow.step(0.8f, mmu::FlowInput{});
            handoffFlow.step(1.0f / 60.0f, confirm);
            handoffFlow.step(0.6f, mmu::FlowInput{});
            bool handoffStarted = false;
            while (handoffFlow.pendingEventCount() != 0) {
                const mmu::FlowEvent event = handoffFlow.pollEvent();
                if (event.type == mmu::FlowEventType::StageSelected)
                    handoffRuntime.consumeFlowEvent(state, event);
                else if (event.type == mmu::FlowEventType::LevelSetupApplied)
                    handoffStarted |= handoffRuntime.consumeFlowEvent(state, event);
            }
            check("stage handoff to gameplay",
                  stageSelectActionClean && handoffStarted &&
                      handoffRuntime.mode == GameMode::Playing &&
                      handoffRuntime.sceneIndex >= 0 &&
                      handoffRuntime.sceneIndex <
                          static_cast<int>(state.pak.scenes().size()) &&
                      state.pak.scenes()[handoffRuntime.sceneIndex].name.find(
                          "JetMan") != std::string_view::npos &&
                      state.pak.scenes()[handoffRuntime.sceneIndex].spawnCount != 0 &&
                      handoffFlow.pendingEventCount() == 0);

            mmu::World handoffWorld;
            handoffWorld.loadScene(state.pak,
                                   state.pak.scenes()[handoffRuntime.sceneIndex],
                                   mmu::resolveTypeImages(state.pak));
            mmu::PlayerInput handoffInput{};
            handoffInput.moveX = 1.0f;
            const float handoffX0 = handoffWorld.controller().x;
            for (int i = 0; i < 30; ++i)
                handoffWorld.step(1.0f / 60.0f, handoffInput);
            const float handoffX1 = handoffWorld.controller().x;
            const float handoffVX = handoffWorld.controller().vx;
            std::snprintf(line, sizeof(line),
                          "  handoff player: state=%s x=%.0f -> %.0f vx=%.0f objects=%zu",
                          mmu::playerStateName(handoffWorld.controller().state),
                          handoffX0, handoffX1, handoffVX,
                          handoffWorld.objects().size());
            state.console.log(line);
            check("stage handoff player accepts input",
                  handoffWorld.hasScene() && handoffWorld.player() != nullptr &&
                      handoffWorld.controller().facingRight && handoffX1 > handoffX0);

            constexpr u32 kLogoKey = 1548880663u;
            check("cutscene key mapping",
                  mmu::cutsceneIndexForKey(state.assets, kLogoKey) == 0 &&
                      std::string(mmu::cutscenePathForIndex(state.assets, 0))
                              .find("CSN00Logos.xcut") != std::string::npos);
            mmu::CutsceneRuntime cutscene;
            cutscene.attach(state.assets);
            check("cutscene start",
                  cutscene.startKey(kLogoKey) && cutscene.playing() &&
                      cutscene.clockMs() == 0.0f &&
                      cutscene.sceneName().find("CSN00Logos") != std::string_view::npos);
            cutscene.update(0.05f);
            bool hasLogoElement = false;
            for (const mmu::CutsceneElement& element : cutscene.elements())
                hasLogoElement |= element.id == "TextBG" &&
                                  element.resource == "TextBG.xgun" &&
                                  element.anim == "TextBG";
            check("cutscene timestamp zero",
                  hasLogoElement && cutscene.pendingEventCount() >= 2 &&
                      cutscene.currentTimestamp() >= 1);
            bool resolvedLogo = false;
            for (const mmu::CutsceneElement& element : cutscene.elements()) {
                if (element.id != "TextBG") continue;
                const mmu::CutsceneSpriteRef ref = mmu::resolveCutsceneSprite(
                    state.pak, cutscene.sceneName(), element.resource, element.anim,
                    element.id);
                resolvedLogo = ref.gameObjectKey == 0x5122d3edu && ref.valid() &&
                               ref.imageName.find("EmptyScreen") !=
                                   std::string_view::npos;
            }
            check("cutscene sprite resolve", resolvedLogo);
            const mmu::CutsceneSpriteRef megaRef = mmu::resolveCutsceneSprite(
                state.pak, cutscene.sceneName(), {}, {}, "MegaMan");
            check("cutscene object id resolve",
                  megaRef.gameObjectKey == 0xc31e776au && megaRef.valid());
            cutscene.update(1.1f);
            check("cutscene timestamp advance",
                  cutscene.currentTimestamp() > 1 &&
                      cutscene.pendingEventCount() > 2 &&
                      cutscene.clockMs() >= 1100.0f);
            cutscene.update(12.0f);
            check("cutscene EndScene", !cutscene.playing() &&
                                           cutscene.currentTimestamp() >= 1);
            check("cutscene path lookup",
                  cutscene.startPathSuffix("csn00logos.xcut") &&
                      cutscene.sceneName().find("CSN00Logos") != std::string_view::npos);
            cutscene.stop();

            check("boot cutscene table",
                  mmu::kBootActivityMask == 0x80u &&
                      mmu::kBootPlayCutscene.find("CSN00Logos") !=
                          std::string_view::npos &&
                      mmu::findCutsceneByPathSuffix(state.assets,
                                                    mmu::kBootPlayCutscene) !=
                          static_cast<std::size_t>(-1) &&
                      mmu::findCutsceneByPathSuffix(
                          state.assets, mmu::kBootPendingCutscene) !=
                          static_cast<std::size_t>(-1));
            state.cutsceneRuntime.stop();

            // FUN_00511350 completion flag 7 starts CSN01Intro before loader
            // routing. The harness must start the literal path carried by the
            // StageCompleted event, not a guessed scene name.
            state.flowRuntime.reset();
            state.flowRuntime.setCompletionFlags(1ull << 7);
            state.flowRuntime.clearEvents();
            state.flowRuntime.completeStage(0x99u);
            bool completionCutsceneStarted = false;
            mmu::GameFlowRuntime completionProbe = state.flowRuntime;
            while (completionProbe.pendingEventCount() != 0) {
                const mmu::FlowEvent completed = completionProbe.pollEvent();
                if (completed.type == mmu::FlowEventType::StageCompleted &&
                    !completed.cutscenePath.empty())
                    completionCutsceneStarted =
                        startFlowCutscene(state, completed.cutscenePath);
            }
            check("completion cutscene start",
                  completionCutsceneStarted && state.cutsceneRuntime.playing() &&
                      state.cutsceneRuntime.sceneName().find("CSN01Intro") !=
                          std::string_view::npos);
            state.cutsceneRuntime.stop();
            state.flowRuntime.reset();
            check("cutscene stop", !cutscene.playing() && cutscene.elements().empty() &&
                                       cutscene.texts().empty() &&
                                       cutscene.pendingEventCount() == 0);

            for (const mmu::PackedEntry& entry : state.pak.entries()) {
                if (entry.type != mmu::PackedEntryType::Audio) continue;
                std::snprintf(line, sizeof(line), "  sfx key=%u bytes=%zu vol=%u %.*s",
                              entry.key, entry.data.size, entry.volume,
                              static_cast<int>(entry.name.size()), entry.name.data());
                state.console.log(line);
                break;
            }
            for (const mmu::PackedEntry& entry : state.pak.entries()) {
                if (entry.type != mmu::PackedEntryType::Music) continue;
                const mmu::sdl2::AudioProbe probe = mmu::sdl2::probeMusic(entry);
                std::snprintf(line, sizeof(line),
                              "  music key=%u ogg=%s %d Hz %d ch frames=%zu decoded=%zu %.*s",
                              entry.key, probe.ok ? "ok" : "FAIL", probe.sampleRate,
                              probe.channels, probe.frames, probe.decodedSamples,
                              static_cast<int>(entry.name.size()), entry.name.data());
                state.console.log(line);
                break;
            }
            {
                u32 musicKey = 0;
                for (const mmu::PackedEntry& entry : state.pak.entries()) {
                    if (entry.type != mmu::PackedEntryType::Music) continue;
                    musicKey = entry.key;
                    break;
                }
                mmu::sdl2::AudioEngine playback;
                if (musicKey != 0 && playback.open()) {
                    check("music playback active",
                          playback.playMusic(state.pak, musicKey) &&
                              playback.status().activeVoices == 1);
                    playback.close();
                } else {
                    state.console.log("  music playback check skipped (no audio device)");
                }
            }
            if (!state.pak.sprites().empty()) {
                const mmu::PackedSprite& sprite = state.pak.sprites().front();
                std::snprintf(line, sizeof(line), "  sprite key=%u frames=%u handle=(%d,%d) %s",
                              sprite.key, sprite.frameCount, sprite.xHandle, sprite.yHandle,
                              spriteLabel(state.pak, sprite).c_str());
                state.console.log(line);
            }
            if (!state.pak.scenes().empty()) {
                const mmu::PackedScene& scene = state.pak.scenes().front();
                const mmu::PackedSpawn* first = scene.spawnCount ? &scene.spawns[0] : nullptr;
                std::snprintf(line, sizeof(line), "  scene key=%u spawns=%u firstType=%.*s",
                              scene.key, scene.spawnCount,
                              first ? static_cast<int>(first->type.size()) : 0,
                              first ? first->type.data() : "");
                state.console.log(line);
                std::snprintf(line, sizeof(line), "  scene name=%.*s",
                              static_cast<int>(scene.name.size()), scene.name.data());
                state.console.log(line);
                const mmu::SceneArt art = mmu::resolveSceneArt(state.pak, scene);
                std::snprintf(line, sizeof(line),
                              "  scene art frames=%zu first=%ux%u",
                              art.frames.size(), art.width, art.height);
                state.console.log(line);
                std::size_t withArt = 0;
                for (const mmu::PackedScene& candidate : state.pak.scenes())
                    if (!mmu::resolveSceneArt(state.pak, candidate).empty()) ++withArt;
                std::snprintf(line, sizeof(line), "  scene art coverage=%zu/%zu",
                              withArt, state.pak.scenes().size());
                state.console.log(line);
                std::size_t withCollision = 0;
                std::size_t withLoadableCollision = 0;
                int shownUnloadableCollision = 0;
                for (const mmu::PackedScene& candidate : state.pak.scenes()) {
                    const u32 image = mmu::findSceneCollisionImage(state.pak, candidate);
                    if (image == mmu::CollisionMap::kNoImage) continue;
                    ++withCollision;
                    const mmu::PackedEntry& imageEntry = state.pak.entries()[image];
                    if (mmu::collisionImageLoadable(imageEntry))
                        ++withLoadableCollision;
                    else if (shownUnloadableCollision < 8) {
                        std::snprintf(line, sizeof(line),
                                      "  unloadable collision: %.*s (bpp=%u pal=%zu %ux%u)",
                                      static_cast<int>(candidate.name.size()),
                                      candidate.name.data(), imageEntry.bitsPerPixel,
                                      imageEntry.palette.size, imageEntry.width,
                                      imageEntry.height);
                        state.console.log(line);
                        ++shownUnloadableCollision;
                    }
                }
                std::snprintf(line, sizeof(line),
                              "  scene collision coverage=%zu/%zu loadable=%zu/%zu",
                              withCollision, state.pak.scenes().size(),
                              withLoadableCollision, state.pak.scenes().size());
                state.console.log(line);
                int shownMissingCollision = 0;
                for (const mmu::PackedScene& candidate : state.pak.scenes()) {
                    if (mmu::findSceneCollisionImage(state.pak, candidate) !=
                        mmu::CollisionMap::kNoImage)
                        continue;
                    std::snprintf(line, sizeof(line), "  no collision: %.*s",
                                  static_cast<int>(candidate.name.size()),
                                  candidate.name.data());
                    state.console.log(line);
                    if (++shownMissingCollision >= 8) break;
                }
            }
            state.typeImages = mmu::resolveTypeImages(state.pak);
            std::snprintf(line, sizeof(line), "  resolved spawn types=%zu", state.typeImages.size());
            state.console.log(line);
            int shownTypes = 0;
            for (const auto& resolved : state.typeImages) {
                if (shownTypes++ >= 3) break;
                const mmu::PackedEntry& image = state.pak.entries()[resolved.second];
                std::snprintf(line, sizeof(line), "    %s -> image %u %.*s",
                              resolved.first.c_str(), resolved.second,
                              static_cast<int>(image.name.size()), image.name.data());
                state.console.log(line);
            }
            if (!state.pak.scenes().empty()) {
                mmu::World sim;
                sim.loadScene(state.pak, state.pak.scenes().front(), state.typeImages);
                const mmu::WorldObject* before = sim.player();
                const float x0 = before ? before->x : 0.0f;
                const float y0 = before ? before->y : 0.0f;
                for (int i = 0; i < 60; ++i) sim.step(1.0f / 60.0f, 96.0f, 0.0f);
                const mmu::WorldObject* after = sim.player();
                std::snprintf(line, sizeof(line),
                              "  world sim: objects=%zu camera=(%.0f,%.0f) player (%.0f,%.0f) -> (%.0f,%.0f) t=%.2f",
                              sim.objects().size(), sim.camera().x, sim.camera().y,
                              x0, y0, after ? after->x : 0.0f, after ? after->y : 0.0f,
                              sim.time());
                state.console.log(line);
                check("world sim moves player", after && after->x > x0 + 50.0f);

                // Reconstructed MegaMan path: GameParams from guns.mmu, the
                // room's typed collision (0x5b4970) and per-frame surfaces.
                {
                    mmu::World typed;
                    typed.loadScene(state.pak, state.pak.scenes().front(), state.typeImages);
                    const mmu::PlayerController& pc = typed.controller();
                    std::snprintf(line, sizeof(line),
                                  "  megaman object=%s params=%u move_speed=%.0f jump=%.0f level types=%ux%u",
                                  typed.playerObject() ? "yes" : "no",
                                  typed.playerObject() ? typed.playerObject()->paramCount : 0u,
                                  pc.params.moveSpeed, pc.params.jumpVelocity,
                                  typed.levelTypes().width, typed.levelTypes().height);
                    state.console.log(line);
                    check("megaman params from pak",
                          typed.playerObject() && pc.params.moveSpeed == 90.0f &&
                          pc.params.jumpVelocity == -325.0f);
                    check("typed level collision", typed.levelTypes().valid());
                    if (typed.playerObject()) {
                        for (const char* anim : {"stand", "run", "jump", "slide", "ladder"}) {
                            mmu::u32 h = 0;
                            for (const unsigned char* p = reinterpret_cast<const unsigned char*>(anim); *p; ++p)
                                for (int b = 7; b >= 0; --b) {
                                    const bool top = (h & 0x80000000u) != 0;
                                    h = (h << 1) | ((*p >> b) & 1u);
                                    if (top) h ^= 0x04c11db7u;
                                }
                            const mmu::PackedSprite* sp =
                                state.pak.animationOf(*typed.playerObject(), h);
                            std::snprintf(line, sizeof(line),
                                          "  anim %-7s sprite=%s frames=%u col0=%d handle=(%d,%d)",
                                          anim, sp ? "yes" : "no", sp ? sp->frameCount : 0u,
                                          sp && sp->frameCount ? static_cast<int>(sp->frames[0].collisionIndex) : -2,
                                          sp ? sp->xHandle : 0, sp ? sp->yHandle : 0);
                            state.console.log(line);
                        }
                    }
                    for (int i = 0; i < 180; ++i) typed.step(1.0f / 60.0f, mmu::PlayerInput{});
                    const float landY = pc.y;
                    std::snprintf(line, sizeof(line),
                                  "  typed idle: state=%s pos=(%.1f,%.1f) contacts=0x%x anim=%08x",
                                  mmu::playerStateName(pc.state), pc.x, pc.y, pc.contactFlags,
                                  pc.animKey);
                    state.console.log(line);
                    check("typed player lands", pc.state == mmu::PlayerState::Stand);
                    mmu::PlayerInput right;
                    right.moveX = 1.0f;
                    const float x0 = pc.x;
                    for (int i = 0; i < 60; ++i) typed.step(1.0f / 60.0f, right);
                    std::snprintf(line, sizeof(line),
                                  "  typed run: state=%s dx=%.1f dy=%.1f vx=%.0f",
                                  mmu::playerStateName(pc.state), pc.x - x0, pc.y - landY, pc.vx);
                    state.console.log(line);
                    check("typed run uses move_speed",
                          pc.state == mmu::PlayerState::Run && pc.x - x0 > 70.0f &&
                          pc.x - x0 < 95.0f);
                    mmu::PlayerInput jump;
                    jump.jump = true;
                    float peak = pc.y;
                    for (int i = 0; i < 90; ++i) {
                        typed.step(1.0f / 60.0f, jump);
                        peak = std::min(peak, pc.y);
                    }
                    std::snprintf(line, sizeof(line),
                                  "  typed jump: rise=%.1f end state=%s", pc.y - peak,
                                  mmu::playerStateName(pc.state));
                    state.console.log(line);
                    // v^2 / 2g = 325^2 / 1980 = 53.3 px for a held jump.
                    check("typed jump height", pc.y - peak > 45.0f && pc.y - peak < 58.0f);
                }

                mmu::World ctrlSim;
                ctrlSim.loadScene(state.pak, state.pak.scenes().front(), state.typeImages);
                mmu::PlayerInput runInput;
                runInput.moveX = 1.0f;
                for (int i = 0; i < 30; ++i) ctrlSim.step(1.0f / 60.0f, runInput);
                const float runX = ctrlSim.controller().x;
                std::snprintf(line, sizeof(line),
                              "  player run: state=%s x=%.0f y=%.0f grounded=%s hp=%.0f",
                              mmu::playerStateName(ctrlSim.controller().state), runX,
                              ctrlSim.controller().y,
                              ctrlSim.controller().grounded ? "yes" : "no",
                              ctrlSim.controller().health);
                state.console.log(line);
                // With collision active the spawn column can still be in the
                // air at t=0.5s; the invariant is rightward progress and no
                // accidental damage, not a particular animation state.
                check("player run advances", runX > 400.0f &&
                      ctrlSim.controller().health == 100.0f);
                ctrlSim.controller().handleEvent(mmu::PlayerEvent::Damage,
                                                 ctrlSim.controller().x + 24.0f);
                std::snprintf(line, sizeof(line), "  player hurt: state=%s hp=%.0f",
                              mmu::playerStateName(ctrlSim.controller().state),
                              ctrlSim.controller().health);
                state.console.log(line);
                check("player damage event",
                      ctrlSim.controller().state == mmu::PlayerState::Hurt &&
                      ctrlSim.controller().health == 90.0f);
                mmu::PlayerInput idle;
                for (int i = 0; i < 40; ++i) ctrlSim.step(1.0f / 60.0f, idle);
                std::snprintf(line, sizeof(line), "  player recovered: state=%s x=%.0f",
                              mmu::playerStateName(ctrlSim.controller().state),
                              ctrlSim.controller().x);
                state.console.log(line);
                check("player recovers",
                      ctrlSim.controller().state == mmu::PlayerState::Stand);
                const mmu::CollisionMap& col = ctrlSim.collision();
                std::snprintf(line, sizeof(line),
                              "  collision map: image=%u size=%ux%u player=(%.0f,%.0f) solid=%s grounded=%s",
                              col.imageIndex, col.width, col.height,
                              ctrlSim.controller().x, ctrlSim.controller().y,
                              col.valid() && col.solidAt(ctrlSim.controller().x,
                                                         ctrlSim.controller().y) ? "yes" : "no",
                              ctrlSim.controller().grounded ? "yes" : "no");
                state.console.log(line);
                check("collision map lands player",
                      col.valid() && ctrlSim.controller().grounded);

                std::snprintf(line, sizeof(line), "  scene enemy controllers=%zu",
                              ctrlSim.enemies().size());
                state.console.log(line);
                for (const mmu::EnemyController& enemy : ctrlSim.enemies()) {
                    if (enemy.kind != mmu::EnemyKind::Telly) continue;
                    std::snprintf(line, sizeof(line),
                                  "  scene telly: state=%s pos=(%.0f,%.0f) home=(%.0f,%.0f)",
                                  mmu::enemyStateName(enemy.state), enemy.x, enemy.y,
                                  enemy.homeX, enemy.homeY);
                    state.console.log(line);
                    break;
                }

                mmu::EnemyController telly;
                telly.reset(mmu::EnemyKind::Telly, 100.0f, 100.0f);
                for (int i = 0; i < 10; ++i) telly.update(1.0f / 60.0f, 200.0f, 100.0f);
                std::snprintf(line, sizeof(line), "  telly seek: state=%s x=%.0f vx=%.0f",
                              mmu::enemyStateName(telly.state), telly.x, telly.vx);
                state.console.log(line);
                check("telly Idle -> MoveH",
                      telly.state == mmu::EnemyState::MoveH && telly.vx > 0.0f);
                for (int i = 0; i < 150; ++i) telly.update(1.0f / 60.0f, 180.0f, 100.0f);
                std::snprintf(line, sizeof(line), "  telly arrive: state=%s x=%.0f home=%.0f range=%.0f",
                              mmu::enemyStateName(telly.state), telly.x,
                              telly.homeX, telly.range);
                state.console.log(line);
                check("telly MoveH -> Idle", telly.state == mmu::EnemyState::Idle);

                // Overshooting home range re-reads the player, exactly like
                // 0x586310/0x5861d0: MoveH can hand off to MoveV.
                telly.reset(mmu::EnemyKind::Telly, 100.0f, 100.0f);
                for (int i = 0; i < 10; ++i) telly.update(1.0f / 60.0f, 200.0f, 200.0f);
                for (int i = 0; i < 150; ++i) telly.update(1.0f / 60.0f, 150.0f, 220.0f);
                std::snprintf(line, sizeof(line), "  telly axis switch: state=%s pos=(%.0f,%.0f) vy=%.0f",
                              mmu::enemyStateName(telly.state), telly.x, telly.y, telly.vy);
                state.console.log(line);
                check("telly MoveH -> MoveV",
                      telly.state == mmu::EnemyState::MoveV && telly.vy != 0.0f);

                telly.reset(mmu::EnemyKind::Telly, 100.0f, 100.0f);
                for (int i = 0; i < 10; ++i) telly.update(1.0f / 60.0f, 100.0f, 200.0f);
                std::snprintf(line, sizeof(line), "  telly vertical seek: state=%s y=%.0f vy=%.0f",
                              mmu::enemyStateName(telly.state), telly.y, telly.vy);
                state.console.log(line);
                check("telly Idle -> MoveV",
                      telly.state == mmu::EnemyState::MoveV && telly.vy > 0.0f);
                for (int i = 0; i < 150; ++i) telly.update(1.0f / 60.0f, 100.0f, 160.0f);
                std::snprintf(line, sizeof(line), "  telly vertical arrive: state=%s y=%.0f home=%.0f",
                              mmu::enemyStateName(telly.state), telly.y, telly.homeY);
                state.console.log(line);
                check("telly MoveV -> Idle", telly.state == mmu::EnemyState::Idle);

                // EZBlock: the 0x614a20 constructor registers Wait then Idle.
                mmu::EnemyController block;
                block.reset(mmu::EnemyKind::EZBlock, 200.0f, 100.0f);
                for (int i = 0; i < 70; ++i) block.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("ezblock Wait -> Idle", block.state == mmu::EnemyState::Idle);
                for (int i = 0; i < 70; ++i) block.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("ezblock Idle -> Wait", block.state == mmu::EnemyState::Wait);

                // MineE: 0x58f540 registers Scan then Explode; the update
                // callback switches when the player enters the activation radius.
                mmu::EnemyController mine;
                mine.reset(mmu::EnemyKind::MineE, 400.0f, 100.0f);
                mine.update(1.0f / 60.0f, 100.0f, 100.0f);
                check("minee stays Scan when far", mine.state == mmu::EnemyState::Scan);
                mine.update(1.0f / 60.0f, 420.0f, 100.0f);
                check("minee Scan -> Explode",
                      mine.state == mmu::EnemyState::Explode && mine.activated);
                for (int i = 0; i < 30; ++i) mine.update(1.0f / 60.0f, 420.0f, 100.0f);
                check("minee Explode -> Death", mine.state == mmu::EnemyState::Death);

                // SawPole: constructor 0x5a5870 registers Patrol only.
                mmu::EnemyController saw;
                saw.reset(mmu::EnemyKind::SawPole, 300.0f, 100.0f, true);
                for (int i = 0; i < 10; ++i) saw.update(1.0f / 60.0f, 300.0f, 100.0f);
                const float sawX = saw.x;
                check("sawpole Patrol advances",
                      saw.state == mmu::EnemyState::Patrol && sawX > 300.0f);
                const float initialSign = saw.moveSign;
                bool sawReversed = false;
                for (int i = 0; i < 200; ++i) {
                    saw.update(1.0f / 60.0f, 300.0f, 100.0f);
                    if (saw.moveSign != initialSign) sawReversed = true;
                }
                check("sawpole Patrol reverses", sawReversed);

                // SuperCannon: 0x5a45d0 registers Stand / Turn / Attack. The
                // Attack enter callback 0x5a43c0 spawns a projectile volley.
                mmu::EnemyController cannon;
                cannon.reset(mmu::EnemyKind::SuperCannon, 500.0f, 100.0f, true);
                for (int i = 0; i < 115; ++i) cannon.update(1.0f / 60.0f, 400.0f, 100.0f);
                check("supercannon Stand -> Turn", cannon.state == mmu::EnemyState::Turn);
                for (int i = 0; i < 25; ++i) cannon.update(1.0f / 60.0f, 400.0f, 100.0f);
                check("supercannon Turn -> Attack",
                      cannon.state == mmu::EnemyState::Attack && cannon.shotsFired == 1);
                for (int i = 0; i < 55; ++i) cannon.update(1.0f / 60.0f, 400.0f, 100.0f);
                check("supercannon Attack -> Stand", cannon.state == mmu::EnemyState::Stand);

                // 0x692024 registers Flasher Move / Attack; slot 24 sets Attack
                // and slot 27 plays FlasherAttack before returning to Move.
                mmu::EnemyController flasher;
                flasher.reset(mmu::EnemyKind::Flasher, 500.0f, 100.0f, true);
                check("flasher initial Move", flasher.state == mmu::EnemyState::Move);
                for (int i = 0; i < 95; ++i) flasher.update(1.0f / 60.0f, 400.0f, 100.0f);
                check("flasher Move -> Attack",
                      flasher.state == mmu::EnemyState::Attack && flasher.shotsFired == 1);
                for (int i = 0; i < 30; ++i) flasher.update(1.0f / 60.0f, 400.0f, 100.0f);
                check("flasher Attack -> Move", flasher.state == mmu::EnemyState::Move);

                // 0x691554 registers Electrode Init / Move / Shoot; slot 29
                // plays the Electrode cue before slot 30 returns to Move.
                mmu::EnemyController electrode;
                electrode.reset(mmu::EnemyKind::Electrode, 500.0f, 100.0f, true);
                check("electrode initial Init", electrode.state == mmu::EnemyState::Init);
                for (int i = 0; i < 35; ++i) electrode.update(1.0f / 60.0f, 400.0f, 100.0f);
                for (int i = 0; i < 100; ++i) electrode.update(1.0f / 60.0f, 400.0f, 100.0f);
                check("electrode Move -> Shoot",
                      electrode.state == mmu::EnemyState::Shoot && electrode.shotsFired == 1);
                for (int i = 0; i < 30; ++i) electrode.update(1.0f / 60.0f, 400.0f, 100.0f);
                check("electrode Shoot -> Move", electrode.state == mmu::EnemyState::Move);

                // Concrete archetypes recovered from constructor vftables.
                // Type inference must pick them before the broad platform and
                // shooter fallbacks.
                check("enemy archetype inference",
                      mmu::inferEnemyKind("ExplosivePlatform") == mmu::EnemyKind::ExplosivePlatform &&
                      mmu::inferEnemyKind("YokuSpike") == mmu::EnemyKind::YokuSpike &&
                      mmu::inferEnemyKind("MetLaser") == mmu::EnemyKind::MetLaser &&
                      mmu::inferEnemyKind("CrossShooter") == mmu::EnemyKind::CrossShooter &&
                      mmu::inferEnemyKind("CannonBall") == mmu::EnemyKind::CannonBall);

                // ExplosivePlatform 0x69500c: Idle / Flash with a 0.8s fuse.
                mmu::EnemyController explosive;
                explosive.reset(mmu::EnemyKind::ExplosivePlatform, 240.0f, 100.0f);
                bool sawFlash = false, sawExplosion = false;
                for (int i = 0; i < 80; ++i) {
                    explosive.update(1.0f / 60.0f, 240.0f, 100.0f);
                    sawFlash |= explosive.state == mmu::EnemyState::Flash;
                    for (const mmu::SpawnedObjectEvent& event : explosive.spawnEvents)
                        sawExplosion |= event.kind == mmu::SpawnedObjectKind::EnemyExplosion;
                }
                check("explosive platform Idle -> Flash -> explosion",
                      sawFlash && sawExplosion && explosive.activated &&
                      explosive.state == mmu::EnemyState::Death);

                // YokuBlock 0x6950dc: Wait / Appear / Idle; YokuSpike shares
                // all slots through its 0x69514c vftable.
                mmu::EnemyController yoku;
                yoku.reset(mmu::EnemyKind::YokuSpike, 260.0f, 100.0f);
                check("yoku initial Wait", yoku.state == mmu::EnemyState::Wait);
                for (int i = 0; i < 80; ++i) yoku.update(1.0f / 60.0f, 260.0f, 100.0f);
                check("yoku Wait -> Appear", yoku.state == mmu::EnemyState::Appear);
                for (int i = 0; i < 25; ++i) yoku.update(1.0f / 60.0f, 260.0f, 100.0f);
                check("yoku Appear -> Idle", yoku.state == mmu::EnemyState::Idle);
                for (int i = 0; i < 40; ++i) yoku.update(1.0f / 60.0f, 260.0f, 100.0f);
                check("yoku Idle -> Wait", yoku.state == mmu::EnemyState::Wait);

                // MetStand 0x68e2bc: Pop -> Fire -> Hidden -> Stand. Hidden
                // clears contact damage and Fire emits one EnemyShot.
                mmu::EnemyController met;
                met.reset(mmu::EnemyKind::MetStand, 320.0f, 100.0f);
                mmu::SpawnedObjectEvent firstEnemyShot;
                bool haveEnemyShot = false;
                for (int i = 0; i < 70; ++i) met.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("metstand Stand -> Pop", met.state == mmu::EnemyState::Pop);
                for (int i = 0; i < 25; ++i) {
                    met.update(1.0f / 60.0f, 200.0f, 100.0f);
                    for (const mmu::SpawnedObjectEvent& event : met.spawnEvents)
                        if (event.kind == mmu::SpawnedObjectKind::EnemyShot && !haveEnemyShot) {
                            firstEnemyShot = event;
                            haveEnemyShot = true;
                        }
                }
                check("metstand Pop -> Fire emits shot",
                      met.state == mmu::EnemyState::Fire && met.shotsFired == 1 &&
                      haveEnemyShot && std::abs(firstEnemyShot.vx) > 1.0f);
                for (int i = 0; i < 30; ++i) met.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("metstand Fire -> Hidden clears contact",
                      met.state == mmu::EnemyState::Hidden && !met.contactDamage);
                for (int i = 0; i < 70; ++i) met.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("metstand Hidden -> Stand",
                      met.state == mmu::EnemyState::Stand && met.contactDamage);

                // MetLaser 0x69101c starts Hidden and cues a shot with Pop.
                mmu::EnemyController metLaser;
                metLaser.reset(mmu::EnemyKind::MetLaser, 340.0f, 100.0f);
                check("metlaser initial Hidden",
                      metLaser.state == mmu::EnemyState::Hidden && !metLaser.contactDamage);
                for (int i = 0; i < 70; ++i)
                    metLaser.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("metlaser Hidden -> Pop emits shot",
                      metLaser.state == mmu::EnemyState::Pop && metLaser.shotsFired == 1);
                for (int i = 0; i < 25; ++i)
                    metLaser.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("metlaser Pop -> Exposed", metLaser.state == mmu::EnemyState::Exposed);
                for (int i = 0; i < 35; ++i)
                    metLaser.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("metlaser Exposed -> Hiding", metLaser.state == mmu::EnemyState::Hiding);
                for (int i = 0; i < 45; ++i)
                    metLaser.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("metlaser Hiding -> Hidden", metLaser.state == mmu::EnemyState::Hidden);

                // CrossShooter 0x691e4c and SpikePlatform 0x69522c share the
                // Wait / Shoot turret machine and CrossShooterShot cue.
                mmu::EnemyController turret;
                turret.reset(mmu::EnemyKind::CrossShooter, 380.0f, 100.0f);
                for (int i = 0; i < 80; ++i) turret.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("crossshooter Wait -> Shoot emits shot",
                      turret.state == mmu::EnemyState::Shoot && turret.shotsFired == 1);
                for (int i = 0; i < 25; ++i) turret.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("crossshooter Shoot -> Wait", turret.state == mmu::EnemyState::Wait);
                mmu::EnemyController spikePlatform;
                spikePlatform.reset(mmu::EnemyKind::SpikePlatform, 390.0f, 100.0f);
                check("spike platform initial Wait",
                      spikePlatform.state == mmu::EnemyState::Wait &&
                      !spikePlatform.contactDamage);

                // ShieldAttacker 0x690c84: Init -> Move (two shots) <-> Turn.
                mmu::EnemyController shield;
                shield.reset(mmu::EnemyKind::ShieldAttacker, 420.0f, 100.0f);
                for (int i = 0; i < 25; ++i) shield.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("shield attacker Init -> Move fires twice",
                      shield.state == mmu::EnemyState::Move && shield.shotsFired == 2);
                for (int i = 0; i < 80; ++i) shield.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("shield attacker Move -> Turn", shield.state == mmu::EnemyState::Turn);
                for (int i = 0; i < 30; ++i) shield.update(1.0f / 60.0f, 200.0f, 100.0f);
                check("shield attacker Turn -> Move", shield.state == mmu::EnemyState::Move);

                // CannonBall 0x6914ac: all six callbacks and both shots are
                // reachable; player position selects TurnUp versus TurnDown.
                mmu::EnemyController cannonBall;
                cannonBall.reset(mmu::EnemyKind::CannonBall, 460.0f, 100.0f);
                bool sawUp = false, sawShootUp = false, sawTurnUp = false;
                bool sawDown = false, sawShootDown = false, sawTurnDown = false;
                bool sawCannonShot = false;
                float cannonPlayerY = 100.0f;
                for (int i = 0; i < 600; ++i) {
                    if (cannonBall.state == mmu::EnemyState::ShootUp) cannonPlayerY = 0.0f;
                    if (cannonBall.state == mmu::EnemyState::ShootDown) cannonPlayerY = 200.0f;
                    cannonBall.update(1.0f / 60.0f, 460.0f, cannonPlayerY);
                    sawUp |= cannonBall.state == mmu::EnemyState::Up;
                    sawShootUp |= cannonBall.state == mmu::EnemyState::ShootUp;
                    sawTurnUp |= cannonBall.state == mmu::EnemyState::TurnUp;
                    sawDown |= cannonBall.state == mmu::EnemyState::Down;
                    sawShootDown |= cannonBall.state == mmu::EnemyState::ShootDown;
                    sawTurnDown |= cannonBall.state == mmu::EnemyState::TurnDown;
                    for (const mmu::SpawnedObjectEvent& event : cannonBall.spawnEvents)
                        sawCannonShot |= event.kind == mmu::SpawnedObjectKind::EnemyShot;
                }
                check("cannonball six-state cycle",
                      sawUp && sawShootUp && sawTurnUp && sawDown &&
                      sawShootDown && sawTurnDown && sawCannonShot);

                // Generic shots are real ballistic WorldObjects, not just a
                // console counter.
                if (haveEnemyShot) {
                    mmu::World shotSim;
                    shotSim.spawnProjectile(firstEnemyShot);
                    const float startX = shotSim.objects()[0].x;
                    const float startY = shotSim.objects()[0].y;
                    for (int i = 0; i < 30; ++i)
                        shotSim.step(1.0f / 60.0f, mmu::PlayerInput{});
                    const mmu::WorldObject& shot = shotSim.objects()[0];
                    const float speed = std::sqrt(shot.vx * shot.vx + shot.vy * shot.vy);
                    check("world moves EnemyShot at 180 px/s",
                          shotSim.projectileCount() == 1 && speed > 179.0f && speed < 181.0f &&
                          (std::abs(shot.x - startX) > 1.0f ||
                           std::abs(shot.y - startY) > 1.0f));
                } else {
                    check("world moves EnemyShot at 180 px/s", false);
                }

                // Base Boss vtable 0x6900cc: Spawn descends to player.y+112,
                // Pose fills the 28-unit meter over 2.24s, then Attack.
                mmu::EnemyController boss;
                boss.reset(mmu::EnemyKind::Boss, 600.0f, 100.0f);
                check("boss initial Spawn", boss.state == mmu::EnemyState::Spawn);
                for (int i = 0; i < 130; ++i) boss.update(1.0f / 60.0f, 600.0f, 100.0f);
                check("boss Spawn -> Pose", boss.state == mmu::EnemyState::Pose);
                for (int i = 0; i < 70; ++i) boss.update(1.0f / 60.0f, 600.0f, 100.0f);
                check("boss Pose -> Attack", boss.state == mmu::EnemyState::Attack);
                boss.hurt(2000.0f);
                check("boss lethal damage -> Death", boss.state == mmu::EnemyState::Death);

                // MidBoss vtable 0x691b94: Death -> BigExplosion -> MidBossExplosion.
                mmu::EnemyController midBoss;
                midBoss.reset(mmu::EnemyKind::MidBoss, 620.0f, 100.0f);
                midBoss.hurt(2000.0f);
                for (int i = 0; i < 60; ++i)
                    midBoss.update(1.0f / 60.0f, 620.0f, 100.0f);
                check("midboss Death -> BigExplosion",
                      midBoss.state == mmu::EnemyState::BigExplosion);
                for (int i = 0; i < 90; ++i)
                    midBoss.update(1.0f / 60.0f, 620.0f, 100.0f);
                check("midboss BigExplosion -> MidBossExplosion",
                      midBoss.state == mmu::EnemyState::MidBossExplosion);

                // WilyMachine vtable 0x692cc4: Wait -> four setup/attack pairs.
                // PillarDrop cues the persistent pillar with 0xf, Crush releases
                // it with 0x20, Lightning strikes twice and GotoAttack3 enter
                // emits the three-shot volley (0x56ccb0 / 0x56d940 / 0x56cf70).
                mmu::EnemyController machine;
                machine.reset(mmu::EnemyKind::WilyMachine, 700.0f, 100.0f);
                check("wily machine initial Wait", machine.state == mmu::EnemyState::Wait);
                bool sawRam = false, sawAttack3 = false;
                bool sawPillar = false, sawLightning = false, sawCrush = false;
                bool sawThreeShots = false, volleyFromGotoAttack3 = false;
                u32 maxStrikes = 0;
                u32 pillarSpawns = 0, dropCues = 0, followCues = 0;
                u32 shotsSpawned = 0, boltsSpawned = 0;
                mmu::SpawnedObjectEvent firstPillar, firstShot, firstBolt;
                mmu::SpawnedObjectEvent firstDropCue, firstFollowCue;
                bool havePillar = false, haveShot = false, haveBolt = false;
                bool haveDropCue = false, haveFollowCue = false;
                float shotDelays[3] = {-1.0f, -1.0f, -1.0f};
                bool shotDamageOk = true, boltDamageOk = true, pillarDamageOk = true;
                for (int i = 0; i < 2400; ++i) {
                    machine.update(1.0f / 60.0f, 700.0f, 100.0f);
                    sawRam |= machine.state == mmu::EnemyState::Ram;
                    sawAttack3 |= machine.state == mmu::EnemyState::Attack3;
                    sawPillar |= machine.state == mmu::EnemyState::PillarDrop;
                    sawLightning |= machine.state == mmu::EnemyState::Lightning;
                    sawCrush |= machine.state == mmu::EnemyState::Crush;
                    if (machine.state == mmu::EnemyState::Attack3)
                        sawThreeShots |= machine.shotsFired == 3;
                    if (machine.state == mmu::EnemyState::Lightning)
                        maxStrikes = std::max(maxStrikes, machine.shotsFired);
                    for (const mmu::SpawnedObjectEvent& event : machine.spawnEvents) {
                        if (event.kind == mmu::SpawnedObjectKind::WilyMachinePillar) {
                            if (event.event == 0x0fu) {
                                ++dropCues;
                                if (!haveDropCue) { firstDropCue = event; haveDropCue = true; }
                            } else if (event.event == 0x20u) {
                                ++followCues;
                                if (!haveFollowCue) { firstFollowCue = event; haveFollowCue = true; }
                            } else {
                                ++pillarSpawns;
                                pillarDamageOk &= std::abs(event.damage - 6.0f) < 0.01f;
                                if (!havePillar) { firstPillar = event; havePillar = true; }
                            }
                        } else if (event.kind == mmu::SpawnedObjectKind::WilyMachineShot) {
                            if (shotsSpawned < 3) shotDelays[shotsSpawned] = event.delay;
                            shotDamageOk &= std::abs(event.damage - 4.0f) < 0.01f;
                            volleyFromGotoAttack3 |= machine.state == mmu::EnemyState::GotoAttack3;
                            ++shotsSpawned;
                            if (!haveShot) { firstShot = event; haveShot = true; }
                        } else if (event.kind == mmu::SpawnedObjectKind::WilyMachineLightning) {
                            boltDamageOk &= std::abs(event.damage - 6.0f) < 0.01f &&
                                            std::abs(event.lifetime - 1.6f) < 0.01f;
                            ++boltsSpawned;
                            if (!haveBolt) { firstBolt = event; haveBolt = true; }
                        }
                    }
                }
                check("wily machine attack cycle",
                      sawRam && sawAttack3 && sawPillar && sawLightning && sawCrush &&
                      machine.phase >= 4);
                check("wily machine 3-shot volley on GotoAttack3 enter",
                      sawThreeShots && volleyFromGotoAttack3 && shotsSpawned >= 3);
                check("wily machine shot delays 1/2/3s",
                      shotDelays[0] == 1.0f && shotDelays[1] == 2.0f && shotDelays[2] == 3.0f);
                check("wily machine 2-strike lightning", maxStrikes == 2 && boltsSpawned >= 2);
                check("wily machine spawns one persistent pillar", pillarSpawns == 1);
                check("wily machine drops and releases pillar",
                      dropCues > 0 && followCues > 0 && haveDropCue && haveFollowCue);
                check("wily machine projectile damage",
                      shotDamageOk && boltDamageOk && pillarDamageOk);

                mmu::World projectileSim;
                if (havePillar) projectileSim.spawnProjectile(firstPillar);
                if (haveShot) projectileSim.spawnProjectile(firstShot);
                if (haveBolt) projectileSim.spawnProjectile(firstBolt);
                check("world instantiates WilyMachine projectiles",
                      projectileSim.projectileCount() == 3);
                if (haveShot) {
                    const float startX = projectileSim.objects()[1].x;
                    const float startY = projectileSim.objects()[1].y;
                    for (int i = 0; i < 180; ++i)
                        projectileSim.step(1.0f / 60.0f, mmu::PlayerInput{});
                    const mmu::WorldObject& shot = projectileSim.objects()[1];
                    const float speed = std::sqrt(shot.vx * shot.vx + shot.vy * shot.vy);
                    check("WilyMachineShot homes at speed 250",
                          (std::abs(shot.x - startX) > 1.0f ||
                           std::abs(shot.y - startY) > 1.0f) &&
                          speed > 240.0f && speed < 260.0f);
                } else {
                    check("WilyMachineShot homes at speed 250", false);
                }

                // Cue routing keeps the pillar across attack cycles.
                mmu::World cueSim;
                if (havePillar) cueSim.spawnProjectile(firstPillar);
                if (haveDropCue) cueSim.routeProjectileCue(firstDropCue);
                check("pillar 0xf cue enters Drop", cueSim.objects()[0].projectilePhase == 1);
                if (haveFollowCue) cueSim.routeProjectileCue(firstFollowCue);
                check("pillar 0x20 cue returns to Follow",
                      cueSim.objects()[0].projectilePhase == 0);

                // A dropped pillar fires exactly three 230px/s blocks.
                mmu::World blockSim;
                if (havePillar) blockSim.spawnProjectile(firstPillar);
                if (haveDropCue) blockSim.routeProjectileCue(firstDropCue);
                u32 blocksSpawned = 0;
                bool blockDamageOk = true, blockSpeedOk = false;
                for (int i = 0; i < 420; ++i) {
                    blockSim.step(1.0f / 60.0f, mmu::PlayerInput{});
                    blocksSpawned = 0;
                    for (const mmu::WorldObject& object : blockSim.objects())
                        if (object.projectile == mmu::SpawnedObjectKind::WilyMachineBlock) {
                            ++blocksSpawned;
                            blockDamageOk &= std::abs(object.damage - 4.0f) < 0.01f;
                        }
                    if (!blockSpeedOk && blocksSpawned > 0) {
                        const mmu::WorldObject& block = blockSim.objects()[1];
                        const float speed = std::sqrt(block.vx * block.vx + block.vy * block.vy);
                        blockSpeedOk = speed > 225.0f && speed < 235.0f;
                    }
                    if (blocksSpawned >= 3) break;
                }
                check("pillar fires three 4-damage blocks", blocksSpawned >= 3 && blockDamageOk);
                check("WilyMachineBlock flies at speed 230", blockSpeedOk);

                // WilyCapsule vtable 0x692e94: FlyUp / Wait / FlyDown are
                // position-conditional, with a 5.0s countdown in Wait.
                mmu::EnemyController capsule;
                capsule.reset(mmu::EnemyKind::WilyCapsule, 800.0f, 100.0f);
                check("wily capsule initial FlyUp", capsule.state == mmu::EnemyState::FlyUp);
                bool sawWait = false, sawFlyDown = false;
                for (int i = 0; i < 600; ++i) {
                    capsule.update(1.0f / 60.0f, 800.0f, 100.0f);
                    sawWait |= capsule.state == mmu::EnemyState::Wait;
                    sawFlyDown |= capsule.state == mmu::EnemyState::FlyDown;
                }
                check("wily capsule FlyUp -> Wait -> FlyDown", sawWait && sawFlyDown);

                mmu::World contactSim;
                contactSim.loadScene(state.pak, state.pak.scenes().front(), state.typeImages);
                const mmu::EnemyController* contactEnemy = nullptr;
                for (const mmu::EnemyController& enemy : contactSim.enemies()) {
                    if (enemy.contactDamage) { contactEnemy = &enemy; break; }
                }
                if (contactEnemy) {
                    contactSim.controller().x = contactEnemy->x;
                    contactSim.controller().y = contactEnemy->y - 20.0f;
                    contactSim.controller().invincible = false;
                    contactSim.step(1.0f / 60.0f, mmu::PlayerInput{});
                    std::snprintf(line, sizeof(line),
                                  "  enemy contact: hp=%.0f state=%s controller=(%.0f,%.0f)",
                                  contactSim.controller().health,
                                  mmu::playerStateName(contactSim.controller().state),
                                  contactSim.controller().x, contactSim.controller().y);
                    check("enemy contact damage",
                          contactSim.controller().health < 100.0f &&
                          contactSim.controller().state == mmu::PlayerState::Hurt);
                } else {
                    std::snprintf(line, sizeof(line), "  enemy contact: no contact enemy in scene");
                    check("enemy contact damage", false);
                }
                state.console.log(line);
            }
            for (std::size_t i = 0; i < state.pak.entries().size() && i < 3; ++i) {
                const mmu::PackedEntry& e = state.pak.entries()[i];
                std::snprintf(line, sizeof(line), "  %zu: %ux%u bpp=%u name=%.*s",
                              i, e.width, e.height, e.bitsPerPixel,
                              static_cast<int>(e.name.size()), e.name.data());
                state.console.log(line);
            }
        }
        std::snprintf(line, sizeof(line), "selftest: %d/%d checks passed",
                      selftestChecks - selftestFailures, selftestChecks);
        state.console.log(line);
        const char* files[] = {"misc.mmu", "levels.mmu", "cutscenedefs.mmu",
                               "guns.mmu", "menus.mmu", "cutscenes.mmu",
                               "gameobjects.mmu", "levelart.mmu"};
        for (const char* file : files) {
            loadAssetFile(state, file);
            logStats(state);
        }
        for (const std::string& line : state.console.logLines())
            std::printf("%s\n", line.c_str());
        return (selftestLoaded && selftestFailures == 0) ? 0 : 1;
    }

    // ImGui's SDL2 backend receives finger input as touch-tagged mouse events.
    // This is what makes the 2s2h-style large widgets usable on the Switch
    // touchscreen; set the hints before SDL_Init so the mapping is active from
    // the first event.
#ifdef SDL_HINT_TOUCH_MOUSE_EVENTS
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
#endif
#ifdef SDL_HINT_MOUSE_TOUCH_EVENTS
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const InitialWindowSize initialSize = chooseInitialWindowSize();
    SDL_Window* window = SDL_CreateWindow("MMU Recomp - DEBUG HARNESS (not the game)",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          initialSize.width, initialSize.height,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    g_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    TextureCache textureCache(g_renderer);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    int outputWidth = 0, outputHeight = 0;
    SDL_GetRendererOutputSize(g_renderer, &outputWidth, &outputHeight);
    state.outputWidth = outputWidth;
    state.outputHeight = outputHeight;
    state.uiState.windowLayoutPending = kWindowAll;
    state.ui = makeUiProfile(forceTouch, forceDesktop, outputHeight);
    applyUiProfile(state.ui);
    ImGui_ImplSDL2_InitForSDLRenderer(window, g_renderer);
    ImGui_ImplSDLRenderer2_Init(g_renderer);
#if defined(__SWITCH__) || defined(__SWITCH2__)
    switchui::initKeyboard();
#endif
    state.console.log(std::string("ImGui profile: ") + state.ui.name() +
                      " style=" + std::to_string(state.ui.styleScale).substr(0, 3) +
                      " output=" + std::to_string(outputWidth) + "x" +
                      std::to_string(outputHeight));

    renderSplash(g_renderer, 0.08f, "Initializing audio device...");
    if (state.audio.open()) state.console.log("Audio device opened");
    else state.console.log(std::string("Audio unavailable: ") + SDL_GetError());
    state.console.log("MMU debug harness - asset pipeline skeleton (NOT THE GAME)");
    state.console.log("Asset directory: " + state.assetDir);
    state.console.log("Type 'help' for commands. Try 'pak' or 'load misc.mmu'.");

    // Present this frame before the blocking archive read so the bar stays on
    // screen while the pak is parsed.
    renderSplash(g_renderer, 0.18f, "Loading assets.pak...");
    GameRuntime runtime;

    // Prefer the baked archive. It is the only asset format needed at runtime.
    if (loadPackedArchive(state, "assets.pak")) {
        state.typeImages = mmu::resolveTypeImages(state.pak);
        if (presetLevel) {
            int found = -1;
            for (std::size_t i = 0; i < state.pak.scenes().size(); ++i) {
                if (state.pak.scenes()[i].key == presetLevelKey) {
                    found = static_cast<int>(i);
                    break;
                }
            }
            if (found < 0 && presetLevelKey < state.pak.scenes().size())
                found = static_cast<int>(presetLevelKey);
            if (found >= 0) {
                state.levelViewer.selected = found;
                state.console.log("Preset level: scene #" + std::to_string(found));
            } else {
                state.console.log("Preset level not found: " + std::to_string(presetLevelKey));
            }
        }
        if (presetPlay) {
            const int found = findSceneIndex(state.pak, std::to_string(presetPlayKey));
            if (found >= 0) {
                runtime.startScene(state, found);
            } else {
                state.console.log("Preset world not found: " + std::to_string(presetPlayKey));
            }
        }
        if (presetSprite) {
            const int index = findSpriteIndex(state.pak, presetKey);
            if (index >= 0) {
                state.viewer.selected = index;
                state.console.log("Preset sprite: " + std::to_string(presetKey));
            } else {
                state.console.log("Preset sprite not found: " + std::to_string(presetKey));
            }
        }
        for (const mmu::PackedEntry& entry : state.pak.entries()) {
            if (entry.type == mmu::PackedEntryType::Image && entry.width && entry.height) {
                uploadPreview(state, g_renderer, describe(entry));
                break;
            }
        }
    } else {
        // Fall back to originals for development; large archives load on demand.
        loadAssetFile(state, "misc.mmu");
        loadAssetFile(state, "levels.mmu");
        loadAssetFile(state, "cutscenedefs.mmu");
        if (!state.assets.standaloneSurfaces().empty())
            uploadPreview(state, g_renderer, describe(state.assets.standaloneSurfaces().front()));
    }
    renderSplash(g_renderer, 0.72f, "Preparing scenes and cutscenes...");

    state.cutsceneRuntime.attach(state.assets);
    // The global manager constructor FUN_0050bbb0 starts the logos cutscene
    // and stores the intro path before any menu is built. Emit that boot
    // evidence once here; the title loop below drains the event.
    state.flowRuntime.emitBoot();
    while (state.flowRuntime.pendingEventCount() != 0) {
        const mmu::FlowEvent boot = state.flowRuntime.pollEvent();
        if (boot.type == mmu::FlowEventType::ManagerBoot) {
            startFlowCutscene(state, boot.cutscenePath);
            char bootLine[320];
            std::snprintf(bootLine, sizeof(bootLine),
                          "boot mask=0x%x pending='%.*s'",
                          boot.bootActivityMask,
                          static_cast<int>(boot.pendingCutscenePath.size()),
                          boot.pendingCutscenePath.empty()
                              ? ""
                              : boot.pendingCutscenePath.data());
            state.console.log(bootLine);
        }
    }
    renderSplash(g_renderer, 0.92f, "Starting flow runtime...");
    for (const std::string& command : execCommands)
        state.console.execute(command);
    if (presetSfx && !state.audio.playSfx(state.pak, presetSfxKey))
        state.console.log("Preset sfx not found: " + std::to_string(presetSfxKey));
    if (presetMusic && !state.audio.playMusic(state.pak, presetMusicKey))
        state.console.log("Preset music not found: " + std::to_string(presetMusicKey));
    renderSplash(g_renderer, 1.0f, "Ready");

    bool showConsole = false;
    bool showInspector = false;
    bool showFlowEvidence = false;
    bool showCutscene = false;
    bool showAudio = false;
    Uint32 lastTicks = SDL_GetTicks();
    const Uint32 startTicks = lastTicks;
    while (!state.quit) {
        const Uint32 now = SDL_GetTicks();
        const float delta = now > lastTicks
            ? (now - lastTicks) / 1000.0f : 0.0f;
        lastTicks = now;
        if (autoQuitSeconds > 0.0f &&
            (now - startTicks) / 1000.0f >= autoQuitSeconds)
            state.quit = true;

        SDL_GetRendererOutputSize(g_renderer, &state.outputWidth, &state.outputHeight);
        runtime.discoverPad();
        pollUiInput(state, runtime.pad);
        runtime.tick(state, delta);
        // Hold the player at the cursor for the whole drag so gravity and the
        // state machine cannot pull it away between motion events.
        if (state.uiState.draggingPlayer && state.world.hasScene())
            state.world.debugPlacePlayer(state.uiState.dragWorldX, state.uiState.dragWorldY);
        state.cutsceneRuntime.update(delta);
        dispatchCutsceneEvents(state);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1 &&
                !event.key.repeat) {
                const bool open = !(showConsole || showInspector || showFlowEvidence ||
                                    showCutscene || showAudio || state.viewer.open ||
                                    state.levelViewer.open || state.gameView.open);
                showConsole = showInspector = showFlowEvidence = showCutscene =
                    showAudio = open;
                state.viewer.open = state.levelViewer.open = state.gameView.open = open;
            }
            handleDebugDrag(state, window, g_renderer, event, io.WantCaptureMouse);
            if (event.type == SDL_QUIT) state.quit = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)) state.quit = true;
        }
#if defined(__SWITCH__) || defined(__SWITCH2__)
        switchui::processTextInput();
#endif

        // The quit warning reads raw A/B/Dpad/left-stick edges itself; leave
        // ImGui nav off for that frame so a press cannot also click a button.
        const ImGuiConfigFlags savedNavFlags = io.ConfigFlags;
        if (state.uiState.quitConfirmOpen) {
            io.ConfigFlags &= ~(ImGuiConfigFlags_NavEnableGamepad |
                                ImGuiConfigFlags_NavEnableKeyboard);
        }

        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        if (state.uiState.menuBarVisible && ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Menu")) {
                ImGui::TextDisabled("DEBUG HARNESS - NOT THE GAME");
                ImGui::Text("ImGui profile: %s", state.ui.name());
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Window Layout")) {
                    state.uiState.windowLayoutPending = kWindowAll;
                    state.console.log("Window layout reset requested");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit Game")) state.uiState.quitConfirmOpen = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug Tools")) {
                if (ImGui::MenuItem("Drag Player (Right Mouse)", nullptr,
                                    &state.uiState.debugDragPlayer) &&
                    !state.uiState.debugDragPlayer)
                    state.uiState.draggingPlayer = false;
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Right-click and drag in the game view to move the player.");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Debug Console", nullptr, &showConsole);
                ImGui::MenuItem("Asset Inspector", nullptr, &showInspector);
                ImGui::MenuItem("Flow Evidence", nullptr, &showFlowEvidence);
                ImGui::MenuItem("Cutscene Runtime", nullptr, &showCutscene);
                ImGui::MenuItem("Sprite Viewer", nullptr, &state.viewer.open);
                ImGui::MenuItem("Level Viewer", nullptr, &state.levelViewer.open);
                ImGui::MenuItem("Harness View", nullptr, &state.gameView.open);
                ImGui::MenuItem("Audio", nullptr, &showAudio);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        if (showConsole) drawConsole(state);
        if (showInspector) drawInspector(state);
        if (showFlowEvidence) drawFlowEvidence(state);
        if (showCutscene) drawCutscene(state, textureCache);
        if (showAudio) drawAudio(state);
        if (state.viewer.open) drawSpriteViewer(state, textureCache, delta);
        if (state.levelViewer.open) drawLevelViewer(state, textureCache);
        if (state.gameView.open) drawGameView(state, textureCache, delta);
        drawGameOverlay(runtime, state);
        drawQuitConfirm(state);

        ImGui::Render();
        io.ConfigFlags = savedNavFlags;
        SDL_RenderSetScale(g_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        renderGame(g_renderer, state, textureCache);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), g_renderer);
        if (state.quit && !screenshotPath.empty()) {
            // Headless visual check: dump the final frame before presenting.
            int w = 0, h = 0;
            SDL_GetRendererOutputSize(g_renderer, &w, &h);
            SDL_Surface* shot = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
            if (shot && SDL_RenderReadPixels(g_renderer, nullptr, SDL_PIXELFORMAT_ARGB8888,
                                             shot->pixels, shot->pitch) == 0 &&
                SDL_SaveBMP(shot, screenshotPath.c_str()) == 0)
                state.console.log("screenshot: " + screenshotPath);
            if (shot) SDL_FreeSurface(shot);
            screenshotPath.clear();
        }
        SDL_RenderPresent(g_renderer);
    }

    if (autoQuitSeconds > 0.0f) {
        for (const std::string& line : state.console.logLines())
            std::printf("%s\n", line.c_str());
        std::fflush(stdout);
    }
    if (state.preview) SDL_DestroyTexture(state.preview);
    textureCache.clear();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
#if defined(__SWITCH__) || defined(__SWITCH2__)
    switchui::shutdown();
#endif
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
