#pragma once

#include "mmu/core/GameFlow.h"
#include "mmu/core/KeyedFlagRuntime.h"
#include "mmu/core/StateDispatcher.h"
#include "mmu/core/Types.h"

#include <array>
#include <cstddef>

namespace mmu {

struct RecoveredLevelSetup;

// ---------------------------------------------------------------------------
// Evidence-driven top-level flow runtime.
//
// This is the first executable layer built on top of the GameFlow catalogue.
// It reconstructs the state/cue behaviour that is directly observable in the
// decompiled menu callbacks. It deliberately does NOT decide which screen or
// level a terminal state leads to: those destinations are emitted as events so
// a real front-end can resolve them from decoded save/menu data later.
//
// The timers are reconstruction knobs, not frame-accurate MMU.exe constants.
// See docs/gameplay_reconstruction.md for the evidence and current limits.
// ---------------------------------------------------------------------------

struct FlowInput {
    bool confirm = false; // rising edge: confirm / jump / shoot
    bool cancel = false;  // rising edge: cancel / menu back
    bool up = false;      // rising edge
    bool down = false;    // rising edge
    bool left = false;    // rising edge
    bool right = false;   // rising edge
};

// Manager bytes read by the constructor conditions at 0x4032e0 (+0x19),
// 0x403300 (+0x1a) and 0x403cc0 (+0x59). They select alternate menu item
// tables; the offsets are kept neutral because their gameplay meaning is not
// established by the recovered code.
struct FlowMenuFlags {
    bool flag0x19 = false;
    bool flag0x1a = false;
    bool flag0x59 = false;
    // Action 0x06 calls GetSubItemsCount on the separate object at 0x6c55a4.
    // The decompiler exposes the raw count (< 2 enters the cutscene branch),
    // but not the gameplay meaning of the items.
    int subItemCount = 2;
    // FUN_0061dca0() returns the current boss count (4 or 8 in the recovered
    // branches). The manager bytes written by 0x430a10/0x430a50 before the
    // four/eight-boss cutscenes suppress replaying them in action 0x06.
    int bossCount = 0;
    bool fourBossesSeen = false;
    bool eightBossesSeen = false;
    // The action-0x06 branch also rejects manager field +0x3c == 9. The value
    // shares an offset with the continue-stage id elsewhere, so the raw
    // predicate is supplied directly instead of reusing that field.
    bool previousActionIs9 = false;
    // Selects the MenuInGame item table. 0x511350 computes the raw predicate
    // FUN_0061daf0(DAT_006c20f4[stageId]) for stage ids 1..9; true builds the
    // 0x5081a0 table (which has EXIT STAGE), false builds 0x508340 (no EXIT
    // STAGE). The predicate reads a progress byte whose save-data source is
    // not recovered, so the harness supplies the result directly.
    bool pauseExitStageVariant = false;
    // Raw byte at this+0xba8 of the MenuStageSelect object. FUN_004ff460
    // toggles it after the this+0xbac timer when the DAT_006c55a4 subobject
    // flags test with mask 0xf; FUN_004ffc80 branches on it for the center
    // grid cell. The initial value is not written by the recovered code.
    bool stageSelectGridFlag0xba8 = false;
    // Manual override for FUN_00441d90(id) at the start of FUN_00511350. Bit N
    // models id N. It is kept for deterministic harness injection; normal
    // reconstruction uses KeyedFlagRuntime::query(key, kRising) instead.
    u64 completionFlags = 0;
};

struct FlowTimings {
    float flash = 0.60f;
    float fade = 0.80f;
    float confirm = 0.40f;
    float info = 1.20f;
    float titleIdle = 6.00f;
};

enum class FlowEventType : u8 {
    ScreenChanged,
    StateChanged,
    Cue,
    LevelCue,
    ScreenChangeRequested,
    StageSelected,
    StageCompleted,
    ManagerBoot,
    LevelSetupApplied,
    ActivityMaskChanged,
    SlotSelected,
    ResumeRequested,
    MenuAction,
};

struct FlowEvent {
    FlowEventType type = FlowEventType::MenuAction;
    FlowScreen screen = FlowScreen::TitleMenu;
    FlowState state = FlowState::Wait;
    FlowCue cue = FlowCue::Select;
    FlowLevelCue levelCue = FlowLevelCue::Victory;
    int value = 0; // selection index where applicable

    // Set for MenuAction events whose row has a recovered action code and
    // terminal destination (docs/menu_items.txt, docs/menu_actions.txt).
    u32 actionCode = 0;
    u32 menuSupplierVa = 0; // lazy item-table supplier used by this row
    FlowScreen destination = FlowScreen::TitleMenu;
    bool hasDestination = false;
    std::string_view label;

    // For action 0x0a, the saved stage id held in manager field +0x3c and the
    // level class constructed by FUN_0050e610 when that id is recovered.
    u32 continueStageId = 0;
    u32 continueConstructorVa = 0;
    std::string_view continueStageClass;

    // For MenuAction events whose dispatcher case plays a hard-coded cutscene
    // (actions 0x1e/0x1f and the cutscene branches of 0x06) and for the
    // StageCompleted cutscene prelude of FUN_00511350; the literal .xcut path
    // built by the matching branch.
    std::string_view cutscenePath;

    // FUN_00511350 completion cutscene evidence. completionFlagId is the raw
    // FUN_00441d90 argument that fired; completionManagerState is the raw
    // manager value for the 0x40/0x100/0x200 special cases; managerScreensCleared
    // records the FUN_00507fd0(1) call made by the flag branches. The three
    // write arrays mirror literal manager dword writes verified in those
    // branches (byte offset and value).
    u32 completionFlagId = 0;
    u8 keyedFlagBits = 0;
    u32 completionManagerState = 0;
    bool managerScreensCleared = false;
    u8 completionWriteCount = 0;
    u8 completionWriteOffsets[2] = {0, 0};
    u32 completionWriteValues[2] = {0, 0};

    // Raw helper-call evidence for manager state 0x100. FUN_004308c0 copies the
    // chosen global string into manager +0x40 (completionPendingPath) and sets
    // byte +0x60 to 1. FUN_004308a0 clears two fields on the object held at
    // global 0x6c55f8; the arrays give offset, access size and literal value.
    u32 completionPendingStringVa = 0;
    std::string_view completionPendingPath;
    u8 completionPendingOffset = 0;
    u8 completionPendingFlagOffset = 0;
    u8 completionPendingFlagValue = 0;
    u32 completionClearGlobalVa = 0;
    u8 completionClearWriteCount = 0;
    u16 completionClearWriteOffsets[2] = {0, 0};
    u8 completionClearWriteSizes[2] = {0, 0};
    u32 completionClearWriteValues[2] = {0, 0};

    // FUN_0050bbb0 manager-constructor evidence. cutscenePath holds the path
    // started immediately; pendingCutscenePath is stored for later playback.
    u32 bootActivityMask = 0;
    std::string_view pendingCutscenePath;
    u8 bootWriteCount = 0;
    u8 bootWriteOffsets[3] = {0, 0, 0};
    u32 bootWriteValues[3] = {0, 0, 0};

    // Set when a recovered level-constructor setup row is applied. The pointer
    // refers to static data in LevelSetupTable.h; no field is reinterpreted.
    const RecoveredLevelSetup* levelSetup = nullptr;

    // Raw FUN_00507e90(mask) call evidence. The harness applies one of the
    // recovered call sites through GameFlowRuntime::applyActivityMaskCall();
    // owner and evidence strings identify the site without assigning a
    // gameplay meaning to the mask.
    u32 activityMaskValue = 0;
    u32 activityMaskPrevious = 0; // manager +4 before FUN_00507e90 writes +0
    u32 activityMaskCallVa = 0;
    u32 activityMaskFunctionVa = 0;
    std::string_view activityMaskOwner;
    std::string_view activityMaskEvidence;

    // Action 0x06 first clears the manager string at +0x10 and sets byte +0x18.
    // Its cutscene branches then write phase 8 to +0x08 before returning early.
    bool managerFlag0x18 = false;
    bool hasManagerFlag0x18 = false;
    u32 managerStagePhase = 0;
    bool hasManagerStagePhase = false;
    // The fallback branch also clears manager +0x3c before building
    // MenuStageSelect. Kept separate from continueStageId, which models the
    // saved id used by action 0x0a.
    bool managerStageIdCleared = false;

    // Manager fields mirrored from FUN_0050f7d0 and the concrete menu slots.
    // +0x2c is the phase set by 0x402790; +0x34 is an opaque value forwarded
    // by generic action 0x41 (no confirmed writer in MMU.exe). +0x5d is the
    // quit flag set by action 0x10 and read by the main loop at 0x5e0169.
    u32 managerPhase = 0;
    u32 managerOpaque = 0;
    bool hasManagerPhase = false;
    bool exitGame = false;

    // CharacterSelectMenu actions 0x48/0x49: 0 = MEGA MAN, 1 = Z-PROTOTYPE.
    // -1 when the event is not a character selection. The local timer callback
    // at 0x617e10 also calls 0x61f5e0(U) before selecting the character.
    int character = -1;

    // StageCompleted represents the loader-id routing at the end of
    // FUN_00511350, not a guessed "player beat the boss" flag. The id is the
    // raw manager value; the action/phase/cue are the literal effects of the
    // matching branch in that function.
    u32 completedStageId = 0;
    u32 completedActionCode = 0;
    u32 completedManagerPhase = 0;
    std::string_view completionCue;
};

// The initial state for each recovered screen, taken from the slot that calls
// setState() with a literal in docs/game_flow.txt. Screens with no recovered
// literal fall back to Wait and remain data-only until more callbacks decode.
FlowState flowInitialState(FlowScreen screen);

// True for states whose decompiled handlers advance with dt and then exit the
// screen/state. These are reconstruction hints, not MMU.exe enum names.
bool flowStateIsExit(FlowState state);

class GameFlowRuntime {
public:
    static constexpr std::size_t kEventCapacity = 32;

    GameFlowRuntime() { reset(); }

    void reset(FlowScreen first = FlowScreen::TitleMenu);

    // Advance timers and process one frame of rising-edge input.
    void step(float dt, const FlowInput& input);

    void setScreen(FlowScreen next);
    u32 menuSupplier() const { return menuSupplier_; }
    void setMenuSupplier(u32 supplierVa);
    FlowMenuFlags menuFlags() const { return menuFlags_; }
    void setMenuFlags(const FlowMenuFlags& flags);
    // Convenience for the raw completion-flag bitmask in FlowMenuFlags.
    u64 completionFlags() const { return menuFlags_.completionFlags; }
    void setCompletionFlags(u64 flags) { menuFlags_.completionFlags = flags; }
    KeyedFlagRuntime& keyedFlags() { return keyedFlags_; }
    const KeyedFlagRuntime& keyedFlags() const { return keyedFlags_; }
    void evaluateKeyedFlags(const u8* keyboard, std::size_t keyboardCount,
                            const u8* manager, std::size_t managerCount) {
        keyedFlags_.evaluate(keyboard, keyboardCount, manager, managerCount);
    }
    void setKeyedFlagCondition(u32 key, bool value) {
        keyedFlags_.setConditionOverride(key, value);
    }
    // Mirror FUN_00507e90(mask). sourceCallVa, when it matches a recovered
    // direct call site, tags the emitted ActivityMaskChanged event with that
    // site's raw evidence.
    void setBootActivityMask(u32 mask) { keyedFlags_.setBootActivityMask(mask); }
    void setActivityMask(u32 mask, u32 sourceCallVa = 0);
    bool applyActivityMaskCall(u32 callVa);
    // Evaluate one recovered FUN_00507ec0 test against the current mask.
    // Dynamic tests need the caller-held value supplied explicitly.
    bool evaluateActivityMaskRead(u32 callVa, u32 dynamicMask = 0) const;
    u32 continueStageId() const { return continueStageId_; }
    void setContinueStageId(u32 stageId) { continueStageId_ = stageId; }
    // Emit the literal setup shared by the recovered level constructor for a
    // saved stage id or constructor VA. No transition or cue playback occurs.
    bool applyLevelSetup(u32 stageId);
    bool applyLevelSetupConstructor(u32 constructorVa);
    const RecoveredLevelSetup* levelSetup() const { return levelSetup_; }
    u32 managerPhase() const { return managerPhase_; }
    void setManagerPhase(u32 phase) { managerPhase_ = phase; }
    u32 managerOpaque() const { return managerOpaque_; }
    void setManagerOpaque(u32 value) { managerOpaque_ = value; }
    u32 managerStagePhase() const { return managerStagePhase_; }
    void setManagerStagePhase(u32 phase) { managerStagePhase_ = phase; }
    bool exitRequested() const { return exitRequested_; }
    void setExitRequested(bool value) { exitRequested_ = value; }
    void setState(FlowState next);
    void postCue(FlowCue cue);
    void postLevelCue(FlowLevelCue cue);
    // Mirrors the loader-id routing at the end of FUN_00511350. Emits a
    // StageCompleted event with the recovered action, manager phase and cue.
    void completeStage(u32 loaderId);
    // Mirrors FUN_0050bbb0, the constructor of the global manager at 0x6c55a0:
    // initial activity mask, boot cutscene paths and literal manager writes.
    void emitBoot();
    void setLevelPhase(FlowLevelPhase phase);

    // Shop/purchase failure states recovered from 0x4f3de0 and 0x4f3ee0.
    void failPurchase(bool tooManyItems);

    FlowScreen screen() const { return screen_; }
    FlowState state() const { return state_; }
    FlowState previousState() const { return previousState_; }
    FlowCue lastCue() const { return lastCue_; }
    FlowLevelCue lastLevelCue() const { return lastLevelCue_; }
    FlowLevelPhase levelPhase() const { return levelPhase_; }
    const NamedStateDispatcher& dispatcher() const { return dispatcher_; }
    float time() const { return time_; }
    float stateTime() const { return stateTime_; }
    int selection() const { return selection_; }
    // Raw this+0xbb0 value computed by FUN_004ffc80 for the current
    // MenuStageSelect grid cell, using the raw this+0xba8 flag and the
    // DAT_006c55a4 sub-item count from FlowMenuFlags. Returns -1 for a cell
    // the original code does not assign.
    int stageSelectStage() const;
    bool running() const { return running_; }

    const FlowTimings& timings() const { return timings_; }
    void setTimings(const FlowTimings& timings) { timings_ = timings; }

    std::size_t pendingEventCount() const { return eventCount_; }
    FlowEvent pollEvent();
    void clearEvents();

private:
    void emit(FlowEvent event);
    void emitSimple(FlowEventType type, int value = 0);
    void emitMenuAction(int index);
    void stepTitle(const FlowInput& input);
    void stepMainMenu(const FlowInput& input);
    void stepStageSelect(const FlowInput& input);
    void moveStageSelection(const FlowInput& input);
    void stepOptions(const FlowInput& input);
    void stepMenuInGame(const FlowInput& input);
    void stepPause(const FlowInput& input);
    void stepShop(const FlowInput& input);
    void stepLoadMenu(const FlowInput& input);
    void stepSimpleMenu(const FlowInput& input);
    void moveSelection(const FlowInput& input, int count, bool horizontal = false);

    FlowScreen screen_ = FlowScreen::TitleMenu;
    u32 menuSupplier_ = 0;
    FlowMenuFlags menuFlags_{};
    KeyedFlagRuntime keyedFlags_{};
    u32 continueStageId_ = 0;
    const RecoveredLevelSetup* levelSetup_ = nullptr;
    u32 managerPhase_ = 0;
    u32 managerOpaque_ = 0;
    u32 managerStagePhase_ = 0;
    bool exitRequested_ = false;
    FlowState state_ = FlowState::Wait;
    FlowState previousState_ = FlowState::Wait;
    FlowCue lastCue_ = FlowCue::Select;
    FlowLevelCue lastLevelCue_ = FlowLevelCue::Victory;
    FlowLevelPhase levelPhase_ = FlowLevelPhase::Active;
    float time_ = 0.0f;
    float stateTime_ = 0.0f;
    int selection_ = 0;
    bool running_ = true;
    FlowTimings timings_{};
    FlowInput previousInput_{};
    NamedStateDispatcher dispatcher_{};
    std::array<FlowEvent, kEventCapacity> events_{};
    std::size_t eventHead_ = 0;
    std::size_t eventCount_ = 0;
};

} // namespace mmu
