#pragma once

#include "mmu/core/AssetDatabase.h"
#include "mmu/core/PackedArchive.h"
#include "mmu/core/Types.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace mmu {

// Command identity in both the original protobuf and legacy .xcut dispatchers
// is the string in Command.name (field 1). There is no numeric opcode table.
enum class CutsceneCommandKind : u8 {
    Unknown,
    DisplayObject,
    GetInGameObject,
    MoveObject,
    PlayAnim,
    KillObject,
    PlaySound,
    StopSound,
    PlayMusicLoop,
    PlayMusicOnce,
    WriteText,
    EraseText,
    EndScene,
};

CutsceneCommandKind cutsceneCommandKind(std::string_view name);
const char* cutsceneCommandName(CutsceneCommandKind kind);

// Portable mirror of the fields the original 0x621e00 dispatcher reads from
// mmu::Command. It is a decoded action, not a renderer or audio implementation.
struct CutsceneEvent {
    CutsceneCommandKind kind = CutsceneCommandKind::Unknown;
    std::string name;
    std::string id;
    std::string resource;
    std::string value;
    std::string anim;
    std::string object;
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
    bool playThenKill = false;
    bool faceRight = false;
    i32 moveDuration = 0;
    i32 moveSpeed = 0;
    i32 delayBetweenLetters = 0;
    u32 timestampMs = 0;
};

struct CutsceneElement {
    std::string id;
    std::string resource;
    std::string anim;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float fadeDelayMs = 0.0f;
    float targetX = 0.0f;
    float targetY = 0.0f;
    float moveSpeed = 0.0f;
    bool moving = false;
    bool playThenKill = false;
    bool faceRight = true;
};

struct CutsceneText {
    std::string id;
    std::string value;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    i32 delayBetweenLetters = 0;
};

// Evidence-driven playback of cutscenedefs.mmu. The original plays commands at
// millisecond timestamps and stops when BaseCutscene+0x34 is set by EndScene.
// This core preserves command order, arguments, element lifetime and text
// state; sound/texture drawing is left to platform code through events.
class CutsceneRuntime {
public:
    static constexpr std::size_t kEventCapacity = 64;

    void attach(const AssetDatabase& assets) { assets_ = &assets; }

    bool startIndex(std::size_t index);
    bool startKey(u32 key);
    bool startPathSuffix(std::string_view suffix);
    void stop();
    void update(float dtSeconds);

    bool playing() const { return playing_; }
    float clockMs() const { return clockMs_; }
    std::size_t currentTimestamp() const { return nextTimestamp_; }
    std::string_view sceneName() const { return sceneName_; }
    const std::vector<CutsceneElement>& elements() const { return elements_; }
    const std::vector<CutsceneText>& texts() const { return texts_; }

    std::size_t pendingEventCount() const { return eventCount_; }
    CutsceneEvent pollEvent();

private:
    void apply(const CommandAsset& command, u32 timestampMs);
    void emit(CutsceneEvent event);
    CutsceneElement* findElement(std::string_view id);
    void removeElement(std::string_view id);

    const AssetDatabase* assets_ = nullptr;
    const CutSceneAsset* scene_ = nullptr;
    std::string sceneName_;
    float clockMs_ = 0.0f;
    std::size_t nextTimestamp_ = 0;
    bool playing_ = false;
    std::vector<CutsceneElement> elements_;
    std::vector<CutsceneText> texts_;
    std::array<CutsceneEvent, kEventCapacity> events_{};
    std::size_t eventHead_ = 0;
    std::size_t eventCount_ = 0;
};

std::size_t cutsceneIndexForKey(const AssetDatabase& assets, u32 key);
std::string_view cutscenePathForIndex(const AssetDatabase& assets, std::size_t index);
std::size_t findCutsceneByPathSuffix(const AssetDatabase& assets,
                                     std::string_view suffix);

// The directory segment of "\Cutscenes\<scene>\<file>.xcut". MMU.exe uses it
// when rebuilding a DisplayObject resource key.
std::string_view cutsceneSceneFolder(std::string_view scenePath);

// Resolved packed image for a cutscene element. DisplayObject supplies
// resource + anim; PlayAnim supplies the anim after the element exists;
// GetInGameObject has only the element id, which is matched against the
// editor-side object path baked from guns.mmu. This mirrors the original chain
// resource path -> GameObject.spriteKeys -> GameObject.sprites -> image.
struct CutsceneSpriteRef {
    static constexpr u32 kNoImage = 0xFFFFFFFFu;

    u32 gameObjectKey = 0;
    u32 spriteKey = 0;
    u32 imageIndex = kNoImage;
    std::string_view gameObjectName;
    std::string_view imageName;

    bool valid() const { return imageIndex != kNoImage; }
};

CutsceneSpriteRef resolveCutsceneSprite(const PackedArchive& pak,
                                        std::string_view scenePath,
                                        std::string_view resource,
                                        std::string_view anim,
                                        std::string_view id = {});

} // namespace mmu
