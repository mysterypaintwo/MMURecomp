#pragma once

#include "mmu/core/Types.h"

#include <string_view>

namespace mmu {

// ---------------------------------------------------------------------------
// Recovered top-level flow catalogue (evidence only)
//
// This is intentionally NOT a game loop. It records names and vtable anchors
// recovered from MMU.exe so future reconstruction code has a checked source of
// truth. It does not claim transition ordering, timing, or behaviour that has
// not been decompiled. See docs/game_flow.txt and docs/gameplay_reconstruction.md
// for the evidence and its limits.
// ---------------------------------------------------------------------------

enum class FlowScreen : u8 {
    TitleMenu,
    MainMenu,
    NewGameMenu,
    LoadMenu,
    MenuStageSelect,
    OptionsMenu,
    MenuInGame,
    PauseMenuStatus,
    ShopMenu,
    BusterMenu,
    CharacterSelectMenu,
    EndlessAttackMenu,
    EndlessBossAttackMenu,
    GauntletMenu,
    SpecialMenu,
    SuperRobotMastersMenuA,
    SuperRobotMastersMenuB,
    SuperRobotMastersMenuC,
    SuperRobotMastersMenuD,
    WhirlpoolManStageStartMenu,
};

// State names registered by menu constructors or passed to the real menu state
// setter at 0x4405c0. Intro states come from the constructor graph recovered by
// tools/analyze_state_graph.py; the remaining names come from setter call sites.
enum class FlowState : u8 {
    Wait,
    Select,
    Confirm,
    Flash,
    IntroFadeOut,
    SelectDone,
    SelectLevel,
    IntroSlideOut,
    SelectItem,
    SlideOut,
    EnergyFill,
    WeaponEnergyFill,
    NotEnoughBolts,
    TooManyItems,
    MapKeyboardInputs,
    MapJoystickInputs,
    ChangePage,
    IntroFadeIn,
    IntroSlideIn,
    SlideIn,
    Count,
};

// Names passed to the global cue setter at 0x4026b0, plus the cues used by the
// BaseLevel family. A cue is not necessarily a state name.
enum class FlowCue : u8 {
    Select,
    Confirm,
    GameOverLoop,
    TitleScreenA,
    TitleScreenB,
    StageSelect,
    StageSelectBIntro,
    StageSelectBLoop,
    MenuThemeA,
    MenuThemeB,
    MMTeleportOUTOld,
    PauseMenu,
    Wrong,
    EnergyMeter3s,
    Purchase,
    Victory,
    VictoryZ,
    Death,
    Environment,
    Teleport,
    BigExplosion,
};

// Cues observed specifically in BaseLevel/concrete level callbacks.
enum class FlowLevelCue : u8 {
    Victory,
    VictoryZ,
    Death,
    Environment,
    Teleport,
    BigExplosion,
};

// BaseLevel stores an integer phase at +0xb0. Values 0/2/3/5 are observed in
// 0x53b2f0 and 0x539c70; 1 and 4 are not evidenced yet. These are phase values,
// not setState names.
enum class FlowLevelPhase : u8 {
    Active = 0,
    Victory = 2,
    Death = 3,
    Exiting = 5,
};

struct FlowScreenEvidence {
    FlowScreen screen;
    const char* className;
    u32 vftableVa;
    FlowState states[8];
    u8 stateCount;
    FlowCue cues[8];
    u8 cueCount;
};

// States are listed in constructor registration order where that evidence is
// available (docs/state_graph.txt); otherwise they are the observed setter
// targets. kFlowScreens[].initialState is filled from the constructor's final
// setState call by flowInitialState() in GameFlowRuntime.cpp.
inline constexpr FlowScreenEvidence kFlowScreens[] = {
    {FlowScreen::TitleMenu, "TitleMenu", 0x696ad8u,
     {FlowState::IntroFadeIn, FlowState::Wait, FlowState::Flash,
      FlowState::IntroFadeOut}, 4,
     {FlowCue::TitleScreenA, FlowCue::TitleScreenB, FlowCue::Confirm}, 3},

    {FlowScreen::MainMenu, "MainMenu", 0x69691cu,
     {FlowState::IntroFadeIn, FlowState::Select, FlowState::Confirm,
      FlowState::Flash, FlowState::IntroFadeOut}, 5,
     {FlowCue::GameOverLoop, FlowCue::Select, FlowCue::Confirm}, 3},

    {FlowScreen::NewGameMenu, "NewGameMenu", 0x6969acu,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::LoadMenu, "LoadMenu", 0x6968e0u,
     {FlowState::SlideIn, FlowState::SelectItem, FlowState::Confirm,
      FlowState::SlideOut}, 4,
     {FlowCue::Confirm, FlowCue::Select}, 2},

    {FlowScreen::MenuStageSelect, "MenuStageSelect", 0x696974u,
     {FlowState::IntroSlideIn, FlowState::SelectLevel, FlowState::Flash,
      FlowState::IntroSlideOut}, 4,
     {FlowCue::StageSelectBIntro, FlowCue::StageSelect,
      FlowCue::StageSelectBLoop, FlowCue::Confirm, FlowCue::MenuThemeB,
      FlowCue::MMTeleportOUTOld}, 6},

    {FlowScreen::OptionsMenu, "OptionsMenu", 0x6969ccu,
     {FlowState::IntroFadeIn, FlowState::Select, FlowState::Flash,
      FlowState::IntroFadeOut, FlowState::MapKeyboardInputs,
      FlowState::MapJoystickInputs, FlowState::SelectDone}, 7,
     {FlowCue::GameOverLoop, FlowCue::Select, FlowCue::Confirm}, 3},

    // MenuInGame has four slots but no printable states or cues in the scanned
    // windows; keep the evidence entry to avoid inventing transitions.
    {FlowScreen::MenuInGame, "MenuInGame", 0x696960u, {}, 0, {}, 0},

    {FlowScreen::PauseMenuStatus, "PauseMenuStatus", 0x696a2cu,
     {FlowState::SlideIn, FlowState::SelectItem, FlowState::SlideOut,
      FlowState::EnergyFill, FlowState::WeaponEnergyFill}, 5,
     {FlowCue::PauseMenu, FlowCue::Wrong, FlowCue::EnergyMeter3s}, 3},

    {FlowScreen::ShopMenu, "ShopMenu", 0x696a74u,
     {FlowState::SlideIn, FlowState::SelectItem, FlowState::Confirm,
      FlowState::SlideOut, FlowState::NotEnoughBolts,
      FlowState::TooManyItems}, 6,
     {FlowCue::MenuThemeA, FlowCue::Confirm, FlowCue::Select,
      FlowCue::Purchase, FlowCue::Wrong, FlowCue::MMTeleportOUTOld,
      FlowCue::MenuThemeB}, 7},

    {FlowScreen::BusterMenu, "BusterMenu", 0x68dc3cu,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::Confirm}, 2},

    {FlowScreen::CharacterSelectMenu, "CharacterSelectMenu", 0x68dc5cu,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::Confirm}, 2},

    {FlowScreen::EndlessAttackMenu, "EndlessAttackMenu", 0x68dc7cu,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::EndlessBossAttackMenu, "EndlessBossAttackMenu", 0x68dc9cu,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::GauntletMenu, "GauntletMenu", 0x68dd14u,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::SpecialMenu, "SpecialMenu", 0x68ef48u,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::Confirm}, 2},

    {FlowScreen::SuperRobotMastersMenuA, "SuperRobotMastersMenuA", 0x68efe4u,
     {FlowState::Select, FlowState::SelectDone, FlowState::ChangePage}, 3,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::SuperRobotMastersMenuB, "SuperRobotMastersMenuB", 0x68f010u,
     {FlowState::Select, FlowState::SelectDone, FlowState::ChangePage}, 3,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::SuperRobotMastersMenuC, "SuperRobotMastersMenuC", 0x68f03cu,
     {FlowState::Select, FlowState::SelectDone, FlowState::ChangePage}, 3,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::SuperRobotMastersMenuD, "SuperRobotMastersMenuD", 0x68f068u,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},

    {FlowScreen::WhirlpoolManStageStartMenu, "WhirlpoolManStageStartMenu",
     0x68f648u,
     {FlowState::Select, FlowState::SelectDone}, 2,
     {FlowCue::Select, FlowCue::MMTeleportOUTOld, FlowCue::Confirm}, 3},
};

inline constexpr std::size_t kFlowScreenCount =
    sizeof(kFlowScreens) / sizeof(kFlowScreens[0]);

struct FlowLevelClass {
    const char* name;
    u32 vftableVa;
};

// The BaseLevel family recovered by tools/analyze_game_flow.py. Concrete
// stages share most BaseLevel slots and override only a small number of hooks.
inline constexpr FlowLevelClass kFlowLevelClasses[] = {
    {"BaseLevel", 0x696614u},
    {"LevelCometWoman", 0x696640u},
    {"LevelEndless", 0x68de9cu},
    {"LevelEndlessBoss", 0x68dec8u},
    {"LevelGauntlet", 0x68def4u},
    {"LevelGlueMan", 0x69666cu},
    {"LevelJetMan", 0x696698u},
    {"LevelNailMan", 0x6966c4u},
    {"LevelNitroMan", 0x6966f0u},
    {"LevelPerfectCometWoman", 0x68df20u},
    {"LevelPerfectGlueMan", 0x68df4cu},
    {"LevelPerfectJetMan", 0x68df78u},
    {"LevelPerfectNailMan", 0x68dfa4u},
    {"LevelPerfectRainbowMan", 0x68dfd0u},
    {"LevelPerfectTankMan", 0x68dffcu},
    {"LevelPerfectTrinitroMan", 0x68e028u},
    {"LevelPerfectWhirlpoolMan", 0x68e054u},
    {"LevelPerfectYokuMan", 0x68e080u},
    {"LevelPerfectYoyoMan", 0x68e0acu},
    {"LevelRainbowMan", 0x69671cu},
    {"LevelSuperCometWoman", 0x68e0d8u},
    {"LevelSuperGlueMan", 0x68e104u},
    {"LevelSuperJetMan", 0x68e130u},
    {"LevelSuperNailMan", 0x68e15cu},
    {"LevelSuperRainbowMan", 0x68e188u},
    {"LevelSuperTankMan", 0x68e1b4u},
    {"LevelSuperTrinitroMan", 0x68e1e0u},
    {"LevelSuperWhirlpoolMan", 0x68e20cu},
    {"LevelSuperYokuMan", 0x68e238u},
    {"LevelSuperYoyoMan", 0x68e264u},
    {"LevelTankMan", 0x696748u},
    {"LevelWhirlpoolMan", 0x68e290u},
    {"LevelWilyFive", 0x696774u},
    {"LevelWilyFour", 0x6967a0u},
    {"LevelWilyOne", 0x6967ccu},
    {"LevelWilyThree", 0x6967f8u},
    {"LevelWilyTwo", 0x696824u},
    {"LevelYoYoMan", 0x69687cu},
    {"LevelYokuMan", 0x696850u},
};

inline constexpr std::size_t kFlowLevelClassCount =
    sizeof(kFlowLevelClasses) / sizeof(kFlowLevelClasses[0]);

inline constexpr FlowLevelCue kFlowLevelCues[] = {
    FlowLevelCue::Victory, FlowLevelCue::VictoryZ, FlowLevelCue::Death,
    FlowLevelCue::Environment, FlowLevelCue::Teleport,
    FlowLevelCue::BigExplosion,
};

inline constexpr std::size_t kFlowLevelCueCount =
    sizeof(kFlowLevelCues) / sizeof(kFlowLevelCues[0]);

inline const FlowScreenEvidence& flowScreenEvidence(FlowScreen screen) {
    const std::size_t index = static_cast<std::size_t>(screen);
    return index < kFlowScreenCount ? kFlowScreens[index] : kFlowScreens[0];
}

inline const char* flowScreenName(FlowScreen screen) {
    return flowScreenEvidence(screen).className;
}

inline bool flowScreenHasState(FlowScreen screen, FlowState state) {
    const FlowScreenEvidence& evidence = flowScreenEvidence(screen);
    for (u8 i = 0; i < evidence.stateCount; ++i)
        if (evidence.states[i] == state) return true;
    return false;
}

inline bool flowScreenHasCue(FlowScreen screen, FlowCue cue) {
    const FlowScreenEvidence& evidence = flowScreenEvidence(screen);
    for (u8 i = 0; i < evidence.cueCount; ++i)
        if (evidence.cues[i] == cue) return true;
    return false;
}

inline const char* flowStateName(FlowState state) {
    switch (state) {
    case FlowState::Wait: return "Wait";
    case FlowState::Select: return "Select";
    case FlowState::Confirm: return "Confirm";
    case FlowState::Flash: return "Flash";
    case FlowState::IntroFadeOut: return "IntroFadeOut";
    case FlowState::SelectDone: return "SelectDone";
    case FlowState::SelectLevel: return "SelectLevel";
    case FlowState::IntroSlideOut: return "IntroSlideOut";
    case FlowState::SelectItem: return "SelectItem";
    case FlowState::SlideOut: return "SlideOut";
    case FlowState::EnergyFill: return "EnergyFill";
    case FlowState::WeaponEnergyFill: return "WeaponEnergyFill";
    case FlowState::NotEnoughBolts: return "NotEnoughBolts";
    case FlowState::TooManyItems: return "TooManyItems";
    case FlowState::MapKeyboardInputs: return "MapKeyboardInputs";
    case FlowState::MapJoystickInputs: return "MapJoystickInputs";
    case FlowState::ChangePage: return "ChangePage";
    case FlowState::IntroFadeIn: return "IntroFadeIn";
    case FlowState::IntroSlideIn: return "IntroSlideIn";
    case FlowState::SlideIn: return "SlideIn";
    case FlowState::Count: break;
    }
    return "?";
}

inline FlowState flowStateFromName(std::string_view name) {
    for (int i = 0; i < static_cast<int>(FlowState::Count); ++i) {
        const FlowState state = static_cast<FlowState>(i);
        if (name == flowStateName(state)) return state;
    }
    return FlowState::Wait;
}

inline const char* flowCueName(FlowCue cue) {
    switch (cue) {
    case FlowCue::Select: return "Select";
    case FlowCue::Confirm: return "Confirm";
    case FlowCue::GameOverLoop: return "GameOverLoop";
    case FlowCue::TitleScreenA: return "TitleScreenA";
    case FlowCue::TitleScreenB: return "TitleScreenB";
    case FlowCue::StageSelect: return "StageSelect";
    case FlowCue::StageSelectBIntro: return "StageSelectBIntro";
    case FlowCue::StageSelectBLoop: return "StageSelectBLoop";
    case FlowCue::MenuThemeA: return "MenuThemeA";
    case FlowCue::MenuThemeB: return "MenuThemeB";
    case FlowCue::MMTeleportOUTOld: return "MMTeleportOUTOld";
    case FlowCue::PauseMenu: return "PauseMenu";
    case FlowCue::Wrong: return "Wrong";
    case FlowCue::EnergyMeter3s: return "EnergyMeter3s";
    case FlowCue::Purchase: return "Purchase";
    case FlowCue::Victory: return "Victory";
    case FlowCue::VictoryZ: return "VictoryZ";
    case FlowCue::Death: return "Death";
    case FlowCue::Environment: return "Environment";
    case FlowCue::Teleport: return "Teleport";
    case FlowCue::BigExplosion: return "BigExplosion";
    }
    return "?";
}

inline const char* flowLevelCueName(FlowLevelCue cue) {
    return flowCueName(static_cast<FlowCue>(
        static_cast<u8>(FlowCue::Victory) + static_cast<u8>(cue)));
}

inline const char* flowLevelPhaseName(FlowLevelPhase phase) {
    switch (phase) {
    case FlowLevelPhase::Active: return "Active(0)";
    case FlowLevelPhase::Victory: return "Victory(2)";
    case FlowLevelPhase::Death: return "Death(3)";
    case FlowLevelPhase::Exiting: return "Exiting(5)";
    }
    return "?";
}

} // namespace mmu
