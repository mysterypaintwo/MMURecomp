// Host-only selftest for the portable top-level flow runtime.

#include "mmu/core/ActivityMaskReaderTable.h"
#include "mmu/core/ActivityMaskTable.h"
#include "mmu/core/BootCutsceneTable.h"
#include "mmu/core/CompletionCutsceneTable.h"
#include "mmu/core/GameFlow.h"
#include "mmu/core/GameFlowRuntime.h"
#include "mmu/core/ContinueStageTable.h"
#include "mmu/core/FlowEvidence.h"
#include "mmu/core/LevelSetupTable.h"
#include "mmu/core/ManagerFieldTable.h"
#include "mmu/core/MenuActionTable.h"
#include "mmu/core/OutroHandoffTable.h"
#include "mmu/core/MenuItemTables.h"
#include "mmu/core/RegisteredStateNames.h"
#include "mmu/core/RegisteredStateTransitions.h"
#include "mmu/core/StageSelectGrid.h"
#include "mmu/core/StateDispatcher.h"

#include <array>
#include <cstdio>
#include <string>

namespace {

int checks = 0;
int failures = 0;

void check(const char* name, bool ok) {
    ++checks;
    if (!ok) ++failures;
    std::printf("  check %-28s %s\n", name, ok ? "PASS" : "FAIL");
}

bool hasEvent(const mmu::GameFlowRuntime& flow, mmu::FlowEventType type) {
    // pollEvent() is non-const, so copy the runtime for this helper.
    mmu::GameFlowRuntime copy = flow;
    while (copy.pendingEventCount() != 0) {
        if (copy.pollEvent().type == type) return true;
    }
    return false;
}

bool findEventValue(const mmu::GameFlowRuntime& flow, mmu::FlowEventType type,
                    int& value) {
    mmu::GameFlowRuntime copy = flow;
    while (copy.pendingEventCount() != 0) {
        const mmu::FlowEvent event = copy.pollEvent();
        if (event.type == type) {
            value = event.value;
            return true;
        }
    }
    return false;
}

bool findMenuActionEvent(const mmu::GameFlowRuntime& flow,
                         mmu::FlowEvent& result) {
    mmu::GameFlowRuntime copy = flow;
    while (copy.pendingEventCount() != 0) {
        const mmu::FlowEvent event = copy.pollEvent();
        if (event.type == mmu::FlowEventType::MenuAction) {
            result = event;
            return true;
        }
    }
    return false;
}

bool findStageCompletedEvent(const mmu::GameFlowRuntime& flow, mmu::u32 id,
                             mmu::FlowEvent& result) {
    mmu::GameFlowRuntime copy = flow;
    while (copy.pendingEventCount() != 0) {
        const mmu::FlowEvent event = copy.pollEvent();
        if (event.type == mmu::FlowEventType::StageCompleted &&
            event.completedStageId == id) {
            result = event;
            return true;
        }
    }
    return false;
}

bool findLevelSetupEvent(const mmu::GameFlowRuntime& flow,
                         mmu::FlowEvent& result) {
    mmu::GameFlowRuntime copy = flow;
    while (copy.pendingEventCount() != 0) {
        const mmu::FlowEvent event = copy.pollEvent();
        if (event.type == mmu::FlowEventType::LevelSetupApplied) {
            result = event;
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    std::printf("MMU game-flow runtime selftest\n");

    mmu::GameFlowRuntime flow;
    check("reset screen", flow.screen() == mmu::FlowScreen::TitleMenu);
    check("reset state", flow.state() == mmu::FlowState::IntroFadeIn);
    check("reset time", flow.time() == 0.0f && flow.stateTime() == 0.0f);

    bool initialStatesValid = true;
    for (std::size_t i = 0; i < mmu::kFlowScreenCount; ++i) {
        const mmu::FlowScreen screen = mmu::kFlowScreens[i].className == nullptr
                                           ? mmu::FlowScreen::TitleMenu
                                           : static_cast<mmu::FlowScreen>(i);
        const mmu::FlowState state = mmu::flowInitialState(screen);
        if (mmu::kFlowScreens[i].stateCount == 0) {
            if (state != mmu::FlowState::Wait) initialStatesValid = false;
        } else if (!mmu::flowScreenHasState(screen, state)) {
            initialStatesValid = false;
        }
    }
    check("initial states", initialStatesValid);

    const mmu::FlowInput none{};
    flow.step(1.0f, none);
    check("title intro exit", flow.state() == mmu::FlowState::Wait);
    mmu::FlowInput confirm{};
    confirm.confirm = true;
    flow.step(1.0f / 60.0f, confirm);
    check("title confirm", flow.state() == mmu::FlowState::Flash &&
                               flow.lastCue() == mmu::FlowCue::Confirm);
    check("title events", hasEvent(flow, mmu::FlowEventType::Cue) &&
                              hasEvent(flow, mmu::FlowEventType::StateChanged));

    flow.step(1.0f, none);
    check("title flash exit", flow.state() == mmu::FlowState::IntroFadeOut);
    flow.step(1.0f, none);
    check("title fade request", hasEvent(flow, mmu::FlowEventType::ScreenChangeRequested) &&
                                   !flow.running());
    flow.clearEvents();

    flow.reset();
    flow.step(1.0f, none);
    flow.step(7.0f, none);
    check("title idle exit", flow.state() == mmu::FlowState::IntroFadeOut);

    flow.setScreen(mmu::FlowScreen::MainMenu);
    check("main initial", flow.state() == mmu::FlowState::IntroFadeIn);
    flow.step(1.0f, none);
    check("main intro exit", flow.state() == mmu::FlowState::Select);
    flow.clearEvents();
    mmu::FlowInput up{};
    up.up = true;
    flow.step(1.0f / 60.0f, up);
    // Slots 9, 6, 5 and 3 are blank; the first visible row above slot 0 is 8.
    check("main skip blank up", flow.selection() == 8);
    flow.step(1.0f / 60.0f, none);
    mmu::FlowInput down{};
    mmu::FlowInput right{};
    down.down = true;
    right.right = true;
    flow.step(1.0f / 60.0f, down);
    check("main skip blank down", flow.selection() == 0);
    flow.step(1.0f / 60.0f, none);

    constexpr mmu::u32 kInitialMainMenuSupplier = 0x508EA0u;
    std::size_t mainSlots = 0;
    for (const mmu::RecoveredMenuItem& item : mmu::kRecoveredMenuItems) {
        if (item.className == "MainMenu" &&
            item.supplierVa == kInitialMainMenuSupplier) {
            ++mainSlots;
        }
    }
    const mmu::RecoveredMenuItem* newGameItem = mmu::findMenuItem(
        "MainMenu", 0, kInitialMainMenuSupplier);
    const mmu::RecoveredMenuItem* blankItem = mmu::findMenuItem(
        "MainMenu", 3, kInitialMainMenuSupplier);
    check("main item table", mainSlots == 10 && newGameItem != nullptr &&
                                 newGameItem->actionCode == 0x47 &&
                                 newGameItem->label == "NEW GAME" &&
                                 blankItem != nullptr &&
                                 blankItem->actionCode == 0 &&
                                 blankItem->label.empty());
    check("menu item corpus", mmu::kRecoveredMenuItemsCount >= 240 &&
                                  mmu::hasMenuItemLabel("EXIT GAME") &&
                                  mmu::hasMenuItemLabel("BUSTER:"));
    const mmu::RecoveredMenuAction* newGameAction =
        mmu::findMenuAction(newGameItem ? newGameItem->actionCode : 0);
    check("menu action table", mmu::kRecoveredMenuActionsCount >= 30 &&
                                   newGameAction != nullptr &&
                                   newGameAction->kind ==
                                       mmu::MenuActionKind::Screen &&
                                   newGameAction->screenCount == 2 &&
                                   newGameAction->screens[0] ==
                                       mmu::FlowScreen::NewGameMenu &&
                                   mmu::findMenuAction(0) == nullptr);
    const mmu::RecoveredMenuAction* gameMenuAction = mmu::findMenuAction(0x02);
    const mmu::RecoveredMenuAction* gameOverAction = mmu::findMenuAction(0x09);
    check("menu action variants",
          gameMenuAction != nullptr && gameMenuAction->variantCount == 1 &&
              gameMenuAction->variantSuppliers[0] == kInitialMainMenuSupplier &&
              gameOverAction != nullptr && gameOverAction->variantCount == 2 &&
              gameOverAction->variantSuppliers[0] == 0x5089C0u &&
              gameOverAction->variantSuppliers[1] == 0x5084E0u);

    const mmu::RecoveredContinueStage* firstStage =
        mmu::findContinueStage(1);
    const mmu::RecoveredContinueStage* lastStage =
        mmu::findContinueStage(39);
    check("continue stage table",
          mmu::kRecoveredContinueStageCount == 38 && firstStage != nullptr &&
              firstStage->className == "LevelTankMan" &&
              firstStage->constructorVa == 0x528420u &&
              mmu::findContinueStage(14) != nullptr &&
              mmu::findContinueStage(14)->className == "LevelWilyFive" &&
              mmu::findContinueStage(16) != nullptr &&
              mmu::findContinueStage(16)->className == "LevelEndless" &&
              lastStage != nullptr &&
              lastStage->className == "LevelPerfectWhirlpoolMan" &&
              mmu::findContinueStage(0) == nullptr &&
              mmu::findContinueStage(15) == nullptr);

    const mmu::RecoveredLevelSetup* yoyoSetup = mmu::findLevelSetup(5);
    const mmu::RecoveredLevelSetup* cometSetup = mmu::findLevelSetup(2);
    const mmu::RecoveredLevelSetup* perfectComet = mmu::findLevelSetup(37);
    check("level setup table",
          mmu::kRecoveredLevelSetupCount == 38 &&
              mmu::kRecoveredLevelSetupCueCount == 33 && yoyoSetup != nullptr &&
              yoyoSetup->constructorVa == 0x512a90u &&
              yoyoSetup->cueRegistrarCallVa == 0x512af3u &&
              yoyoSetup->environmentCallVa == 0x512b71u &&
              yoyoSetup->globalSourceVa == 0x69899cu &&
              yoyoSetup->loopCueList == "YoYoManStageLoop" &&
              yoyoSetup->introCueCount == 1 && yoyoSetup->introCueList.empty() &&
              yoyoSetup->zeroWriteCount == 3 &&
              yoyoSetup->zeroWriteOffsets[0] == 0x04u &&
              yoyoSetup->zeroWriteOffsets[1] == 0xf4u &&
              yoyoSetup->zeroWriteOffsets[2] == 0xf8u);
    check("level setup alternatives",
          cometSetup != nullptr && cometSetup->globalSourceVa == 0x6989b4u &&
              cometSetup->loopCueCount == 2 &&
              mmu::levelSetupHasLoopCue(*cometSetup,
                                        "CometWomanStageLoopAlt") &&
              mmu::levelSetupHasIntroCue(*cometSetup,
                                         "CometWomanStageIntroAlt") &&
              perfectComet != nullptr &&
              perfectComet->globalSourceVa == 0x6989b4u &&
              mmu::findLevelSetupByLoopCue("WilyStageOneLoopOriginal") !=
                  nullptr &&
              mmu::findLevelSetupByLoopCue("WilyStageOneLoopOriginal")
                      ->stageId == 10u);
    check("level setup constructor lookup",
          mmu::findLevelSetupByConstructor(0x5ef650u) != nullptr &&
              mmu::findLevelSetupByConstructor(0x5ef650u)->stageId == 37u &&
              mmu::findLevelSetupByConstructor(0) == nullptr);
    flow.clearEvents();
    check("level setup apply",
          flow.applyLevelSetup(5) && flow.levelSetup() == yoyoSetup &&
              hasEvent(flow, mmu::FlowEventType::LevelSetupApplied));
    mmu::FlowEvent setupEvent;
    check("level setup event fields",
          findLevelSetupEvent(flow, setupEvent) &&
              setupEvent.levelSetup == yoyoSetup &&
              setupEvent.levelSetup->stageId == 5u &&
              setupEvent.levelSetup->constructorVa == 0x512a90u);
    flow.clearEvents();
    check("level setup constructor apply",
          flow.applyLevelSetupConstructor(0x5ef650u) &&
              flow.levelSetup() != nullptr && flow.levelSetup()->stageId == 37u);
    flow.clearEvents();

    check("manager field table",
          mmu::kManagerGlobalVa == 0x6c55a0u &&
              mmu::kManagerCompletionObjectGlobalVa == 0x6c55f8u &&
              mmu::kManagerLiteralWriteCount == 29 &&
              mmu::kManagerAccessorCount == 11 &&
              mmu::kManagerFieldAccessCount == 44 &&
              mmu::countManagerLiteralWrites(0) == 2 &&
              mmu::managerHasLiteralWrite(0x08u, 0x80u) &&
              mmu::managerHasLiteralWrite(0x0cu, 0x04u) &&
              mmu::managerHasLiteralWrite(0x30u, 0x01u));
    const mmu::ManagerAccessor* flag18Writer = mmu::findManagerAccessor(0x403220u);
    const mmu::ManagerAccessor* flag19Reader = mmu::findManagerAccessor(0x4032e0u);
    const mmu::ManagerAccessor* endingReader = mmu::findManagerAccessor(0x403d40u);
    const mmu::ManagerAccessor* maskReader = mmu::findManagerAccessor(0x507ec0u);
    check("manager accessor helpers",
          flag18Writer != nullptr && flag18Writer->offset == 0x18u &&
              flag18Writer->size == 1 &&
              flag18Writer->kind == mmu::ManagerFieldAccessKind::Write &&
              flag18Writer->callCount == 157 &&
              flag19Reader != nullptr && flag19Reader->offset == 0x19u &&
              flag19Reader->callCount == 4 && endingReader != nullptr &&
              endingReader->offset == 0x59u && endingReader->callCount == 115 &&
              endingReader->forwardedVa == 0x403cc0u &&
              maskReader != nullptr && maskReader->offset == 0 &&
              maskReader->size == 4 && maskReader->callCount == 36);
    bool hasManagerPollPointer = false;
    bool hasManagerStageId = false;
    bool hasCompletionObjectOffset = false;
    for (const mmu::ManagerFieldAccess& row : mmu::kManagerFieldAccesses) {
        if (row.functionVa == 0x50f7d0u && row.offset == 0x18u &&
            row.size == 4 &&
            row.kind == mmu::ManagerFieldAccessKind::Write &&
            row.count == 30) {
            hasManagerPollPointer = true;
        }
        if (row.functionVa == 0x50f7d0u && row.offset == 0x08u &&
            row.kind == mmu::ManagerFieldAccessKind::Write &&
            row.count == 4) {
            hasManagerStageId = true;
        }
        if (row.functionVa == 0x4308a0u && row.objectGlobalVa == 0x6c55f8u &&
            row.offset == 0x178u &&
            row.kind == mmu::ManagerFieldAccessKind::Write) {
            hasCompletionObjectOffset = true;
        }
    }
    check("manager consumer fields",
          hasManagerPollPointer && hasManagerStageId &&
              hasCompletionObjectOffset &&
              mmu::findManagerAccessorAt(0x59u) != nullptr &&
              mmu::findManagerFieldAccess(0x50bbb0u, 0x18u) == nullptr);

    const mmu::OutroHandoffCall* namedEnergy =
        mmu::findOutroHandoffCall(0x5a8a97u);
    const mmu::OutroHandoffCall* megaHandoff =
        mmu::findOutroHandoffCall(0x5b0a57u);
    const mmu::OutroHandoffCall* cutsceneLoad =
        mmu::findOutroHandoffCall(0x526410u);
    const mmu::OutroHandoffCall* bigExplosion =
        mmu::findOutroHandoffCall(0x526632u);
    const mmu::OutroHandoffCall* teleporterEndless =
        mmu::findOutroHandoffCall(0x5d3955u);
    check("outro handoff table",
          mmu::kOutroHandoffCallCount == 19 &&
              mmu::kOutroHandoffStringCount == 16 &&
              mmu::kOutroHandoffFactCount == 43 &&
              mmu::hasOutroHandoffOwner(0x5b02f0u) &&
              mmu::hasOutroHandoffOwner(0x5262c0u) &&
              !mmu::hasOutroHandoffOwner(0x400000u) &&
              namedEnergy != nullptr && megaHandoff != nullptr &&
              mmu::findOutroHandoffCall(0x5b0b69u) != nullptr &&
              cutsceneLoad != nullptr && bigExplosion != nullptr &&
              mmu::findOutroHandoffCall(0x5c682fu) != nullptr &&
              mmu::findOutroHandoffCall(0x5c8625u) != nullptr &&
              teleporterEndless != nullptr);
    check("outro handoff raw fields",
          namedEnergy != nullptr &&
              namedEnergy->targetVa == 0x420a90u &&
              namedEnergy->stringVa == 0x67d744u &&
              namedEnergy->hash == 0xa5964be6u &&
              namedEnergy->kind == mmu::OutroHandoffKind::NamedStateCall &&
              namedEnergy->literal == "EnergyMeter3s" &&
              megaHandoff != nullptr &&
              megaHandoff->targetVa == 0x5a8870u &&
              megaHandoff->stringVa == 0 &&
              cutsceneLoad != nullptr &&
              cutsceneLoad->targetVa == 0x621890u &&
              cutsceneLoad->stringVa == 0x66f044u &&
              cutsceneLoad->hash == 0x60bb8d06u &&
              bigExplosion != nullptr &&
              bigExplosion->kind == mmu::OutroHandoffKind::CueRegistrar &&
              bigExplosion->hash == 0xd3f76c3cu &&
              teleporterEndless != nullptr &&
              teleporterEndless->targetVa == 0x4405c0u &&
              teleporterEndless->stringVa == 0x68138cu &&
              teleporterEndless->hash == 0x3bdfa46fu &&
              teleporterEndless->literal == "ConcludeBossFight");
    bool hasEnergyMeterMaskOne = false;
    bool hasEnergyMeterMaskTwo = false;
    for (const mmu::OutroHandoffFact& fact : mmu::kOutroHandoffFacts) {
        if (fact.functionVa != 0x5a8870u ||
            fact.kind != mmu::OutroHandoffFactKind::LiteralConstant) {
            continue;
        }
        hasEnergyMeterMaskOne = hasEnergyMeterMaskOne || fact.value == 1u;
        hasEnergyMeterMaskTwo = hasEnergyMeterMaskTwo || fact.value == 2u;
    }
    check("outro handoff raw facts",
          !mmu::hasOutroHandoffFactOffset(mmu::kOutroHandoffNoOffset) &&
              mmu::hasOutroHandoffFactOffset(0x18cu) &&
              mmu::hasOutroHandoffFactOffset(0x190u) &&
              mmu::hasOutroHandoffFactOffset(0x194u) &&
              mmu::hasOutroHandoffFactOffset(0x17cu) &&
              mmu::hasOutroHandoffFactOffset(0x3dcu) &&
              mmu::hasOutroHandoffFactOffset(0x144u) &&
              mmu::hasOutroHandoffFactOffset(0x148u) &&
              mmu::hasOutroHandoffFactOffset(0x14cu) &&
              hasEnergyMeterMaskOne && hasEnergyMeterMaskTwo);

    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    check("main confirm behavior", flow.state() == mmu::FlowState::Flash &&
                                       flow.lastCue() == mmu::FlowCue::Confirm);
    flow.step(1.0f, none);
    mmu::FlowEvent actionEvent;
    check("main action event",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x47 && actionEvent.value == 0 &&
              actionEvent.label == "NEW GAME" &&
              actionEvent.menuSupplierVa == kInitialMainMenuSupplier &&
              actionEvent.hasDestination &&
              actionEvent.destination == mmu::FlowScreen::NewGameMenu);

    mmu::FlowMenuFlags newGameFlags;
    newGameFlags.flag0x19 = true;
    flow.setMenuFlags(newGameFlags);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("conditional new game destination",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x47 &&
              actionEvent.destination == mmu::FlowScreen::CharacterSelectMenu);
    flow.setMenuFlags(mmu::FlowMenuFlags{});

    constexpr mmu::u32 kGameOverSupplier = 0x5084E0u;
    flow.setMenuSupplier(kGameOverSupplier);
    check("menu supplier selection", flow.menuSupplier() == kGameOverSupplier);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("variant action event",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x0a &&
              actionEvent.label == "CONTINUE" &&
              actionEvent.menuSupplierVa == kGameOverSupplier &&
              !actionEvent.hasDestination);

    flow.setContinueStageId(13);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("continue stage event",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x0a &&
              actionEvent.continueStageId == 13 &&
              actionEvent.continueConstructorVa == 0x5211F0u &&
              actionEvent.continueStageClass == "LevelWilyFour");
    flow.setContinueStageId(0);
    flow.setMenuSupplier(kInitialMainMenuSupplier);

    // SpecialMenu slot 5 is a spacer; the credits rows are 0x1e/0x1f and
    // carry the literal .xcut paths built in FUN_0050f7d0.
    flow.setScreen(mmu::FlowScreen::SpecialMenu);
    flow.step(1.0f, none);
    check("special menu initial", flow.selection() == 0);
    for (int i = 0; i < 5; ++i) {
        flow.step(1.0f / 60.0f, down);
        flow.step(1.0f / 60.0f, none);
    }
    check("special credits selection", flow.selection() == 6);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    check("credits cutscene event",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x1e &&
              actionEvent.label == "CREDITS (MAIN GAME)" &&
              actionEvent.cutscenePath ==
                  "..\\Cutscenes\\CSN07Credits\\CSNMainCredits.xcut");
    flow.setMenuSupplier(kInitialMainMenuSupplier);

    struct ExpectedSupplier {
        mmu::FlowScreen screen;
        mmu::u32 supplier;
        int slots;
    };
    const ExpectedSupplier expectedSuppliers[] = {
        {mmu::FlowScreen::MainMenu, 0x508EA0u, 10},
        {mmu::FlowScreen::OptionsMenu, 0x4FB700u, 18},
        {mmu::FlowScreen::BusterMenu, 0x618C20u, 8},
        {mmu::FlowScreen::CharacterSelectMenu, 0x617C70u, 9},
        {mmu::FlowScreen::NewGameMenu, 0x4FE3B0u, 9},
        {mmu::FlowScreen::EndlessAttackMenu, 0x616840u, 10},
        {mmu::FlowScreen::EndlessBossAttackMenu, 0x615390u, 10},
        {mmu::FlowScreen::GauntletMenu, 0x613B10u, 9},
        {mmu::FlowScreen::SpecialMenu, 0x5DA920u, 9},
        {mmu::FlowScreen::SuperRobotMastersMenuA, 0x5D9360u, 9},
        {mmu::FlowScreen::SuperRobotMastersMenuB, 0x5D79E0u, 9},
        {mmu::FlowScreen::SuperRobotMastersMenuC, 0x5D6070u, 9},
        {mmu::FlowScreen::SuperRobotMastersMenuD, 0x5D4AA0u, 9},
        {mmu::FlowScreen::WhirlpoolManStageStartMenu, 0x5B8A80u, 8},
    };
    bool suppliersValid = true;
    for (const ExpectedSupplier& expected : expectedSuppliers) {
        flow.setScreen(expected.screen);
        if (flow.menuSupplier() != expected.supplier)
            suppliersValid = false;
        int slots = 0;
        for (const mmu::RecoveredMenuItem& item : mmu::kRecoveredMenuItems) {
            if (item.className == mmu::flowScreenName(expected.screen) &&
                item.supplierVa == expected.supplier) {
                ++slots;
            }
        }
        if (slots != expected.slots) suppliersValid = false;
    }
    check("menu supplier defaults", suppliersValid);

    // 0x511350 builds 0x508340 when the raw stage predicate is false and
    // 0x5081a0 when it is true. The two tables differ by the EXIT STAGE row.
    flow.setScreen(mmu::FlowScreen::MenuInGame);
    check("pause default variant", flow.menuSupplier() == 0x508340u);
    mmu::FlowMenuFlags pauseFlags;
    pauseFlags.pauseExitStageVariant = true;
    flow.setMenuFlags(pauseFlags);
    check("pause exit-stage variant", flow.menuSupplier() == 0x5081A0u);
    flow.setMenuFlags(mmu::FlowMenuFlags{});

    mmu::FlowMenuFlags flags;
    flags.flag0x19 = true;
    flow.setMenuFlags(flags);
    flow.setScreen(mmu::FlowScreen::EndlessAttackMenu);
    check("endless flag variant", flow.menuSupplier() == 0x6166A0u);
    flow.setScreen(mmu::FlowScreen::GauntletMenu);
    check("gauntlet flag variant", flow.menuSupplier() == 0x613970u);
    flags = mmu::FlowMenuFlags{};
    flags.flag0x59 = true;
    flow.setMenuFlags(flags);
    flow.setScreen(mmu::FlowScreen::NewGameMenu);
    check("newgame flag variant", flow.menuSupplier() == 0x4FE210u);
    flow.setMenuFlags(mmu::FlowMenuFlags{});

    // Action 0x06 (MainMenu STAGE SELECT) has two conditional cutscene branches
    // and a fallback that clears manager +0x3c and builds MenuStageSelect.
    constexpr const char* kFourBossCutscene =
        "..\\Cutscenes\\CSN02FourBosses\\CSN02FourBosses.xcut";
    constexpr const char* kEightBossCutscene =
        "..\\Cutscenes\\CSN03EightBosses\\CSN03EightBosses.xcut";
    flow.reset();
    flow.setMenuFlags(mmu::FlowMenuFlags{});
    flow.setScreen(mmu::FlowScreen::MainMenu);
    flow.setMenuSupplier(0x508820u);
    flow.step(1.0f, none);
    mmu::FlowMenuFlags stageFlags;
    stageFlags.subItemCount = 1;
    stageFlags.bossCount = 4;
    flow.setMenuFlags(stageFlags);
    flow.setMenuSupplier(0x508820u);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("four boss cutscene branch",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x06 &&
              actionEvent.cutscenePath == kFourBossCutscene &&
              actionEvent.hasManagerStagePhase &&
              actionEvent.managerStagePhase == 8 &&
              actionEvent.hasManagerFlag0x18 && !actionEvent.hasDestination &&
              !actionEvent.managerStageIdCleared && flow.managerStagePhase() == 8);

    flow.setScreen(mmu::FlowScreen::MainMenu);
    flow.setMenuSupplier(0x508820u);
    flow.step(1.0f, none);
    stageFlags = mmu::FlowMenuFlags{};
    stageFlags.subItemCount = 1;
    stageFlags.bossCount = 8;
    flow.setMenuFlags(stageFlags);
    flow.setMenuSupplier(0x508820u);
    flow.setManagerStagePhase(0);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("eight boss cutscene branch",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x06 &&
              actionEvent.cutscenePath == kEightBossCutscene &&
              actionEvent.hasManagerStagePhase &&
              actionEvent.managerStagePhase == 8 && !actionEvent.hasDestination &&
              flow.managerStagePhase() == 8);

    flow.setScreen(mmu::FlowScreen::MainMenu);
    flow.setMenuSupplier(0x508820u);
    flow.step(1.0f, none);
    stageFlags = mmu::FlowMenuFlags{};
    stageFlags.subItemCount = 1;
    flow.setMenuFlags(stageFlags);
    flow.setMenuSupplier(0x508820u);
    flow.setManagerStagePhase(0);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("stage select fallback",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x06 &&
              actionEvent.managerStageIdCleared &&
              !actionEvent.hasManagerStagePhase &&
              actionEvent.hasDestination &&
              actionEvent.destination == mmu::FlowScreen::MenuStageSelect &&
              flow.managerStagePhase() == 0);
    flow.setMenuFlags(mmu::FlowMenuFlags{});

    // Action 0x10 writes manager byte +0x5d, read by the main loop as quit.
    flow.reset();
    flow.setScreen(mmu::FlowScreen::MainMenu);
    flow.setMenuSupplier(0x5084E0u);
    flow.step(1.0f, none);
    for (int i = 0; i < 5; ++i) {
        flow.step(1.0f / 60.0f, down);
        flow.step(1.0f / 60.0f, none);
    }
    check("exit game selection", flow.selection() == 8);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("exit game action",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x10 &&
              actionEvent.label == "EXIT GAME" && actionEvent.exitGame &&
              flow.exitRequested());

    // BusterMenu's generic 0x41 forwards the opaque manager field at +0x34.
    flow.reset();
    flow.setScreen(mmu::FlowScreen::BusterMenu);
    flow.step(1.0f, none);
    flow.setManagerOpaque(0x1234u);
    for (int i = 0; i < 3; ++i) {
        flow.step(1.0f / 60.0f, down);
        flow.step(1.0f / 60.0f, none);
    }
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("opaque manager phase",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x41 &&
              actionEvent.hasManagerPhase &&
              actionEvent.managerPhase == 0x1234u &&
              flow.managerPhase() == 0x1234u);

    flow.reset();
    flow.setScreen(mmu::FlowScreen::CharacterSelectMenu);
    flow.step(1.0f, none);
    flow.step(1.0f / 60.0f, down);
    flow.step(1.0f / 60.0f, none);
    flow.step(1.0f / 60.0f, down); // blanks 2-7 skip to the BACK row
    flow.step(1.0f / 60.0f, none);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("literal manager phase",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x41 &&
              actionEvent.hasManagerPhase && actionEvent.managerPhase == 2 &&
              flow.managerPhase() == 2);

    flow.setScreen(mmu::FlowScreen::CharacterSelectMenu);
    flow.step(1.0f / 60.0f, none);
    check("character initial", flow.state() == mmu::FlowState::Select &&
                                   flow.menuSupplier() == 0x617C70u);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    check("character action event",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x48 &&
              actionEvent.label == "MEGA MAN" &&
              actionEvent.menuSupplierVa == 0x617C70u &&
              !actionEvent.hasDestination && actionEvent.character == 0 &&
              actionEvent.hasManagerPhase && actionEvent.managerPhase == 5 &&
              flow.managerPhase() == 5);
    flow.step(1.0f / 60.0f, none);
    flow.step(1.0f / 60.0f, down);
    check("character down", flow.selection() == 1);
    flow.step(1.0f / 60.0f, none);
    flow.step(1.0f / 60.0f, down);
    // Slots 2-7 are blanks; slot 8 is BACK TO GAME MENU.
    check("character blank skip", flow.selection() == 8);
    flow.step(1.0f / 60.0f, none);
    flow.step(1.0f / 60.0f, down);
    check("character blank wrap", flow.selection() == 0);

    flow.reset();
    flow.setScreen(mmu::FlowScreen::CharacterSelectMenu);
    flow.step(1.0f, none);
    flow.step(1.0f / 60.0f, down);
    flow.step(1.0f / 60.0f, none);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("prototype character action",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x49 &&
              actionEvent.label == "Z-PROTOTYPE" &&
              actionEvent.character == 1 && actionEvent.hasManagerPhase &&
              actionEvent.managerPhase == 5 && flow.managerPhase() == 5);

    // StageCompleted mirrors the literal loader-id routing at the end of
    // FUN_00511350. It is evidence for that dispatch, not a gameplay flag.
    flow.reset();
    flow.setContinueStageId(13);
    flow.clearEvents();
    flow.completeStage(0x10u);
    mmu::FlowEvent completed;
    check("complete stage route 10",
          findStageCompletedEvent(flow, 0x10u, completed) &&
              completed.completedActionCode == 0x12u &&
              completed.completedManagerPhase == 0x12u &&
              completed.completionCue == "MenuThemeB" &&
              completed.hasManagerPhase && flow.managerPhase() == 0x12u);
    flow.completeStage(0x20u);
    check("complete stage route 20",
          findStageCompletedEvent(flow, 0x20u, completed) &&
              completed.completedActionCode == 0x20u &&
              completed.completionCue == "MenuThemeB");
    flow.completeStage(0x1au);
    check("complete stage route 1a",
          findStageCompletedEvent(flow, 0x1au, completed) &&
              completed.completedActionCode == 0x22u &&
              completed.completionCue == "MenuThemeB");
    flow.completeStage(0x99u);
    check("complete stage fallback",
          findStageCompletedEvent(flow, 0x99u, completed) &&
              completed.completedActionCode == 0 &&
              completed.completedManagerPhase == 5 &&
              completed.completionCue == "GameOverIntro");
    flow.setContinueStageId(0);
    flow.clearEvents();
    flow.completeStage(0x99u);
    check("complete stage fallback phase 9",
          findStageCompletedEvent(flow, 0x99u, completed) &&
              completed.completedManagerPhase == 9 &&
              completed.completionCue == "GameOverIntro");

    // FUN_0050bbb0 constructs the global manager and starts the logos cutscene
    // before any menu exists. The initial mask is the source read by 0x507ec0.
    flow.reset();
    flow.clearEvents();
    flow.emitBoot();
    check("boot manager evidence",
          mmu::kBootActivityMask == 0x80u &&
              mmu::kBootManagerWriteCount == 3 &&
              mmu::kBootManagerGlobalVa == 0x6c55a0u);
    {
        mmu::GameFlowRuntime bootCopy = flow;
        bool bootFields = false;
        while (bootCopy.pendingEventCount() != 0) {
            const mmu::FlowEvent boot = bootCopy.pollEvent();
            if (boot.type != mmu::FlowEventType::ManagerBoot) continue;
            bootFields =
                boot.type == mmu::FlowEventType::ManagerBoot &&
                boot.bootActivityMask == 0x80u &&
                boot.cutscenePath.find("CSN00Logos") != std::string_view::npos &&
                boot.pendingCutscenePath.find("CSN01Intro") !=
                    std::string_view::npos &&
                boot.bootWriteCount == 3 &&
                boot.bootWriteOffsets[0] == 0x08 &&
                boot.bootWriteValues[0] == 0x80 &&
                boot.bootWriteOffsets[1] == 0x0c &&
                boot.bootWriteValues[1] == 0x04 &&
                boot.bootWriteOffsets[2] == 0x30 &&
                boot.bootWriteValues[2] == 0x01;
        }
        check("boot event paths and writes", bootFields);
    }
    check("boot installs activity mask",
          flow.keyedFlags().activityMask() == mmu::kBootActivityMask);
    check("boot leaves previous mask untouched",
          flow.keyedFlags().previousActivityMask() ==
              mmu::KeyedFlagRuntime::kAllCategories);
    flow.clearEvents();

    // FUN_004412d0 registers keyed flags and FUN_00441820 advances a four-bit
    // state machine at record +0x24. FUN_00441c80 then reads those bits: 4 is
    // current true, 8 is a rising edge, 2 is a falling edge.
    check("save flag tables",
          mmu::kSaveFlagWriterCount == 138 &&
              mmu::kSaveFlagMask8KeyCount > 0 &&
              mmu::kSaveFlagMask4KeyCount > 0 &&
              mmu::findSaveFlagWriter(7) != nullptr &&
              mmu::findSaveFlagWriter(7)->aux == 49);
    {
        bool covered = true;
        for (const mmu::u32 key : mmu::kSaveFlagMask8Keys)
            covered = covered && mmu::findSaveFlagWriter(key) != nullptr;
        for (const mmu::u32 key : mmu::kSaveFlagMask4Keys)
            covered = covered && mmu::findSaveFlagWriter(key) != nullptr;
        for (const auto& reader : mmu::kDirectSaveFlagReaders)
            covered = covered && mmu::findSaveFlagWriter(reader.key) != nullptr;
        check("save flag reader coverage",
              covered && mmu::kDirectSaveFlagReaderCount == 3 &&
                  mmu::kDirectSaveFlagReaders[0].key == 1 &&
                  mmu::kDirectSaveFlagReaders[0].mask == 8 &&
                  mmu::kDirectSaveFlagReaders[1].key == 30 &&
                  mmu::kDirectSaveFlagReaders[1].mask == 8 &&
                  mmu::kDirectSaveFlagReaders[2].key == 30 &&
                  mmu::kDirectSaveFlagReaders[2].mask == 2);
    }
    {
        const mmu::RecoveredActivityMaskCall* title =
            mmu::findActivityMaskCall(0x4f3099u);
        const mmu::RecoveredActivityMaskCall* wily =
            mmu::findActivityMaskFunction(0x5262c0u);
        check("activity mask raw sites",
              mmu::kRecoveredActivityMaskCallCount == 24 &&
                  mmu::kActivityMaskSetterVa == 0x507e90u &&
                  mmu::kActivityMaskGetterVa == 0x507ec0u &&
                  mmu::countActivityMaskCalls(0x1u) == 13 &&
                  mmu::countActivityMaskCalls(0x2u) == 2 &&
                  mmu::countActivityMaskCalls(0x10u) == 4 &&
                  mmu::countActivityMaskCalls(0x40u) == 2 &&
                  mmu::countActivityMaskCalls(0x100u) == 2 &&
                  mmu::countActivityMaskCalls(0x200u) == 1 &&
                  title != nullptr && title->mask == 0x200u &&
                  title->owner == "TitleMenu" &&
                  wily != nullptr && wily->mask == 0x100u &&
                  wily->owner == "LevelWilyFive");
    }
    {
        mmu::GameFlowRuntime maskFlow;
        maskFlow.reset();
        maskFlow.clearEvents();
        check("activity mask call applied",
              maskFlow.applyActivityMaskCall(0x4f3099u) &&
                  maskFlow.keyedFlags().activityMask() == 0x200u);
        bool eventFields = false;
        mmu::GameFlowRuntime maskCopy = maskFlow;
        while (maskCopy.pendingEventCount() != 0) {
            const mmu::FlowEvent event = maskCopy.pollEvent();
            if (event.type != mmu::FlowEventType::ActivityMaskChanged) continue;
            eventFields = event.activityMaskValue == 0x200u &&
                          event.activityMaskPrevious ==
                              mmu::KeyedFlagRuntime::kAllCategories &&
                          event.activityMaskCallVa == 0x4f3099u &&
                          event.activityMaskFunctionVa == 0x4f3040u &&
                          event.activityMaskOwner == "TitleMenu" &&
                          event.activityMaskEvidence == "cue:Confirm,setState:Flash";
        }
        check("activity mask event evidence", eventFields);
        check("unknown activity mask call rejected",
              !maskFlow.applyActivityMaskCall(0x123456u));

        const mmu::RecoveredSaveFlagWriter* keyOne =
            mmu::findSaveFlagWriter(1);
        std::array<mmu::u8, mmu::kKeyedFlagKeyboardLimit> keyboard{};
        keyboard[keyOne->aux] = 1;
        maskFlow.evaluateKeyedFlags(keyboard.data(), keyboard.size(),
                                    nullptr, 0);
        check("activity mask 0x200 gates key one",
              !maskFlow.keyedFlags().query(1, mmu::KeyedFlagRuntime::kTrue));
        check("activity mask previous write",
              maskFlow.keyedFlags().previousActivityMask() ==
                  mmu::KeyedFlagRuntime::kAllCategories);
        maskFlow.applyActivityMaskCall(0x54a72fu);
        check("activity mask previous retained",
              maskFlow.keyedFlags().previousActivityMask() == 0x200u);
        maskFlow.evaluateKeyedFlags(keyboard.data(), keyboard.size(),
                                    nullptr, 0);
        check("activity mask 0x1 enables key one",
              maskFlow.keyedFlags().query(1, mmu::KeyedFlagRuntime::kRising));
    }
    {
        const mmu::RecoveredActivityMaskRead* bit =
            mmu::findActivityMaskRead(0x6235f0u);
        const mmu::RecoveredActivityMaskRead* equal =
            mmu::findActivityMaskRead(0x54d19fu);
        const mmu::RecoveredActivityMaskRead* intersect =
            mmu::findActivityMaskRead(0x43d061u);
        check("activity mask read sites",
              mmu::kRecoveredActivityMaskReadCount == 36 &&
                  mmu::countActivityMaskReads(
                      mmu::ActivityMaskReadKind::Equal) == 26 &&
                  mmu::countActivityMaskReads(
                      mmu::ActivityMaskReadKind::Bit) == 3 &&
                  mmu::countActivityMaskReads(
                      mmu::ActivityMaskReadKind::Intersect) == 7 &&
                  bit != nullptr &&
                  bit->kind == mmu::ActivityMaskReadKind::Bit &&
                  bit->mask == 0x80u && bit->operand == "eax, 0x80" &&
                  equal != nullptr &&
                  equal->kind == mmu::ActivityMaskReadKind::Equal &&
                  equal->mask == 0x2u && equal->owner == "SpikePlatform" &&
                  intersect != nullptr &&
                  intersect->kind == mmu::ActivityMaskReadKind::Intersect &&
                  intersect->operand == "dword ptr [esi + 0x9c], eax");
        mmu::GameFlowRuntime readFlow;
        readFlow.setActivityMask(0x80u);
        check("mask read bit evaluates",
              readFlow.evaluateActivityMaskRead(0x6235f0u));
        check("mask read equal rejects mismatch",
              !readFlow.evaluateActivityMaskRead(0x54d19fu));
        readFlow.setActivityMask(0x2u);
        check("mask read equal evaluates",
              readFlow.evaluateActivityMaskRead(0x54d19fu));
        check("mask read intersect needs source",
              !readFlow.evaluateActivityMaskRead(0x43d061u));
        check("mask read intersect evaluates",
              readFlow.evaluateActivityMaskRead(0x43d061u, 0x2u));
        check("unknown mask read rejected",
              !readFlow.evaluateActivityMaskRead(0x123456u));
    }
    {
        mmu::KeyedFlagRuntime flags;
        flags.setInitialFalse(true); // FUN_00441b70(1) activation
        flags.setConditionOverride(7, true);
        flags.evaluate(nullptr, 0, nullptr, 0);
        check("keyed flag rising", flags.query(7, mmu::KeyedFlagRuntime::kTrue) &&
                                      flags.query(7, mmu::KeyedFlagRuntime::kRising) &&
                                      flags.bits(7) == 0x0c);
        flags.evaluate(nullptr, 0, nullptr, 0);
        check("keyed flag stable true",
              flags.bits(7) == mmu::KeyedFlagRuntime::kTrue &&
                  !flags.query(7, mmu::KeyedFlagRuntime::kRising));
        flags.setConditionOverride(7, false);
        flags.evaluate(nullptr, 0, nullptr, 0);
        check("keyed flag falling",
              flags.query(7, mmu::KeyedFlagRuntime::kFalling) &&
                  !flags.query(7, mmu::KeyedFlagRuntime::kTrue) &&
                  flags.bits(7) == 0x03);
        flags.evaluate(nullptr, 0, nullptr, 0);
        check("keyed flag stable false", flags.bits(7) == 0x01);

        // FUN_00441820 skips records when category & activity mask is zero.
        mmu::KeyedFlagRuntime gated;
        const mmu::RecoveredSaveFlagWriter* keyOne =
            mmu::findSaveFlagWriter(1);
        std::array<mmu::u8, mmu::kKeyedFlagKeyboardLimit> keyboard{};
        keyboard[keyOne->aux] = 1;
        gated.setActivityMask(~keyOne->category);
        gated.evaluate(keyboard.data(), keyboard.size(), nullptr, 0);
        check("keyed flag activity mask skips writer",
              !gated.query(1, mmu::KeyedFlagRuntime::kTrue));
        gated.setActivityMask(keyOne->category);
        gated.evaluate(keyboard.data(), keyboard.size(), nullptr, 0);
        check("keyed flag activity mask enables writer",
              gated.query(1, mmu::KeyedFlagRuntime::kTrue) &&
                  gated.query(1, mmu::KeyedFlagRuntime::kRising));
    }
    {
        mmu::GameFlowRuntime keyedFlow;
        keyedFlow.reset();
        keyedFlow.keyedFlags().setInitialFalse(true);
        keyedFlow.setCompletionFlags(0);
        keyedFlow.setKeyedFlagCondition(7, true);
        keyedFlow.evaluateKeyedFlags(nullptr, 0, nullptr, 0);
        keyedFlow.clearEvents();
        keyedFlow.completeStage(0x99u);
        check("completion from keyed rising edge",
              findStageCompletedEvent(keyedFlow, 0x99u, completed) &&
                  completed.completionFlagId == 7 &&
                  completed.keyedFlagBits == 0x0c &&
                  completed.cutscenePath ==
                      "..\\Cutscenes\\CSN01Intro\\csn01Intro.xcut");
        keyedFlow.evaluateKeyedFlags(nullptr, 0, nullptr, 0);
        keyedFlow.clearEvents();
        keyedFlow.completeStage(0x99u);
        check("completion rising edge consumed",
              findStageCompletedEvent(keyedFlow, 0x99u, completed) &&
                  completed.cutscenePath.empty() &&
                  completed.completionFlagId == 0);
    }

    // FUN_00511350 checks raw completion flags before loader-id routing. The
    // manual bitmask remains a deterministic harness override; the paths come
    // from the binary's literal .xcut strings.
    check("completion cutscene table",
          mmu::kRecoveredCompletionCutsceneCount == 12 &&
              mmu::findCompletionCutsceneFlag(7) != nullptr &&
              mmu::findCompletionCutsceneFlag(0x0Eu) != nullptr);
    {
        const mmu::RecoveredCompletionCutscene* ending =
            mmu::findCompletionCutsceneState(0x100u, false);
        const mmu::RecoveredCompletionCutscene* credits =
            mmu::findCompletionCutsceneState(0x100u, true);
        check("completion helper evidence",
              ending != nullptr && credits != nullptr &&
                  ending->pendingStringVa == 0x6c6964u &&
                  credits->pendingStringVa == 0x6c6a98u &&
                  mmu::kCompletionPendingCallVa == 0x4308c0u &&
                  mmu::kCompletionPendingStringOffset == 0x40u &&
                  mmu::kCompletionPendingFlagOffset == 0x60u &&
                  mmu::kCompletionPendingFlagValue == 1u &&
                  mmu::kCompletionClearCallVa == 0x4308a0u &&
                  mmu::kCompletionClearGlobalVa == 0x6c55f8u &&
                  mmu::kCompletionClearWriteCount == 2 &&
                  mmu::kCompletionClearWrites[0].offset == 0x178u &&
                  mmu::kCompletionClearWrites[0].size == 1 &&
                  mmu::kCompletionClearWrites[1].offset == 0x17cu &&
                  mmu::kCompletionClearWrites[1].size == 4);
    }
    flow.reset();
    flow.setCompletionFlags(1ull << 7);
    flow.clearEvents();
    flow.completeStage(0x99u);
    check("completion cutscene flag 7",
          findStageCompletedEvent(flow, 0x99u, completed) &&
              completed.cutscenePath ==
                  "..\\Cutscenes\\CSN01Intro\\csn01Intro.xcut" &&
              completed.completionFlagId == 7 &&
              completed.managerScreensCleared &&
              completed.completedActionCode == 0);
    flow.setCompletionFlags(1ull << 0x0Eu);
    flow.clearEvents();
    flow.completeStage(0x99u);
    check("completion cutscene flag 0e",
          findStageCompletedEvent(flow, 0x99u, completed) &&
              completed.cutscenePath ==
                  "..\\Cutscenes\\CSN08Epilogue\\CSN08Epilogue.xcut" &&
              completed.completionFlagId == 0x0Eu);

    flow.setCompletionFlags(0);
    mmu::FlowMenuFlags completionFlags = flow.menuFlags();
    completionFlags.flag0x59 = false;
    flow.setMenuFlags(completionFlags);
    flow.clearEvents();
    flow.completeStage(0x100u);
    check("completion state 100 normal",
          findStageCompletedEvent(flow, 0x100u, completed) &&
              completed.cutscenePath ==
                  "..\\Cutscenes\\CSN05Ending\\CSN05Ending.xcut" &&
              completed.completionManagerState == 0x100u &&
              completed.completionWriteCount == 1 &&
              completed.completionWriteOffsets[0] == 0x08 &&
              completed.completionWriteValues[0] == 0x100u &&
              !completed.managerScreensCleared &&
              completed.completionPendingStringVa == 0x6c6964u &&
              completed.completionPendingPath == completed.cutscenePath &&
              completed.completionPendingOffset == 0x40u &&
              completed.completionPendingFlagOffset == 0x60u &&
              completed.completionPendingFlagValue == 1u &&
              completed.completionClearGlobalVa == 0x6c55f8u &&
              completed.completionClearWriteCount == 2 &&
              completed.completionClearWriteOffsets[0] == 0x178u &&
              completed.completionClearWriteSizes[0] == 1 &&
              completed.completionClearWriteOffsets[1] == 0x17cu &&
              completed.completionClearWriteSizes[1] == 4);
    completionFlags.flag0x59 = true;
    flow.setMenuFlags(completionFlags);
    flow.clearEvents();
    flow.completeStage(0x100u);
    check("completion state 100 special",
          findStageCompletedEvent(flow, 0x100u, completed) &&
              completed.cutscenePath ==
                  "..\\Cutscenes\\CSN07Credits\\CSN07Credits.xcut" &&
              completed.completionManagerState == 0x100u &&
              completed.completionPendingStringVa == 0x6c6a98u &&
              completed.completionPendingPath == completed.cutscenePath &&
              completed.completionPendingOffset == 0x40u &&
              completed.completionPendingFlagOffset == 0x60u &&
              completed.completionPendingFlagValue == 1u &&
              completed.completionClearGlobalVa == 0x6c55f8u);
    flow.clearEvents();
    flow.completeStage(0x200u);
    check("completion state 200",
          findStageCompletedEvent(flow, 0x200u, completed) &&
              completed.cutscenePath ==
                  "..\\Cutscenes\\CSN01Intro\\csn01Intro.xcut" &&
              completed.completionManagerState == 0x200u &&
              completed.completionWriteCount == 2 &&
              completed.completionWriteOffsets[0] == 0x08 &&
              completed.completionWriteValues[0] == 0x004u &&
              completed.completionWriteOffsets[1] == 0x30 &&
              completed.completionWriteValues[1] == 0x001u);
    flow.clearEvents();
    flow.completeStage(0x40u);
    check("completion state 40",
          findStageCompletedEvent(flow, 0x40u, completed) &&
              completed.cutscenePath.empty() &&
              completed.completionManagerState == 0x40u &&
              completed.completionWriteCount == 0);

    flow.setScreen(mmu::FlowScreen::OptionsMenu);
    flow.step(1.0f, none);
    check("options slot count", flow.state() == mmu::FlowState::Select &&
                                    flow.menuSupplier() == 0x4FB700u);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    check("options mapping state",
          flow.state() == mmu::FlowState::MapKeyboardInputs);
    flow.step(2.0f, none);
    check("options mapping action",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x39 &&
              actionEvent.label == "MAP KEYBOARD INPUTS" &&
              actionEvent.menuSupplierVa == 0x4FB700u);

    flow.setScreen(mmu::FlowScreen::MenuStageSelect);
    check("stage initial", flow.state() == mmu::FlowState::IntroSlideIn);
    check("stage grid origin", flow.stageSelectStage() == 8);
    flow.step(1.0f, none);
    check("stage intro exit", flow.state() == mmu::FlowState::SelectLevel);
    flow.step(1.0f / 60.0f, down);
    check("stage grid down", flow.stageSelectStage() == 7);
    flow.step(1.0f / 60.0f, right);
    check("stage grid center", flow.stageSelectStage() == 11);

    check("stage grid table",
          mmu::kStageSelectCenterFlagStage == 9 &&
              mmu::kStageSelectCenterCountMin == 1 &&
              mmu::kStageSelectCenterCountMax == 5 &&
              mmu::stageSelectGridOuterStage(0, 0) == 8 &&
              mmu::stageSelectGridOuterStage(2, 2) == 5 &&
              mmu::stageSelectGridOuterStage(1, 1) == -1 &&
              std::size(mmu::kStageSelectCursorKeys) == 4 &&
              mmu::kStageSelectCursorKeys[0].key == 0x1cu &&
              mmu::kStageSelectCursorKeys[0].fieldOffset == 0xba4u &&
              mmu::kStageSelectCursorKeys[0].bound == 2 &&
              mmu::kStageSelectCursorKeys[0].delta == 1 &&
              mmu::kStageSelectCursorKeys[1].key == 0x1bu &&
              mmu::kStageSelectCursorKeys[1].fieldOffset == 0xba4u &&
              mmu::kStageSelectCursorKeys[1].bound == 0 &&
              mmu::kStageSelectCursorKeys[1].delta == -1 &&
              mmu::kStageSelectCursorKeys[2].key == 0x19u &&
              mmu::kStageSelectCursorKeys[2].fieldOffset == 0xba0u &&
              mmu::kStageSelectCursorKeys[2].bound == 2 &&
              mmu::kStageSelectCursorKeys[2].delta == 1 &&
              mmu::kStageSelectCursorKeys[3].key == 0x1au &&
              mmu::kStageSelectCursorKeys[3].fieldOffset == 0xba0u &&
              mmu::kStageSelectCursorKeys[3].bound == 0 &&
              mmu::kStageSelectCursorKeys[3].delta == -1);
    const mmu::FlowMenuFlags savedStageFlags = flow.menuFlags();
    mmu::FlowMenuFlags centerFlags = savedStageFlags;
    centerFlags.subItemCount = 1;
    flow.setMenuFlags(centerFlags);
    check("stage center count1", flow.stageSelectStage() == 10);
    centerFlags.subItemCount = 5;
    flow.setMenuFlags(centerFlags);
    check("stage center count5", flow.stageSelectStage() == 14);
    centerFlags.subItemCount = 6;
    flow.setMenuFlags(centerFlags);
    check("stage center count6", flow.stageSelectStage() == -1);
    centerFlags.subItemCount = 2;
    centerFlags.stageSelectGridFlag0xba8 = true;
    flow.setMenuFlags(centerFlags);
    check("stage center flag", flow.stageSelectStage() == 9);
    flow.setMenuFlags(savedStageFlags);

    flow.step(1.0f / 60.0f, none);
    flow.step(1.0f / 60.0f, right);
    check("stage grid right", flow.stageSelectStage() == 6);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    check("stage confirm", flow.state() == mmu::FlowState::Flash);
    flow.step(1.0f, none);
    int stageValue = 0;
    check("stage event",
          findEventValue(flow, mmu::FlowEventType::StageSelected, stageValue) &&
              stageValue == 6);

    pauseFlags.pauseExitStageVariant = true;
    flow.setMenuFlags(pauseFlags);
    flow.setScreen(mmu::FlowScreen::MenuInGame);
    check("pause menu supplier", flow.menuSupplier() == 0x5081A0u);
    flow.step(1.0f / 60.0f, none);
    flow.clearEvents();
    flow.step(1.0f / 60.0f, confirm);
    check("pause confirm", flow.state() == mmu::FlowState::Flash);
    flow.step(1.0f, none);
    check("pause resume action",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x0eu &&
              actionEvent.label == "RESUME GAME" &&
              actionEvent.menuSupplierVa == 0x5081A0u);

    flow.clearEvents();
    flow.step(1.0f / 60.0f, down);
    check("pause row move", flow.selection() == 1);
    flow.step(1.0f / 60.0f, confirm);
    flow.step(1.0f, none);
    check("pause exit-stage action",
          findMenuActionEvent(flow, actionEvent) &&
              actionEvent.actionCode == 0x0fu &&
              actionEvent.label == "EXIT STAGE");

    flow.setMenuFlags(mmu::FlowMenuFlags{});
    flow.setScreen(mmu::FlowScreen::PauseMenuStatus);
    check("pause cue", flow.lastCue() == mmu::FlowCue::PauseMenu);

    flow.setScreen(mmu::FlowScreen::ShopMenu);
    flow.clearEvents();
    flow.failPurchase(true);
    check("shop full", flow.state() == mmu::FlowState::TooManyItems &&
                          flow.lastCue() == mmu::FlowCue::Wrong);
    flow.step(2.0f, none);
    check("shop recovery", flow.state() == mmu::FlowState::SelectItem);

    flow.clearEvents();
    flow.postLevelCue(mmu::FlowLevelCue::Teleport);
    check("level cue", flow.lastLevelCue() == mmu::FlowLevelCue::Teleport &&
                           hasEvent(flow, mmu::FlowEventType::LevelCue));
    check("level phase exit",
          flow.levelPhase() == mmu::FlowLevelPhase::Exiting &&
              std::string(mmu::flowLevelPhaseName(flow.levelPhase())) == "Exiting(5)");
    flow.postLevelCue(mmu::FlowLevelCue::Victory);
    check("level phase victory", flow.levelPhase() == mmu::FlowLevelPhase::Victory);
    flow.postLevelCue(mmu::FlowLevelCue::Death);
    check("level phase death", flow.levelPhase() == mmu::FlowLevelPhase::Death);

    flow.clearEvents();
    for (int i = 0; i < 48; ++i) flow.postCue(mmu::FlowCue::Select);
    check("event capacity", flow.pendingEventCount() == mmu::GameFlowRuntime::kEventCapacity);

    check("dispatcher hash packed", mmu::stateNameHash("Wait") == 0x57616974u);
    check("dispatcher hash crc",
          mmu::stateNameHash("Confirm") == 0x5e62a7fbu &&
              mmu::stateNameHash("SelectLevel") == 0x8ca7b881u &&
              mmu::stateNameHash("InGame") == 0xce5de53au);

    bool registryValid = mmu::kRegisteredStateNameCount == 266;
    for (const mmu::RegisteredStateName& entry : mmu::kRegisteredStateNames) {
        if (entry.hash != mmu::stateNameHash(entry.name)) registryValid = false;
    }
    check("state registry hashes", registryValid);
    bool flowStatesRegistered = true;
    for (int i = 0; i < static_cast<int>(mmu::FlowState::Count); ++i) {
        const mmu::FlowState state = static_cast<mmu::FlowState>(i);
        if (!mmu::isRegisteredStateName(mmu::flowStateName(state))) {
            flowStatesRegistered = false;
        }
    }
    check("flow states registered", flowStatesRegistered);

    bool transitionsValid = mmu::kRegisteredStateTransitionCount >= 200;
    for (const mmu::RegisteredStateTransition& row :
         mmu::kRegisteredStateTransitions) {
        if (row.targetHash != mmu::stateNameHash(row.targetName)) {
            transitionsValid = false;
        }
        if ((row.flags & 1u) != 0u &&
            !mmu::isRegisteredStateName(row.targetName)) {
            transitionsValid = false;
        }
    }
    check("transition evidence table", transitionsValid);
    check("observed menu transitions",
          mmu::hasRegisteredTransitionTo("SelectLevel") &&
              mmu::hasRegisteredTransitionTo("IntroFadeOut") &&
              mmu::hasRegisteredTransitionTo("WeaponEnergyFill") &&
              mmu::hasRegisteredTransitionTo("TooManyItems"));

    const mmu::RegisteredStateTransition* tooMany =
        mmu::findFirstTransitionTo("TooManyItems", mmu::kFlowTransitionSetState);
    check("flow evidence to-query",
          tooMany != nullptr && tooMany->className == "ShopMenu" &&
              tooMany->vtableSlot == 5 &&
              std::string(mmu::flowTransitionKindName(tooMany->flags)) ==
                  "setState");
    const mmu::RegisteredStateTransition* titleFlash =
        mmu::findFirstTransitionForClass("TitleMenu", 5,
                                         mmu::kFlowTransitionSetState);
    check("flow evidence class-query",
          titleFlash != nullptr && titleFlash->targetName == "Flash" &&
              titleFlash->callVa == 0x4f3078u);
    bool runtimeTargetsObserved = true;
    for (int i = 0; i < static_cast<int>(mmu::FlowState::Count); ++i) {
        const mmu::FlowState state = static_cast<mmu::FlowState>(i);
        const std::string_view name = mmu::flowStateName(state);
        if (!mmu::hasRegisteredStateTransitionTo(name) &&
            !mmu::graphOrdersState("TitleMenu", name) &&
            !mmu::graphOrdersState("ShopMenu", name) &&
            !mmu::graphOrdersState("MenuStageSelect", name)) {
            runtimeTargetsObserved = false;
        }
    }
    check("runtime targets observed", runtimeTargetsObserved);

    bool graphValid = mmu::stateGraphClassCount() == 237 &&
                      mmu::stateGraphRegionCount() == 238 &&
                      mmu::kRegisteredStateGraphCount == 671 &&
                      mmu::kRegisteredStateGraphEdgeCount == 1446;
    for (const mmu::RegisteredStateGraphRow& row : mmu::kRegisteredStateGraph) {
        if (row.targetHash != mmu::stateNameHash(row.stateName))
            graphValid = false;
        if (!mmu::isRegisteredStateName(row.stateName)) graphValid = false;
    }
    for (const mmu::RegisteredStateGraphEdge& edge :
         mmu::kRegisteredStateGraphEdges) {
        if (edge.targetHash != mmu::stateNameHash(edge.targetName))
            graphValid = false;
        if (edge.flags != 1 && edge.flags != 2) graphValid = false;
    }
    check("state graph tables", graphValid);
    // Single-state classes were previously dropped by the analyzer. Their
    // registration, constructor initial state, VAs and vtable edges are all
    // read directly from MMU.exe and now have their own table rows.
    const mmu::RegisteredStateGraphRow* copterMove = nullptr;
    const mmu::RegisteredStateGraphRow* ezWait = nullptr;
    const mmu::RegisteredStateGraphRow* sawPatrol = nullptr;
    for (const mmu::RegisteredStateGraphRow& row : mmu::kRegisteredStateGraph) {
        if (row.className == "CopterCannon" && row.stateName == "Move")
            copterMove = &row;
        if (row.className == "EZBlock" && row.stateName == "Wait")
            ezWait = &row;
        if (row.className == "SawPole" && row.stateName == "Patrol")
            sawPatrol = &row;
    }
    check("single-state class evidence",
          copterMove != nullptr && copterMove->constructorVa == 0x57bb90u &&
              copterMove->vtableVa == 0x692114u &&
              copterMove->targetHash == mmu::stateNameHash("Move") &&
              copterMove->stateCount == 1 && copterMove->isInitial != 0 &&
              ezWait != nullptr && ezWait->constructorVa == 0x614a20u &&
              ezWait->vtableVa == 0x68dcbcu &&
              ezWait->targetHash == mmu::stateNameHash("Wait") &&
              ezWait->stateCount == 1 && ezWait->isInitial != 0 &&
              sawPatrol != nullptr && sawPatrol->constructorVa == 0x5a5870u &&
              sawPatrol->vtableVa == 0x68ff4cu &&
              sawPatrol->targetHash == mmu::stateNameHash("Patrol") &&
              sawPatrol->stateCount == 1 && sawPatrol->isInitial != 0 &&
              mmu::graphEdgeCount("CopterCannon") == 4 &&
              mmu::graphEdgeCount("SawPole") == 1 &&
              mmu::graphEdgeCount("EZBlock") == 0);
    check("projectile state evidence",
          mmu::recoveredInitialState("MMBullet") != nullptr &&
              std::string_view(mmu::recoveredInitialState("MMBullet")) ==
                  "Moving" &&
              mmu::graphEdgeCount("MMBullet") == 7 &&
              mmu::graphOrdersState("ZeroSaber", "Move"));
    check("title initial evidence",
          mmu::graphInitialState(mmu::FlowScreen::TitleMenu) == "IntroFadeIn" &&
              mmu::flowInitialState(mmu::FlowScreen::TitleMenu) ==
                  mmu::FlowState::IntroFadeIn);
    check("menu initial evidence",
          mmu::graphInitialState(mmu::FlowScreen::MainMenu) == "IntroFadeIn" &&
              mmu::graphInitialState(mmu::FlowScreen::MenuStageSelect) ==
                  "IntroSlideIn" &&
              mmu::graphInitialState(mmu::FlowScreen::ShopMenu) == "SlideIn");
    check("enemy initial evidence",
          mmu::graphInitialStateForClass("Telly") == "Idle" &&
              mmu::graphInitialStateForClass("SuperCannon") == "Stand" &&
              mmu::graphInitialStateForClass("MegaMan") == "Wait");
    check("state graph queries",
          mmu::graphOrdersState("Telly", "MoveH") &&
              mmu::graphStateCount("SuperCannon") == 3 &&
              mmu::graphEdgeCount("TitleMenu") >= 4 &&
              mmu::graphInitialStateForClass("NoSuchClass").empty());

    mmu::NamedStateDispatcher dispatcher;
    std::string order;
    int updates = 0;
    dispatcher.registerState("Wait", {
        [&]() { order += "enter:Wait "; },
        [&]() { order += "exit:Wait "; },
        [&]() { ++updates; },
        0.0f, 0xffffffffu, {},
    });
    dispatcher.registerState("Flash", {
        [&]() { order += "enter:Flash "; },
        [&]() {},
        [&]() { ++updates; },
        0.0f, 0xffffffffu, {},
    });
    dispatcher.setState("Wait");
    dispatcher.update();
    dispatcher.setState("Flash");
    check("dispatcher transition",
          order == "enter:Wait exit:Wait enter:Flash " &&
              dispatcher.currentHash() == mmu::stateNameHash("Flash") &&
              dispatcher.previousHash() == mmu::stateNameHash("Wait"));
    check("dispatcher update", updates == 1);
    dispatcher.setEnabled(false);
    dispatcher.update();
    check("dispatcher disable", updates == 1);
    dispatcher.setEnabled(true);

    bool nestedRejected = false;
    dispatcher.registerState("Loop", {
        [&]() { nestedRejected = !dispatcher.setState("Wait"); },
        [&]() {},
        [&]() {},
        0.0f, 0xffffffffu, {},
    });
    dispatcher.setState("Loop");
    check("dispatcher guard", nestedRejected &&
                                  dispatcher.currentHash() == mmu::stateNameHash("Loop"));

    int timerTicks = 0;
    bool timerChangedState = false;
    mmu::StateHooks timerHooks;
    timerHooks.timerSeconds = 0.25f;
    timerHooks.timerMask = 0x1u;
    timerHooks.onTimer = [&]() {
        ++timerTicks;
        timerChangedState = dispatcher.setState("Wait");
    };
    dispatcher.registerState("Timer", timerHooks);
    dispatcher.tickTimers(0.1f, 0x2u);
    dispatcher.tickTimers(0.1f, 0x1u);
    dispatcher.tickTimers(1.0f, 0x1u, true);
    check("dispatcher timer gating", timerTicks == 0);
    dispatcher.tickTimers(0.1f, 0x1u);
    dispatcher.tickTimers(0.1f, 0x1u);
    check("dispatcher timer callback",
          timerTicks == 1 && timerChangedState &&
              dispatcher.currentHash() == mmu::stateNameHash("Wait"));

    mmu::GameFlowRuntime mirrored;
    check("runtime dispatcher init",
          mirrored.dispatcher().currentName() == "IntroFadeIn" &&
              mirrored.dispatcher().currentHash() ==
                  mmu::stateNameHash("IntroFadeIn"));
    mirrored.setScreen(mmu::FlowScreen::MenuStageSelect);
    check("runtime dispatcher screen",
          mirrored.dispatcher().currentName() == "IntroSlideIn" &&
              mirrored.dispatcher().currentHash() ==
                  mmu::stateNameHash("IntroSlideIn"));
    mirrored.setState(mmu::FlowState::Flash);
    check("runtime dispatcher state",
          mirrored.dispatcher().currentName() == "Flash" &&
              mirrored.dispatcher().previousName() == "IntroSlideIn");

    std::printf("checks=%d failures=%d\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
