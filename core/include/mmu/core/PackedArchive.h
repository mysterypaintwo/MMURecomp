#pragma once

#include "mmu/core/Types.h"

#include <string_view>
#include <vector>

namespace mmu {

// The baked archive is a flat, endianness-fixed table of image/audio blobs
// plus sprite/animation, level/spawn and decoded cutscene metadata. It contains
// no protobuf and no SDL types; host and console readers use this one header.
enum class PackedEntryType : u32 {
    Image = 1,
    Audio = 2,
    Music = 3,
};

enum class PackedSource : u32 {
    Misc = 0,
    Levels,
    CutsceneDefs,
    Guns,
    Menus,
    Cutscenes,
    GameObjects,
    LevelArt,
};

const char* packedSourceName(PackedSource source);
const char* packedEntryTypeName(PackedEntryType type);

struct PackedEntry {
    PackedEntryType type = PackedEntryType::Image;
    u32 key = 0;
    PackedSource source = PackedSource::Misc;
    u32 width = 0;
    u32 height = 0;
    u32 pitch = 0;
    u32 bitsPerPixel = 0;
    u32 bytesPerPixel = 0;
    u32 paletteCount = 0;
    u32 pixelChunks = 0;
    u32 rLoss = 0, gLoss = 0, bLoss = 0, aLoss = 0;
    u32 rShift = 0, gShift = 0, bShift = 0, aShift = 0;
    u32 rMask = 0, gMask = 0, bMask = 0, aMask = 0;
    u32 audioLength = 0; // original decoded byte count (0 = use data.size)
    u32 volume = 0;      // 0..255 from the original sound definition
    ByteView data;      // raw SDL pixel rows, or raw audio bytes
    ByteView palette;   // RGB triples, only for 8-bit images
    std::string_view name;
};

struct PackedFrame {
    u32 imageIndex = 0;
    u32 collisionIndex = 0xFFFFFFFFu;
    float duration = 0.0f;
    i32 xOffset = 0;
    i32 yOffset = 0;
};

struct PackedSprite {
    u32 key = 0;
    i32 xHandle = 0;
    i32 yHandle = 0;
    PackedSource source = PackedSource::Misc;
    std::string_view resRoot;
    const PackedFrame* frames = nullptr; // points into PackedArchive::frames()
    u32 frameCount = 0;
};

// One GameSpawnObject from levels.mmu. Positions are in the game's 256x224
// logical screen space (long scenes extend past x=256). Strings point into the
// archive's string table and stay valid as long as the source bytes do.
struct PackedSpawn {
    float x = 0.0f;
    float y = 0.0f;
    float autoRespawn = 0.0f;
    bool facingRight = false;
    bool respawnOnDeath = false;
    bool respawnOnScreen = false;
    bool deleteOffScreen = false;
    bool collectible = false;
    std::string_view type;
    std::string_view attr1;
    std::string_view attr2;
};

struct PackedScene {
    u32 key = 0;
    const PackedSpawn* spawns = nullptr; // points into PackedArchive::spawns()
    u32 spawnCount = 0;
    std::string_view name; // editor-side .xscn resource path, v4 archives only
};

// Decoded mmu::Command from cutscenedefs.mmu. Command identity is the string
// name; there is no numeric opcode table in either original dispatcher.
struct PackedCutsceneCommand {
    std::string_view name;
    std::string_view id;
    std::string_view resource;
    std::string_view value;
    std::string_view anim;
    std::string_view object;
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
    i32 fadeDelay = 0;
    i32 moveDuration = 0;
    i32 moveSpeed = 0;
    i32 delayBetweenLetters = 0;
    bool playThenKill = false;
    bool faceRight = false;
};

struct PackedCutsceneTimestamp {
    u32 time = 0;
    const PackedCutsceneCommand* commands = nullptr;
    u32 commandCount = 0;
};

struct PackedCutscene {
    u32 key = 0;
    std::string_view path;
    const PackedCutsceneTimestamp* timestamps = nullptr;
    u32 timestampCount = 0;
};

// One GameSpawnObject definition from guns.mmu. The key is the CRC-32 of its
// original .xgun resource path (for cutscene objects,
// "..\Cutscenes\<scene>\<resource>"). The name is a best-effort editor-side
// image path from the first sprite, used only to resolve GetInGameObject ids.
struct PackedGameObject {
    u32 key = 0;
    u32 nameOffset = 0;
    u32 firstSprite = 0;
    u32 spriteCount = 0;
    std::string_view name;
};

// guns.mmu stores parallel sprites and spriteKeys arrays. spriteKeys[i] is the
// CRC-32 of the animation name used by Cutscene PlayAnim; sprites[i] is the
// packed sprite key selected by that animation.
struct PackedGameObjectSprite {
    u32 animKey = 0;
    u32 spriteKey = 0;
    u32 spriteIndex = 0xFFFFFFFFu;
};

class PackedArchive {
public:
    static constexpr u64 kMagic = 0x024B4150554D4DULL; // "MMUPAK"
    static constexpr u32 kVersion = 5;                 // v2/v3/v4 archives are still accepted

    bool load(ByteView bytes);
    void clear();
    bool valid() const { return valid_; }
    u32 version() const { return version_; }
    const std::vector<PackedEntry>& entries() const { return entries_; }
    const std::vector<PackedSprite>& sprites() const { return sprites_; }
    const std::vector<PackedFrame>& frames() const { return frames_; }
    const std::vector<PackedScene>& scenes() const { return scenes_; }
    const std::vector<PackedSpawn>& spawns() const { return spawns_; }
    const std::vector<PackedCutscene>& cutscenes() const { return cutscenes_; }
    const std::vector<PackedCutsceneTimestamp>& cutsceneTimestamps() const {
        return cutsceneTimestamps_;
    }
    const std::vector<PackedCutsceneCommand>& cutsceneCommands() const {
        return cutsceneCommands_;
    }
    const std::vector<PackedGameObject>& gameObjects() const {
        return gameObjects_;
    }
    const std::vector<PackedGameObjectSprite>& gameObjectSprites() const {
        return gameObjectSprites_;
    }

    const PackedEntry* find(u32 key, PackedEntryType type) const;
    const PackedSprite* findSprite(u32 key) const;
    const PackedScene* findScene(u32 key) const;
    const PackedGameObject* findGameObject(u32 key) const;
    std::size_t imageCount() const;
    std::string_view stringAt(u32 offset) const;

private:
    std::vector<PackedEntry> entries_;
    std::vector<PackedSprite> sprites_;
    std::vector<PackedFrame> frames_;
    std::vector<PackedScene> scenes_;
    std::vector<PackedSpawn> spawns_;
    std::vector<PackedCutscene> cutscenes_;
    std::vector<PackedCutsceneTimestamp> cutsceneTimestamps_;
    std::vector<PackedCutsceneCommand> cutsceneCommands_;
    std::vector<PackedGameObject> gameObjects_;
    std::vector<PackedGameObjectSprite> gameObjectSprites_;
    ByteView strings_{};
    u32 version_ = 0;
    bool valid_ = false;
};

} // namespace mmu
