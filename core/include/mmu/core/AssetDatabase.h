#pragma once

#include "mmu/core/Types.h"

#include <string>
#include <vector>

namespace mmu {

class PackedArchive;
class ProtoReader;

enum class AssetKind {
    Unknown,
    AllSprites,
    Guns,
    Level,
    Cutscenes,
    PNG,
};

// Guess the schema from the original filename. The game itself uses a fixed
// set of eight files, each with one hard-coded top-level message type.
AssetKind kindForFilename(const std::string& filename);
const char* kindName(AssetKind kind);

struct ColorAsset { u8 r = 0, g = 0, b = 0; };

struct SurfaceAsset {
    u32 flags = 0;
    u32 width = 0;
    u32 height = 0;
    u32 pitch = 0;
    bool hasPixelFormat = false;
    u32 bitsPerPixel = 0;
    u32 bytesPerPixel = 0;
    u32 rLoss = 0, gLoss = 0, bLoss = 0, aLoss = 0;
    u32 rShift = 0, gShift = 0, bShift = 0, aShift = 0;
    u32 rMask = 0, gMask = 0, bMask = 0, aMask = 0;
    std::vector<ColorAsset> palette;
    std::vector<u8> pixels;
    u32 pixelChunks = 0; // protobuf stores each scanline as a separate bytes field
};

struct AnimationFrameAsset {
    SurfaceAsset image;
    std::string imagePath;
    float duration = 0.0f;
    i32 xOffset = 0;
    i32 yOffset = 0;
    SurfaceAsset collision;
    std::string collisionPath;
};

struct AnimationAsset {
    i32 mode = 0;
    std::vector<AnimationFrameAsset> frames;
    i32 framesCount = 0;
};

struct SpriteAsset {
    AnimationAsset animation;
    SurfaceAsset singleCollision;
    std::string singleCollisionPath;
    i32 xHandle = 0;
    i32 yHandle = 0;
    std::string resRoot;
    u32 key = 0;
    std::string source;
};

struct GameParamAsset {
    u32 key = 0;
    float param = 0.0f;
};

struct SoundAsset {
    u32 key = 0;
    std::string path;
    i32 allocated = 0;
    std::vector<u8> audioBuffer;
    u32 audioLength = 0;
    u32 volume = 0;
    std::vector<u8> musicStream;
    u32 musicStreamLength = 0;
};

struct GameObjectAsset {
    u32 key = 0;
    std::vector<GameParamAsset> gameParams;
    std::vector<SoundAsset> sounds;
    std::vector<u32> sprites;
    std::vector<u32> spriteKeys;
};

struct SpawnAsset {
    float x = 0.0f;
    float y = 0.0f;
    bool facingRight = false;
    bool respawnOnDeath = false;
    bool respawnOnScreen = false;
    bool deleteOffScreen = false;
    bool collectible = false;
    float autoRespawn = 0.0f;
    std::string type;
    std::string attr1;
    std::string attr2;
};

struct SceneAsset {
    std::vector<SpawnAsset> gameObjects;
    u32 key = 0;
};

struct CommandAsset {
    std::string name;
    std::string id;
    std::string resource;
    i32 x = 0, y = 0, z = 0;
    i32 fadeDelay = 0;
    std::string value;
    bool playThenKill = false;
    std::string anim;
    i32 moveDuration = 0;
    std::string object;
    i32 moveSpeed = 0;
    bool faceRight = false;
    i32 delayBetweenLetters = 0;
};

struct TimeStampAsset {
    std::vector<CommandAsset> commands;
    u32 time = 0;
};

struct CutSceneAsset {
    std::vector<TimeStampAsset> timestamps;
};

struct AssetStats {
    std::size_t sprites = 0;
    std::size_t animationFrames = 0;
    std::size_t surfaces = 0;
    std::size_t pixelBytes = 0;
    std::size_t gameObjects = 0;
    std::size_t sounds = 0;
    std::size_t gameParams = 0;
    std::size_t scenes = 0;
    std::size_t spawns = 0;
    std::size_t cutscenes = 0;
    std::size_t timestamps = 0;
    std::size_t commands = 0;
};

// Holds decoded, platform-neutral asset data. No SDL types, no file handles,
// no protobuf runtime: this object is safe to construct on a console thread.
class AssetDatabase {
public:
    bool load(ByteView bytes, AssetKind kind, const std::string& source = {});
    bool loadFile(const std::string& path);
    bool loadPackedCutscenes(const PackedArchive& pak);
    void clear();

    const std::vector<SpriteAsset>& sprites() const { return sprites_; }
    const std::vector<GameObjectAsset>& gameObjects() const { return gameObjects_; }
    const std::vector<SceneAsset>& scenes() const { return scenes_; }
    const std::vector<CutSceneAsset>& cutscenes() const { return cutscenes_; }
    const std::vector<u32>& cutsceneKeys() const { return cutsceneKeys_; }
    const std::vector<std::string>& cutscenePaths() const { return cutscenePaths_; }
    const std::vector<SurfaceAsset>& standaloneSurfaces() const { return surfaces_; }
    const AssetStats& stats() const { return stats_; }

private:
    bool decodeAllSprites(ProtoReader reader, const std::string& source);
    bool decodeGuns(ProtoReader reader);
    bool decodeLevel(ProtoReader reader);
    bool decodeCutscenes(ProtoReader reader);
    bool decodePng(ProtoReader reader);

    std::vector<SpriteAsset> sprites_;
    std::vector<GameObjectAsset> gameObjects_;
    std::vector<SceneAsset> scenes_;
    std::vector<CutSceneAsset> cutscenes_;
    std::vector<u32> cutsceneKeys_;
    std::vector<std::string> cutscenePaths_;
    std::vector<SurfaceAsset> surfaces_;
    AssetStats stats_{};
};

} // namespace mmu
