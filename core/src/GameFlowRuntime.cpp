#include "mmu/core/GameFlowRuntime.h"

#include "mmu/core/ActivityMaskReaderTable.h"
#include "mmu/core/ActivityMaskTable.h"
#include "mmu/core/BootCutsceneTable.h"
#include "mmu/core/CompletionCutsceneTable.h"
#include "mmu/core/ContinueStageTable.h"
#include "mmu/core/FlowEvidence.h"
#include "mmu/core/LevelSetupTable.h"
#include "mmu/core/MenuActionTable.h"
#include "mmu/core/MenuItemTables.h"
#include "mmu/core/StageSelectGrid.h"

#include <algorithm>

namespace mmu {

namespace {

// The reconstructed runtime only uses menu states that were registered by the
// original executable. Mirroring them through NamedStateDispatcher keeps the
// original hash and callback-order semantics visible to the debug tools.
void registerFlowStates(NamedStateDispatcher& dispatcher) {
    for (int i = 0; i < static_cast<int>(FlowState::Count); ++i) {
        const FlowState state = static_cast<FlowState>(i);
        dispatcher.registerState(flowStateName(state));
    }
}

// The initial MainMenu item table is the first supplier used by the menu
// manager (docs/menu_items.txt). Its ten slots are fixed; blank rows are
// skipped by the selection loop in 0x503550.
constexpr u32 kInitialMainMenuSupplier = 0x508EA0u;

// Each constructor's recovered supplier call selects a table from manager
// bytes +0x19, +0x1a or +0x59 through 0x4032e0/0x403300/0x403cc0. The
// unflagged branch is the default; alternate tables can also be forced with
// setMenuSupplier().
u32 defaultMenuSupplier(FlowScreen screen, const FlowMenuFlags& flags) {
    const bool expanded = flags.flag0x19 || flags.flag0x1a;
    switch (screen) {
    case FlowScreen::MainMenu:
        return kInitialMainMenuSupplier;
    case FlowScreen::OptionsMenu:
        return 0x4FB700u;
    case FlowScreen::MenuInGame:
        return flags.pauseExitStageVariant ? 0x5081A0u : 0x508340u;
    case FlowScreen::BusterMenu:
        return 0x618C20u;
    case FlowScreen::CharacterSelectMenu:
        return 0x617C70u;
    case FlowScreen::NewGameMenu:
        return flags.flag0x59 ? 0x4FE210u : 0x4FE3B0u;
    case FlowScreen::EndlessAttackMenu:
        return expanded ? 0x6166A0u : 0x616840u;
    case FlowScreen::EndlessBossAttackMenu:
        return expanded ? 0x6151F0u : 0x615390u;
    case FlowScreen::GauntletMenu:
        return expanded ? 0x613970u : 0x613B10u;
    case FlowScreen::SpecialMenu:
        return 0x5DA920u;
    case FlowScreen::SuperRobotMastersMenuA:
        return 0x5D9360u;
    case FlowScreen::SuperRobotMastersMenuB:
        return 0x5D79E0u;
    case FlowScreen::SuperRobotMastersMenuC:
        return 0x5D6070u;
    case FlowScreen::SuperRobotMastersMenuD:
        return 0x5D4AA0u;
    case FlowScreen::WhirlpoolManStageStartMenu:
        return 0x5B8A80u;
    default:
        return 0;
    }
}

bool hasMenuSupplier(u32 supplierVa) {
    if (supplierVa == 0) return false;
    for (const RecoveredMenuItem& item : kRecoveredMenuItems)
        if (item.supplierVa == supplierVa) return true;
    return false;
}

int menuSlotCount(FlowScreen screen, u32 supplierVa) {
    int count = 0;
    for (const RecoveredMenuItem& item : kRecoveredMenuItems) {
        if (item.className != flowScreenName(screen) ||
            item.supplierVa != supplierVa)
            continue;
        if (static_cast<int>(item.index) + 1 > count)
            count = static_cast<int>(item.index) + 1;
    }
    return count;
}

const RecoveredMenuItem* menuItem(FlowScreen screen, int index, u32 supplierVa) {
    if (index < 0) return nullptr;
    return findMenuItem(flowScreenName(screen), static_cast<u16>(index),
                        supplierVa);
}

// Walk the fixed array until a non-blank label is found, matching the
// `empty()` loop around 0x5035a2/0x503622.
int nextVisibleMenuItem(FlowScreen screen, int current, int step,
                        u32 supplierVa, int count) {
    if (count <= 0) return current;
    for (int i = 0; i < count; ++i) {
        current = (current + step + count) % count;
        const RecoveredMenuItem* item = menuItem(screen, current, supplierVa);
        if (item && item->actionCode != 0) return current;
    }
    return current;
}

// Used only when no constructor registration was recovered for a screen.
FlowState flowInitialStateFallback(FlowScreen screen) {
    switch (screen) {
    case FlowScreen::TitleMenu:
        return FlowState::Wait;
    case FlowScreen::MainMenu:
    case FlowScreen::OptionsMenu:
        return FlowState::Select;
    case FlowScreen::MenuStageSelect:
        return FlowState::SelectLevel;
    case FlowScreen::LoadMenu:
    case FlowScreen::PauseMenuStatus:
    case FlowScreen::ShopMenu:
        return FlowState::SelectItem;
    case FlowScreen::NewGameMenu:
    case FlowScreen::BusterMenu:
    case FlowScreen::CharacterSelectMenu:
    case FlowScreen::EndlessAttackMenu:
    case FlowScreen::EndlessBossAttackMenu:
    case FlowScreen::GauntletMenu:
    case FlowScreen::SpecialMenu:
    case FlowScreen::SuperRobotMastersMenuA:
    case FlowScreen::SuperRobotMastersMenuB:
    case FlowScreen::SuperRobotMastersMenuC:
    case FlowScreen::SuperRobotMastersMenuD:
    case FlowScreen::WhirlpoolManStageStartMenu:
        return FlowState::SelectDone;
    case FlowScreen::MenuInGame:
        break;
    }
    return FlowState::Wait;
}

// The 3x3 grid is recovered in tools/analyze_stage_select_grid.py from
// FUN_004ffc80. The eight outer cells are fixed raw writes to this+0xbb0; the
// center cell combines the raw byte this+0xba8 with the item count of the
// object at DAT_006c55a4. No gameplay label is assigned to either input.
int stageForGridCell(int index, bool flag0xba8, int subItemCount) {
    if (index < 0 || index >= 9) return -1;
    return stageSelectGridStage(index % 3, index / 3, flag0xba8, subItemCount);
}

} // namespace

FlowState flowInitialState(FlowScreen screen) {
    // The constructor's final setState call is hard evidence for the initial
    // state (docs/state_graph.txt); fall back to the older setter-only guess
    // for classes whose constructor was not recovered.
    const std::string_view initial = graphInitialState(screen);
    if (!initial.empty()) return flowStateFromName(initial);
    return flowInitialStateFallback(screen);
}

int GameFlowRuntime::stageSelectStage() const {
    return stageForGridCell(selection_, menuFlags_.stageSelectGridFlag0xba8,
                            menuFlags_.subItemCount);
}

bool flowStateIsExit(FlowState state) {
    switch (state) {
    case FlowState::Flash:
    case FlowState::IntroFadeOut:
    case FlowState::IntroSlideOut:
    case FlowState::SlideOut:
        return true;
    default:
        return false;
    }
}

void GameFlowRuntime::reset(FlowScreen first) {
    registerFlowStates(dispatcher_);
    dispatcher_.reset();
    screen_ = first;
    menuSupplier_ = defaultMenuSupplier(first, menuFlags_);
    state_ = flowInitialState(first);
    previousState_ = state_;
    lastCue_ = FlowCue::Select;
    lastLevelCue_ = FlowLevelCue::Victory;
    levelPhase_ = FlowLevelPhase::Active;
    time_ = 0.0f;
    stateTime_ = 0.0f;
    selection_ = 0;
    keyedFlags_.reset();
    // No per-state mask rule is recovered for most screens; return to the
    // harness default instead of retaining a setter applied by a prior test.
    keyedFlags_.setActivityMask(KeyedFlagRuntime::kAllCategories);
    continueStageId_ = 0;
    levelSetup_ = nullptr;
    managerPhase_ = 0;
    managerOpaque_ = 0;
    managerStagePhase_ = 0;
    exitRequested_ = false;
    running_ = true;
    previousInput_ = FlowInput{};
    dispatcher_.setState(flowStateName(state_));
    clearEvents();
}

void GameFlowRuntime::setScreen(FlowScreen next) {
    screen_ = next;
    menuSupplier_ = defaultMenuSupplier(next, menuFlags_);
    state_ = flowInitialState(next);
    previousState_ = state_;
    stateTime_ = 0.0f;
    selection_ = 0;
    running_ = true;
    dispatcher_.setState(flowStateName(state_));
    emitSimple(FlowEventType::ScreenChanged, static_cast<int>(next));
    emitSimple(FlowEventType::StateChanged, static_cast<int>(state_));

    // The pause status menu sets this cue when it is entered (0x4f8a60).
    if (next == FlowScreen::PauseMenuStatus)
        postCue(FlowCue::PauseMenu);
}

void GameFlowRuntime::setMenuSupplier(u32 supplierVa) {
    // 0 restores the default table for the current screen. Unknown suppliers
    // are ignored so a typo cannot silently blank the recovered rows.
    if (supplierVa == 0) supplierVa = defaultMenuSupplier(screen_, menuFlags_);
    if (!hasMenuSupplier(supplierVa)) return;
    menuSupplier_ = supplierVa;
}

void GameFlowRuntime::setMenuFlags(const FlowMenuFlags& flags) {
    menuFlags_ = flags;
    menuSupplier_ = defaultMenuSupplier(screen_, menuFlags_);
}

void GameFlowRuntime::setState(FlowState next) {
    if (state_ == next)
        return;
    previousState_ = state_;
    state_ = next;
    stateTime_ = 0.0f;
    dispatcher_.setState(flowStateName(next));
    emitSimple(FlowEventType::StateChanged, static_cast<int>(next));
}

void GameFlowRuntime::postCue(FlowCue cue) {
    lastCue_ = cue;
    FlowEvent event;
    event.type = FlowEventType::Cue;
    event.screen = screen_;
    event.state = state_;
    event.cue = cue;
    emit(event);
}

void GameFlowRuntime::postLevelCue(FlowLevelCue cue) {
    lastLevelCue_ = cue;
    switch (cue) {
    case FlowLevelCue::Victory:
    case FlowLevelCue::VictoryZ:
        levelPhase_ = FlowLevelPhase::Victory; // 0x53b49f writes phase 2
        break;
    case FlowLevelCue::Death:
        levelPhase_ = FlowLevelPhase::Death; // 0x53b53f/0x53b58e write phase 3
        break;
    case FlowLevelCue::Teleport:
        levelPhase_ = FlowLevelPhase::Exiting; // 0x53b2c5 writes phase 5
        break;
    case FlowLevelCue::Environment:
    case FlowLevelCue::BigExplosion:
        break;
    }
    FlowEvent event;
    event.type = FlowEventType::LevelCue;
    event.screen = screen_;
    event.state = state_;
    event.levelCue = cue;
    event.value = static_cast<int>(cue);
    emit(event);
}

void GameFlowRuntime::setLevelPhase(FlowLevelPhase phase) {
    levelPhase_ = phase;
}

void GameFlowRuntime::setActivityMask(u32 mask, u32 sourceCallVa) {
    keyedFlags_.setActivityMask(mask);
    FlowEvent event;
    event.type = FlowEventType::ActivityMaskChanged;
    event.screen = screen_;
    event.state = state_;
    event.activityMaskValue = mask;
    event.activityMaskPrevious = keyedFlags_.previousActivityMask();
    event.activityMaskCallVa = sourceCallVa;
    const RecoveredActivityMaskCall* site = findActivityMaskCall(sourceCallVa);
    if (site != nullptr) {
        event.activityMaskFunctionVa = site->functionVa;
        event.activityMaskOwner = site->owner;
        event.activityMaskEvidence = site->stateEvidence.empty()
                                         ? site->decompiledStrings
                                         : site->stateEvidence;
    }
    emit(event);
}

bool GameFlowRuntime::applyActivityMaskCall(u32 callVa) {
    const RecoveredActivityMaskCall* site = findActivityMaskCall(callVa);
    if (site == nullptr) return false;
    setActivityMask(site->mask, callVa);
    return true;
}

bool GameFlowRuntime::evaluateActivityMaskRead(u32 callVa, u32 dynamicMask) const {
    const RecoveredActivityMaskRead* read = findActivityMaskRead(callVa);
    if (read == nullptr) return false;
    return mmu::evaluateActivityMaskRead(*read, keyedFlags_.activityMask(),
                                         dynamicMask);
}

bool GameFlowRuntime::applyLevelSetup(u32 stageId) {
    const RecoveredLevelSetup* row = findLevelSetup(stageId);
    if (row == nullptr) return false;
    levelSetup_ = row;
    FlowEvent event;
    event.type = FlowEventType::LevelSetupApplied;
    event.screen = screen_;
    event.state = state_;
    event.levelSetup = row;
    emit(event);
    return true;
}

bool GameFlowRuntime::applyLevelSetupConstructor(u32 constructorVa) {
    const RecoveredLevelSetup* row = findLevelSetupByConstructor(constructorVa);
    if (row == nullptr) return false;
    levelSetup_ = row;
    FlowEvent event;
    event.type = FlowEventType::LevelSetupApplied;
    event.screen = screen_;
    event.state = state_;
    event.levelSetup = row;
    emit(event);
    return true;
}

void GameFlowRuntime::emitBoot() {
    // FUN_0050bbb0 zeroes the manager, sets the initial activity mask read by
    // FUN_00507ec0, starts the logos cutscene, stores the intro path and writes
    // three literal manager dwords. Its only caller constructs the global
    // manager object at 0x6c55a0, so this is one-time boot evidence. Applying
    // the mask here makes later keyed-flag evaluations use the same value that
    // the constructor installs.
    keyedFlags_.setBootActivityMask(kBootActivityMask);
    FlowEvent event;
    event.type = FlowEventType::ManagerBoot;
    event.screen = screen_;
    event.state = state_;
    event.cutscenePath = kBootPlayCutscene;
    event.pendingCutscenePath = kBootPendingCutscene;
    event.bootActivityMask = kBootActivityMask;
    event.bootWriteCount = static_cast<u8>(kBootManagerWriteCount);
    for (std::size_t i = 0; i < kBootManagerWriteCount && i < 3; ++i) {
        event.bootWriteOffsets[i] = kBootManagerWrites[i].offset;
        event.bootWriteValues[i] = kBootManagerWrites[i].value;
    }
    emit(event);
}

void GameFlowRuntime::completeStage(u32 loaderId) {
    // FUN_00511350 dispatches the manager's raw loader id near the end of the
    // function. Each matching helper writes one action to +0x2c, sets manager
    // phase 4, and passes a literal cue string to 0x4026b0. The catch-all
    // 0x50bb10 writes phase 5/9 from saved stage id +0x3c and plays
    // GameOverIntro. This event reports those writes; it does not assert that
    // the id means a successful stage completion.
    FlowEvent event;
    event.type = FlowEventType::StageCompleted;
    event.screen = screen_;
    event.state = state_;
    event.value = static_cast<int>(loaderId);
    event.completedStageId = loaderId;

    // FUN_00511350 starts with a chain of FUN_00441d90(id) checks. A true
    // result tears down the manager screens and starts a hard-coded cutscene,
    // returning before any loader-id routing. FUN_00441d90 tests mask 8, which
    // FUN_00441820 sets as a one-frame rising edge when a key's condition first
    // becomes true. FlowMenuFlags.completionFlags remains a manual override for
    // deterministic harness injection.
    static constexpr u32 kCompletionFlagIds[] = {7u, 8u, 9u, 10u,
                                                 11u, 12u, 13u, 14u};
    for (u32 flagId : kCompletionFlagIds) {
        const bool rising = keyedFlags_.query(flagId, KeyedFlagRuntime::kRising);
        const bool manual =
            (menuFlags_.completionFlags & (u64{1} << flagId)) != 0;
        const RecoveredCompletionCutscene* cutscene =
            (rising || manual) ? findCompletionCutsceneFlag(flagId) : nullptr;
        if (cutscene == nullptr) continue;
        event.cutscenePath = cutscene->path;
        event.completionFlagId = flagId;
        event.keyedFlagBits = keyedFlags_.bits(flagId);
        event.managerScreensCleared = true;
        emit(event);
        return;
    }

    // The earlier flag branches return before this point. Once execution
    // reaches the manager-state checks, FUN_004308a0 clears two fields on the
    // object held at global 0x6c55f8. Carry that raw call evidence on every
    // remaining StageCompleted event.
    event.completionClearGlobalVa = kCompletionClearGlobalVa;
    event.completionClearWriteCount =
        static_cast<u8>(kCompletionClearWriteCount);
    for (std::size_t i = 0;
         i < kCompletionClearWriteCount && i < 2; ++i) {
        event.completionClearWriteOffsets[i] =
            kCompletionClearWrites[i].offset;
        event.completionClearWriteSizes[i] = kCompletionClearWrites[i].size;
        event.completionClearWriteValues[i] = kCompletionClearWrites[i].value;
    }

    // Manager states 0x100 and 0x200 also select a cutscene before the normal
    // routing. 0x100 chooses between two strings with manager byte +0x59.
    const RecoveredCompletionCutscene* stateCutscene =
        findCompletionCutsceneState(loaderId, menuFlags_.flag0x59);
    if (stateCutscene != nullptr) {
        event.cutscenePath = stateCutscene->path;
        event.completionManagerState = loaderId;
        event.completionWriteCount = stateCutscene->writeCount;
        for (u8 i = 0; i < stateCutscene->writeCount && i < 2; ++i) {
            event.completionWriteOffsets[i] = stateCutscene->writeOffsets[i];
            event.completionWriteValues[i] = stateCutscene->writeValues[i];
        }
        if (stateCutscene->managerState == 0x100u) {
            // FUN_004308c0 re-reads the selector and stores the same global
            // string in the manager's pending slot with byte +0x60 set to 1.
            event.completionPendingStringVa =
                stateCutscene->pendingStringVa;
            event.completionPendingPath = stateCutscene->path;
            event.completionPendingOffset = kCompletionPendingStringOffset;
            event.completionPendingFlagOffset = kCompletionPendingFlagOffset;
            event.completionPendingFlagValue = kCompletionPendingFlagValue;
        }
        emit(event);
        return;
    }

    if (loaderId == 0x10u || loaderId == 0x11u) {
        event.completedActionCode = loaderId == 0x10u ? 0x12u : 0x15u;
        event.completedManagerPhase = event.completedActionCode;
        event.completionCue = "MenuThemeB";
    } else if (loaderId == 0x12u) {
        event.completedActionCode = 0x18u;
        event.completedManagerPhase = 0x18u;
        event.completionCue = "MenuThemeB";
    } else if (loaderId == 0x13u) {
        event.completedActionCode = 0x1bu;
        event.completedManagerPhase = 0x1bu;
        event.completionCue = "MenuThemeB";
    } else if ((loaderId >= 0x1eu && loaderId <= 0x24u) ||
               (loaderId >= 0x14u && loaderId <= 0x17u) ||
               (loaderId >= 0x25u && loaderId <= 0x27u)) {
        // 0x510990 and 0x510a10/0x510910 use different Super Robot Masters
        // menus: id 0x1e-0x24 goes to A, the other two ranges to B.
        event.completedActionCode =
            loaderId >= 0x1eu && loaderId <= 0x24u ? 0x20u : 0x21u;
        event.completedManagerPhase = event.completedActionCode;
        event.completionCue = "MenuThemeB";
    } else if (loaderId >= 0x18u && loaderId <= 0x1du) {
        event.completedActionCode = 0x22u;
        event.completedManagerPhase = 0x22u;
        event.completionCue = "MenuThemeB";
    } else {
        // 0x50bb10: saved ids 0xa-0xe write phase 5; everything else phase 9.
        event.completedManagerPhase =
            (continueStageId_ >= 0xau && continueStageId_ <= 0xeu) ? 5u : 9u;
        event.completionCue = "GameOverIntro";
    }
    managerPhase_ = event.completedManagerPhase;
    event.managerPhase = event.completedManagerPhase;
    event.hasManagerPhase = true;
    emit(event);
}

void GameFlowRuntime::failPurchase(bool tooManyItems) {
    postCue(FlowCue::Wrong);
    setState(tooManyItems ? FlowState::TooManyItems : FlowState::NotEnoughBolts);
}

void GameFlowRuntime::emit(FlowEvent event) {
    if (eventCount_ == kEventCapacity) {
        eventHead_ = (eventHead_ + 1) % kEventCapacity;
        --eventCount_;
    }
    events_[(eventHead_ + eventCount_) % kEventCapacity] = event;
    ++eventCount_;
}

void GameFlowRuntime::emitSimple(FlowEventType type, int value) {
    FlowEvent event;
    event.type = type;
    event.screen = screen_;
    event.state = state_;
    event.value = value;
    emit(event);
}

void GameFlowRuntime::emitMenuAction(int index) {
    FlowEvent event;
    event.type = FlowEventType::MenuAction;
    event.screen = screen_;
    event.state = state_;
    event.value = index;

    const RecoveredMenuItem* item = menuItem(screen_, index, menuSupplier_);
    if (item) {
        event.actionCode = item->actionCode;
        event.label = item->label;
        event.menuSupplierVa = item->supplierVa;
    }
    const RecoveredMenuAction* action = findMenuAction(event.actionCode);
    if (action && action->kind == MenuActionKind::Screen &&
        action->screenCount != 0) {
        // FUN_0050f7d0 lists conditional screens with the default branch
        // first. Action 0x47 takes CharacterSelectMenu when either manager
        // byte +0x19 or +0x1a is set (FUN_004033e0/FUN_00403400).
        event.destination = action->screens[0];
        if (event.actionCode == 0x47 && action->screenCount > 1 &&
            (menuFlags_.flag0x19 || menuFlags_.flag0x1a))
            event.destination = action->screens[1];
        event.hasDestination = true;
    }
    if (action && action->cutscenePath[0] != '\0') {
        event.cutscenePath = action->cutscenePath;
    }
    if (action) {
        // Actions 0x06, 0x1e and 0x1f clear the manager string at +0x10 and
        // set byte +0x18 before starting a cutscene (0x50f7d0:215,411,428).
        if (event.actionCode == 0x06 || event.actionCode == 0x1e ||
            event.actionCode == 0x1f) {
            event.hasManagerFlag0x18 = true;
            event.managerFlag0x18 = true;
        }

        switch (event.actionCode) {
        case 0x06: {
            // 0x50f7d0:219-262. The cutscene branches write phase 8 and return
            // early; the fallback clears +0x3c and builds MenuStageSelect.
            const bool canPlayConditional =
                menuFlags_.subItemCount < 2 && !menuFlags_.previousActionIs9 &&
                !menuFlags_.flag0x59;
            if (canPlayConditional && menuFlags_.bossCount == 4 &&
                !menuFlags_.fourBossesSeen &&
                action->conditionalCutsceneCount > 0) {
                event.cutscenePath = action->conditionalCutscenes[0];
                event.managerStagePhase = 8;
                event.hasManagerStagePhase = true;
                event.hasDestination = false;
                managerStagePhase_ = 8;
            } else if (canPlayConditional && menuFlags_.bossCount == 8 &&
                       !menuFlags_.eightBossesSeen &&
                       action->conditionalCutsceneCount > 1) {
                event.cutscenePath = action->conditionalCutscenes[1];
                event.managerStagePhase = 8;
                event.hasManagerStagePhase = true;
                event.hasDestination = false;
                managerStagePhase_ = 8;
            } else {
                // The fallback constructs MenuStageSelect; the conditional
                // cutscene path above is not played in this branch
                // (0x50f7d0:252-261). Clear it so front-ends do not start a
                // cutscene for an event that has a destination screen.
                event.cutscenePath = "";
                event.managerStageIdCleared = true;
            }
            break;
        }
        case 0x10:
            // 0x5100a1 writes byte [M+0x5d]; 0x5e0169 reads it as quit.
            event.exitGame = true;
            exitRequested_ = true;
            break;
        case 0x41:
            // The global case forwards an opaque manager field (0x50f7d0:489).
            // CharacterSelectMenu's local 0x41 case uses literal phase 2.
            event.managerPhase =
                screen_ == FlowScreen::CharacterSelectMenu ? 2 : managerOpaque_;
            event.hasManagerPhase = true;
            managerPhase_ = event.managerPhase;
            break;
        case 0x48:
        case 0x49:
            // CharacterSelectMenu timer 0x617e10: 0x61dc80(U, 0|1) followed by
            // 0x402790(M, 5). 0x48 is MEGA MAN, 0x49 is Z-PROTOTYPE.
            event.character = event.actionCode == 0x48 ? 0 : 1;
            event.managerPhase = 5;
            event.hasManagerPhase = true;
            managerPhase_ = 5;
            break;
        default:
            break;
        }
    }
    if (action && action->kind == MenuActionKind::ContinueGame) {
        // FUN_0050e610 switches on manager field +0x3c and constructs the
        // matching Level... object. The id is raw save data; only recovered
        // ids get a class name here.
        event.continueStageId = continueStageId_;
        const RecoveredContinueStage* stage =
            findContinueStage(continueStageId_);
        if (stage != nullptr) {
            event.continueConstructorVa = stage->constructorVa;
            event.continueStageClass = stage->className;
        }
    }
    emit(event);
}

FlowEvent GameFlowRuntime::pollEvent() {
    if (eventCount_ == 0)
        return FlowEvent{};
    FlowEvent event = events_[eventHead_];
    eventHead_ = (eventHead_ + 1) % kEventCapacity;
    --eventCount_;
    return event;
}

void GameFlowRuntime::clearEvents() {
    eventHead_ = 0;
    eventCount_ = 0;
}

void GameFlowRuntime::moveSelection(const FlowInput& input, int count,
                                    bool horizontal) {
    if (count <= 0)
        return;
    const bool upEdge = input.up && !previousInput_.up;
    const bool downEdge = input.down && !previousInput_.down;
    const bool leftEdge = horizontal && input.left && !previousInput_.left;
    const bool rightEdge = horizontal && input.right && !previousInput_.right;
    if (upEdge || leftEdge) {
        --selection_;
        if (selection_ < 0) selection_ = count - 1;
    }
    if (downEdge || rightEdge) {
        ++selection_;
        if (selection_ >= count) selection_ = 0;
    }
}

void GameFlowRuntime::step(float dt, const FlowInput& input) {
    if (!running_ || dt < 0.0f)
        return;
    time_ += dt;
    stateTime_ += dt;

    switch (screen_) {
    case FlowScreen::TitleMenu:
        stepTitle(input);
        break;
    case FlowScreen::MainMenu:
        stepMainMenu(input);
        break;
    case FlowScreen::MenuStageSelect:
        stepStageSelect(input);
        break;
    case FlowScreen::OptionsMenu:
        stepOptions(input);
        break;
    case FlowScreen::MenuInGame:
        stepMenuInGame(input);
        break;
    case FlowScreen::PauseMenuStatus:
        stepPause(input);
        break;
    case FlowScreen::ShopMenu:
        stepShop(input);
        break;
    case FlowScreen::LoadMenu:
        stepLoadMenu(input);
        break;
    default:
        stepSimpleMenu(input);
        break;
    }

    // 0x4464a0 iterates registered states and fires expired timer callbacks;
    // the portable mirror runs the active state's update hook and the timer
    // pass with all masks active. The original activity-mask source (0x507ec0)
    // is still unresolved, so no per-input mask is inferred here.
    dispatcher_.update();
    dispatcher_.tickTimers(dt);
    previousInput_ = input;
}

void GameFlowRuntime::stepTitle(const FlowInput& input) {
    switch (state_) {
    case FlowState::IntroFadeIn:
        // TitleMenu's constructor selects IntroFadeIn before Wait.
        if (stateTime_ >= timings_.fade) setState(FlowState::Wait);
        break;
    case FlowState::Wait:
        // 0x4f3040 posts Confirm and enters Flash; 0x4f30b0 times out into
        // IntroFadeOut; 0x4f2f70 advances IntroFadeOut.
        if (input.confirm && !previousInput_.confirm) {
            postCue(FlowCue::Confirm);
            setState(FlowState::Flash);
        } else if ((input.cancel && !previousInput_.cancel) ||
                   stateTime_ >= timings_.titleIdle) {
            setState(FlowState::IntroFadeOut);
        }
        break;
    case FlowState::Flash:
        if (stateTime_ >= timings_.flash)
            setState(FlowState::IntroFadeOut);
        break;
    case FlowState::IntroFadeOut:
        if (stateTime_ >= timings_.fade) {
            emitSimple(FlowEventType::ScreenChangeRequested, selection_);
            running_ = false;
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::stepMainMenu(const FlowInput& input) {
    // The ten fixed slots and their action codes come from the recovered
    // MainMenu item table; 0x503550 skips blank labels and branches on +0x1c.
    const int count = menuSlotCount(screen_, menuSupplier_);
    switch (state_) {
    case FlowState::IntroFadeIn:
        if (stateTime_ >= timings_.fade) setState(FlowState::Select);
        break;
    case FlowState::Select:
        if (input.up && !previousInput_.up) {
            selection_ = nextVisibleMenuItem(screen_, selection_, -1,
                                             menuSupplier_, count);
            postCue(FlowCue::Select);
        } else if (input.down && !previousInput_.down) {
            selection_ = nextVisibleMenuItem(screen_, selection_, 1,
                                             menuSupplier_, count);
            postCue(FlowCue::Select);
        } else if (input.confirm && !previousInput_.confirm) {
            const RecoveredMenuItem* item =
                menuItem(screen_, selection_, menuSupplier_);
            if (!item || item->actionCode == 0)
                break;
            postCue(FlowCue::Confirm);
            const MenuItemBehavior behavior =
                static_cast<MenuItemBehavior>(item->behavior);
            setState(behavior == MenuItemBehavior::Confirm
                         ? FlowState::Confirm
                         : FlowState::Flash);
        } else if (input.cancel && !previousInput_.cancel) {
            setState(FlowState::IntroFadeOut);
        }
        break;
    case FlowState::Confirm:
        if (stateTime_ >= timings_.confirm ||
            (input.confirm && !previousInput_.confirm)) {
            emitMenuAction(selection_);
            setState(FlowState::Select);
        }
        break;
    case FlowState::Flash:
        if (stateTime_ >= timings_.flash) {
            emitMenuAction(selection_);
            setState(FlowState::Select);
        }
        break;
    case FlowState::IntroFadeOut:
        if (stateTime_ >= timings_.fade) {
            emitSimple(FlowEventType::ScreenChangeRequested, selection_);
            running_ = false;
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::moveStageSelection(const FlowInput& input) {
    // 0x4ff070 clamps row/col to 0..2; unlike the text menus it does not wrap.
    int row = selection_ / 3;
    int col = selection_ % 3;
    if (input.up && !previousInput_.up) row = std::max(0, row - 1);
    if (input.down && !previousInput_.down) row = std::min(2, row + 1);
    if (input.left && !previousInput_.left) col = std::max(0, col - 1);
    if (input.right && !previousInput_.right) col = std::min(2, col + 1);
    selection_ = row * 3 + col;
}

void GameFlowRuntime::stepStageSelect(const FlowInput& input) {
    // Grid and stage numbers are observed in 0x4ffc80; intro/exit states come
    // from the MenuStageSelect constructor graph.
    switch (state_) {
    case FlowState::IntroSlideIn:
        if (stateTime_ >= timings_.fade) setState(FlowState::SelectLevel);
        break;
    case FlowState::SelectLevel:
        moveStageSelection(input);
        if (input.confirm && !previousInput_.confirm) {
            postCue(FlowCue::Confirm);
            setState(FlowState::Flash);
        } else if (input.cancel && !previousInput_.cancel) {
            setState(FlowState::IntroSlideOut);
        }
        break;
    case FlowState::Flash:
        if (stateTime_ >= timings_.flash) {
            emitSimple(FlowEventType::StageSelected, stageSelectStage());
            setState(FlowState::SelectLevel);
        }
        break;
    case FlowState::IntroSlideOut:
        if (stateTime_ >= timings_.fade) {
            emitSimple(FlowEventType::ScreenChangeRequested, selection_);
            running_ = false;
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::stepOptions(const FlowInput& input) {
    // The OptionsMenu vector has 18 fixed slots. Its two mapping rows are
    // identified by recovered action codes 0x39 and 0x3a, not by index.
    const int count = menuSlotCount(screen_, menuSupplier_);
    switch (state_) {
    case FlowState::IntroFadeIn:
        if (stateTime_ >= timings_.fade) setState(FlowState::Select);
        break;
    case FlowState::Select:
        if (input.up && !previousInput_.up) {
            selection_ = nextVisibleMenuItem(screen_, selection_, -1,
                                             menuSupplier_, count);
            postCue(FlowCue::Select);
        } else if (input.down && !previousInput_.down) {
            selection_ = nextVisibleMenuItem(screen_, selection_, 1,
                                             menuSupplier_, count);
            postCue(FlowCue::Select);
        } else if (input.confirm && !previousInput_.confirm) {
            const RecoveredMenuItem* item =
                menuItem(screen_, selection_, menuSupplier_);
            if (!item || item->actionCode == 0)
                break;
            postCue(FlowCue::Confirm);
            if (item->actionCode == 0x39)
                setState(FlowState::MapKeyboardInputs);
            else if (item->actionCode == 0x3a)
                setState(FlowState::MapJoystickInputs);
            else
                setState(FlowState::Flash);
        } else if (input.cancel && !previousInput_.cancel) {
            setState(FlowState::IntroFadeOut);
        }
        break;
    case FlowState::MapKeyboardInputs:
    case FlowState::MapJoystickInputs:
        if (stateTime_ >= timings_.info ||
            (input.confirm && !previousInput_.confirm) ||
            (input.cancel && !previousInput_.cancel)) {
            emitMenuAction(selection_);
            setState(FlowState::Select);
        }
        break;
    case FlowState::Flash:
        if (stateTime_ >= timings_.flash) {
            emitMenuAction(selection_);
            setState(FlowState::Select);
        }
        break;
    case FlowState::IntroFadeOut:
        if (stateTime_ >= timings_.fade) {
            emitSimple(FlowEventType::ScreenChangeRequested, selection_);
            running_ = false;
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::stepMenuInGame(const FlowInput& input) {
    // MenuInGame is the pause item menu (supplier 0x5081a0). Its recovered rows
    // are RESUME GAME (0x0e), EXIT STAGE (0x0f), TITLE SCREEN (0x01) and
    // EXIT GAME (0x10). PauseMenuStatus is a separate energy-detail screen and
    // has no item supplier of its own.
    const int count = menuSlotCount(screen_, menuSupplier_);
    switch (state_) {
    case FlowState::Wait:
        moveSelection(input, count);
        if (input.confirm && !previousInput_.confirm) {
            const mmu::RecoveredMenuItem* item =
                menuItem(screen_, selection_, menuSupplier_);
            postCue(FlowCue::Confirm);
            setState(item && item->behavior ==
                                  static_cast<u8>(MenuItemBehavior::Confirm)
                                  ? FlowState::Confirm
                                  : FlowState::Flash);
        } else if (input.cancel && !previousInput_.cancel) {
            emitSimple(FlowEventType::ResumeRequested, selection_);
            running_ = false;
        }
        break;
    case FlowState::Flash:
    case FlowState::Confirm:
        if (stateTime_ >= (state_ == FlowState::Flash ? timings_.flash
                                                      : timings_.confirm)) {
            emitMenuAction(selection_);
            setState(FlowState::Wait);
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::stepPause(const FlowInput& input) {
    switch (state_) {
    case FlowState::SlideIn:
        if (stateTime_ >= timings_.fade) setState(FlowState::SelectItem);
        break;
    case FlowState::SelectItem:
        moveSelection(input, 4);
        if (input.confirm && !previousInput_.confirm) {
            setState(FlowState::EnergyFill);
        } else if (input.right && !previousInput_.right) {
            setState(FlowState::WeaponEnergyFill);
        } else if (input.cancel && !previousInput_.cancel) {
            setState(FlowState::SlideOut);
        }
        break;
    case FlowState::EnergyFill:
    case FlowState::WeaponEnergyFill:
        if (stateTime_ >= timings_.info) {
            // PauseMenuStatus has no item supplier; its fill states only time
            // the status animation before returning to the row selection.
            setState(FlowState::SelectItem);
        }
        break;
    case FlowState::SlideOut:
        if (stateTime_ >= timings_.fade) {
            emitSimple(FlowEventType::ResumeRequested, selection_);
            running_ = false;
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::stepShop(const FlowInput& input) {
    switch (state_) {
    case FlowState::SlideIn:
        if (stateTime_ >= timings_.fade) setState(FlowState::SelectItem);
        break;
    case FlowState::SelectItem:
        moveSelection(input, 10);
        if (input.confirm && !previousInput_.confirm) {
            postCue(FlowCue::Confirm);
            setState(FlowState::Confirm);
        } else if (input.cancel && !previousInput_.cancel) {
            setState(FlowState::SlideOut);
        }
        break;
    case FlowState::Confirm:
        if (stateTime_ >= timings_.confirm) {
            postCue(FlowCue::Purchase);
            emitSimple(FlowEventType::MenuAction, selection_);
            setState(FlowState::SelectItem);
        }
        break;
    case FlowState::NotEnoughBolts:
    case FlowState::TooManyItems:
        if (stateTime_ >= timings_.info)
            setState(FlowState::SelectItem);
        break;
    case FlowState::SlideOut:
        if (stateTime_ >= timings_.fade) {
            emitSimple(FlowEventType::ScreenChangeRequested, selection_);
            running_ = false;
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::stepLoadMenu(const FlowInput& input) {
    switch (state_) {
    case FlowState::SlideIn:
        if (stateTime_ >= timings_.fade) setState(FlowState::SelectItem);
        break;
    case FlowState::SelectItem:
        moveSelection(input, 4);
        if (input.confirm && !previousInput_.confirm) {
            postCue(FlowCue::Confirm);
            setState(FlowState::Confirm);
        } else if (input.cancel && !previousInput_.cancel) {
            setState(FlowState::SlideOut);
        }
        break;
    case FlowState::Confirm:
        if (stateTime_ >= timings_.confirm) {
            emitSimple(FlowEventType::SlotSelected, selection_);
            setState(FlowState::SelectItem);
        }
        break;
    case FlowState::SlideOut:
        if (stateTime_ >= timings_.fade) {
            emitSimple(FlowEventType::ScreenChangeRequested, selection_);
            running_ = false;
        }
        break;
    default:
        break;
    }
}

void GameFlowRuntime::stepSimpleMenu(const FlowInput& input) {
    // Menus with a recovered item table use the same fixed-array selection
    // loop as MainMenu. Screens without a table keep the earlier generic
    // event-only behavior.
    const int count = menuSlotCount(screen_, menuSupplier_);
    switch (state_) {
    // Most simple menu constructors select Select, not SelectDone. Their
    // observable input behaviour is the same in the recovered callbacks.
    case FlowState::Select:
    case FlowState::SelectDone:
        if ((input.up && !previousInput_.up) ||
            (input.left && !previousInput_.left)) {
            selection_ = nextVisibleMenuItem(screen_, selection_, -1,
                                             menuSupplier_, count);
            postCue(FlowCue::Select);
        } else if ((input.down && !previousInput_.down) ||
                   (input.right && !previousInput_.right)) {
            selection_ = nextVisibleMenuItem(screen_, selection_, 1,
                                             menuSupplier_, count);
            postCue(FlowCue::Select);
        } else if (input.confirm && !previousInput_.confirm) {
            const RecoveredMenuItem* item =
                menuItem(screen_, selection_, menuSupplier_);
            if (count != 0 && (!item || item->actionCode == 0))
                break;
            postCue(FlowCue::Confirm);
            if (count != 0)
                emitMenuAction(selection_);
            else
                emitSimple(FlowEventType::MenuAction, selection_);
        } else if (input.cancel && !previousInput_.cancel) {
            emitSimple(FlowEventType::ScreenChangeRequested, selection_);
            running_ = false;
        }
        break;
    case FlowState::ChangePage:
        moveSelection(input, count != 0 ? count : 10, true);
        if (input.confirm && !previousInput_.confirm)
            setState(FlowState::SelectDone);
        break;
    default:
        break;
    }
}

} // namespace mmu
