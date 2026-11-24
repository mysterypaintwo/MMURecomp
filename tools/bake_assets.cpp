// Offline asset baker: original .mmu protobuf databases -> one flat MMUPAK v5.
// The runtime never needs protobuf or the original files.
//
//   mmu_bake <output.pak> <asset_dir> [file.mmu ...] [--max-images N]

#include "mmu/core/AssetDatabase.h"
#include "mmu/core/PackedArchive.h"
#include "mmu/core/RegisteredCutscenes.h"

#include "scene_names.inc"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace {

using namespace mmu;

constexpr u32 kNoIndex = 0xFFFFFFFFu;
constexpr u64 kHeaderSize = 192;
constexpr u64 kEntrySize = 136;
constexpr u64 kSpriteSize = 40;
constexpr u64 kFrameSize = 24;
constexpr u64 kSceneSize = 24;
constexpr u64 kSpawnSize = 32;
constexpr u64 kCutsceneSize = 16;
constexpr u64 kCutsceneTimestampSize = 16;
constexpr u64 kCutsceneCommandSize = 72;
constexpr u64 kGameObjectSize = 16;
constexpr u64 kGameObjectSpriteSize = 12;

struct Header {
    u64 magic = PackedArchive::kMagic;
    u32 version = PackedArchive::kVersion;
    u32 entryCount = 0;
    u64 entriesOffset = 0;
    u64 stringsOffset = 0;
    u64 stringsSize = 0;
    u64 dataOffset = 0;
    u64 dataSize = 0;
    u64 spriteTableOffset = 0;
    u64 frameTableOffset = 0;
    u32 spriteCount = 0;
    u32 frameCount = 0;
    u32 flags = 0;
    u32 reserved = 0;
    u64 sceneTableOffset = 0;
    u64 spawnTableOffset = 0;
    u32 sceneCount = 0;
    u32 spawnCount = 0;
    u32 reserved2 = 0;
    u32 reserved3 = 0;
    u64 cutsceneTableOffset = 0; // occupies the v4 reserved4 slot
    u64 cutsceneTimestampTableOffset = 0;
    u64 cutsceneCommandTableOffset = 0;
    u32 cutsceneCount = 0;
    u32 cutsceneTimestampCount = 0;
    u32 cutsceneCommandCount = 0;
    // v5 extension: the resolved guns.mmu object/animation table. Older v5
    // archives leave gameObjectCount at 0 and are still accepted.
    u32 gameObjectCount = 0;
    u64 gameObjectTableOffset = 0;
    u64 gameObjectSpriteTableOffset = 0;
    u64 reserved8 = 0;
    u64 reserved9 = 0;
};
static_assert(sizeof(Header) == kHeaderSize, "MMUPAK header layout");

struct PendingEntry {
    PackedEntry info;
    std::vector<u8> data;
    std::vector<u8> palette;
    std::string name;
    u64 dataOffset = 0;
    u64 paletteOffset = 0;
    u64 nameOffset = 0;
};

struct PendingFrame {
    u32 imageIndex = 0;
    u32 collisionIndex = kNoIndex;
    float duration = 0.0f;
    i32 xOffset = 0;
    i32 yOffset = 0;
};

struct PendingSprite {
    u32 key = 0;
    u32 firstFrame = 0;
    u32 frameCount = 0;
    i32 xHandle = 0;
    i32 yHandle = 0;
    u32 nameOffset = 0;
    PackedSource source = PackedSource::Misc;
};

struct PendingSpawn {
    float x = 0.0f, y = 0.0f, autoRespawn = 0.0f;
    u32 flags = 0;
    std::string type, attr1, attr2;
    u32 typeOffset = 0, attr1Offset = 0, attr2Offset = 0;
};

struct PendingScene {
    u32 key = 0;
    u32 firstSpawn = 0;
    u32 spawnCount = 0;
    std::string name;
    u32 nameOffset = 0;
};

struct PendingCutsceneCommand {
    std::string name, id, resource, value, anim, object;
    i32 x = 0, y = 0, z = 0;
    i32 fadeDelay = 0, moveDuration = 0, moveSpeed = 0, delayBetweenLetters = 0;
    bool playThenKill = false, faceRight = false;
    u32 nameOffset = 0, idOffset = 0, resourceOffset = 0;
    u32 valueOffset = 0, animOffset = 0, objectOffset = 0;
};

struct PendingCutsceneTimestamp {
    u32 time = 0;
    u32 firstCommand = 0;
    u32 commandCount = 0;
};

struct PendingCutscene {
    u32 key = 0;
    u32 firstTimestamp = 0;
    u32 timestampCount = 0;
    std::string path;
    u32 pathOffset = 0;
};

struct PendingGameObject {
    u32 key = 0;
    u32 nameOffset = 0;
    u32 firstSprite = 0;
    u32 spriteCount = 0;
    std::string name;
};

struct PendingGameObjectSprite {
    u32 animKey = 0;
    u32 spriteKey = 0;
    u32 spriteIndex = static_cast<u32>(-1);
};

void putU32(std::vector<u8>& out, u32 value) {
    for (unsigned i = 0; i < 4; ++i)
        out.push_back(static_cast<u8>(value >> (i * 8)));
}

void putI32(std::vector<u8>& out, i32 value) {
    putU32(out, static_cast<u32>(value));
}

void putU64(std::vector<u8>& out, u64 value) {
    for (unsigned i = 0; i < 8; ++i)
        out.push_back(static_cast<u8>(value >> (i * 8)));
}

void putFloat(std::vector<u8>& out, float value) {
    u32 bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    putU32(out, bits);
}

void fillSurface(PendingEntry& entry, const SurfaceAsset& surface) {
    entry.info.width = surface.width;
    entry.info.height = surface.height;
    entry.info.pitch = surface.pitch;
    entry.info.bitsPerPixel = surface.bitsPerPixel;
    entry.info.bytesPerPixel = surface.bytesPerPixel;
    entry.info.paletteCount = static_cast<u32>(surface.palette.size());
    entry.info.pixelChunks = surface.pixelChunks;
    entry.info.rLoss = surface.rLoss;
    entry.info.gLoss = surface.gLoss;
    entry.info.bLoss = surface.bLoss;
    entry.info.aLoss = surface.aLoss;
    entry.info.rShift = surface.rShift;
    entry.info.gShift = surface.gShift;
    entry.info.bShift = surface.bShift;
    entry.info.aShift = surface.aShift;
    entry.info.rMask = surface.rMask;
    entry.info.gMask = surface.gMask;
    entry.info.bMask = surface.bMask;
    entry.info.aMask = surface.aMask;
    entry.data = surface.pixels;
    for (const ColorAsset& color : surface.palette) {
        entry.palette.push_back(color.r);
        entry.palette.push_back(color.g);
        entry.palette.push_back(color.b);
    }
}

PackedSource sourceFor(const std::string& filename) {
    if (filename == "misc.mmu") return PackedSource::Misc;
    if (filename == "levels.mmu") return PackedSource::Levels;
    if (filename == "cutscenedefs.mmu") return PackedSource::CutsceneDefs;
    if (filename == "guns.mmu") return PackedSource::Guns;
    if (filename == "menus.mmu") return PackedSource::Menus;
    if (filename == "cutscenes.mmu") return PackedSource::Cutscenes;
    if (filename == "gameobjects.mmu") return PackedSource::GameObjects;
    if (filename == "levelart.mmu") return PackedSource::LevelArt;
    return PackedSource::Misc;
}

std::size_t addSurface(std::vector<PendingEntry>& entries, const SurfaceAsset& surface,
                       const std::string& name, u32 key, PackedSource source) {
    if (surface.pixels.empty()) return kNoIndex;
    PendingEntry entry;
    entry.info.type = PackedEntryType::Image;
    entry.info.key = key;
    entry.info.source = source;
    entry.name = name;
    fillSurface(entry, surface);
    entries.push_back(std::move(entry));
    return entries.size() - 1;
}

void addBytes(std::vector<PendingEntry>& entries, const std::vector<u8>& bytes,
              const std::string& name, u32 key, PackedSource source, PackedEntryType type,
              u32 audioLength, u32 volume) {
    if (bytes.empty()) return;
    PendingEntry entry;
    entry.info.type = type;
    entry.info.key = key;
    entry.info.source = source;
    entry.info.width = static_cast<u32>(bytes.size());
    entry.info.audioLength = audioLength;
    entry.info.volume = volume;
    entry.name = name;
    entry.data = bytes;
    entries.push_back(std::move(entry));
}

bool writeArchive(const std::string& path, std::vector<PendingEntry>& entries,
                  std::vector<PendingFrame>& frames, std::vector<PendingSprite>& sprites,
                  std::vector<std::string>& spriteRoots,
                  std::vector<PendingScene>& scenes, std::vector<PendingSpawn>& spawns,
                  std::vector<PendingCutscene>& cutscenes,
                  std::vector<PendingCutsceneTimestamp>& cutsceneTimestamps,
                  std::vector<PendingCutsceneCommand>& cutsceneCommands,
                  std::vector<PendingGameObject>& gameObjects,
                  std::vector<PendingGameObjectSprite>& gameObjectSprites) {
    Header header;
    header.entryCount = static_cast<u32>(entries.size());
    header.frameCount = static_cast<u32>(frames.size());
    header.spriteCount = static_cast<u32>(sprites.size());
    header.sceneCount = static_cast<u32>(scenes.size());
    header.spawnCount = static_cast<u32>(spawns.size());
    header.cutsceneCount = static_cast<u32>(cutscenes.size());
    header.cutsceneTimestampCount = static_cast<u32>(cutsceneTimestamps.size());
    header.cutsceneCommandCount = static_cast<u32>(cutsceneCommands.size());
    header.gameObjectCount = static_cast<u32>(gameObjects.size());
    header.entriesOffset = kHeaderSize;
    header.frameTableOffset = header.entriesOffset + static_cast<u64>(entries.size()) * kEntrySize;
    header.spriteTableOffset = header.frameTableOffset + static_cast<u64>(frames.size()) * kFrameSize;
    header.sceneTableOffset = header.spriteTableOffset + static_cast<u64>(sprites.size()) * kSpriteSize;
    header.spawnTableOffset = header.sceneTableOffset + static_cast<u64>(scenes.size()) * kSceneSize;

    std::vector<u8> strings;
    strings.push_back(0); // offset 0 = empty string
    const auto intern = [&](const std::string& text) -> u32 {
        const u32 offset = static_cast<u32>(strings.size());
        strings.insert(strings.end(), text.begin(), text.end());
        strings.push_back(0);
        return offset;
    };
    for (PendingEntry& entry : entries)
        entry.nameOffset = intern(entry.name);
    for (std::size_t i = 0; i < sprites.size() && i < spriteRoots.size(); ++i)
        sprites[i].nameOffset = intern(spriteRoots[i]);
    for (PendingScene& scene : scenes)
        scene.nameOffset = intern(scene.name);
    for (PendingSpawn& spawn : spawns) {
        spawn.typeOffset = intern(spawn.type);
        spawn.attr1Offset = intern(spawn.attr1);
        spawn.attr2Offset = intern(spawn.attr2);
    }
    for (PendingCutscene& cutscene : cutscenes)
        cutscene.pathOffset = intern(cutscene.path);
    for (PendingCutsceneCommand& command : cutsceneCommands) {
        command.nameOffset = intern(command.name);
        command.idOffset = intern(command.id);
        command.resourceOffset = intern(command.resource);
        command.valueOffset = intern(command.value);
        command.animOffset = intern(command.anim);
        command.objectOffset = intern(command.object);
    }
    for (PendingGameObject& object : gameObjects)
        object.nameOffset = intern(object.name);

    header.cutsceneTableOffset = header.spawnTableOffset +
                                 static_cast<u64>(spawns.size()) * kSpawnSize;
    header.cutsceneTimestampTableOffset = header.cutsceneTableOffset +
                                          static_cast<u64>(cutscenes.size()) * kCutsceneSize;
    header.cutsceneCommandTableOffset = header.cutsceneTimestampTableOffset +
                                        static_cast<u64>(cutsceneTimestamps.size()) *
                                            kCutsceneTimestampSize;
    header.gameObjectTableOffset = header.cutsceneCommandTableOffset +
                                   static_cast<u64>(cutsceneCommands.size()) *
                                       kCutsceneCommandSize;
    header.gameObjectSpriteTableOffset = header.gameObjectTableOffset +
                                         static_cast<u64>(gameObjects.size()) *
                                             kGameObjectSize;
    header.stringsOffset = header.gameObjectSpriteTableOffset +
                           static_cast<u64>(gameObjectSprites.size()) *
                               kGameObjectSpriteSize;
    header.stringsSize = strings.size();
    header.dataOffset = header.stringsOffset + header.stringsSize;

    std::vector<u8> blobs;
    for (PendingEntry& entry : entries) {
        entry.dataOffset = static_cast<u64>(blobs.size());
        blobs.insert(blobs.end(), entry.data.begin(), entry.data.end());
        if (!entry.palette.empty()) {
            entry.paletteOffset = static_cast<u64>(blobs.size());
            blobs.insert(blobs.end(), entry.palette.begin(), entry.palette.end());
        }
    }
    header.dataSize = blobs.size();

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(&header), kHeaderSize);

    for (const PendingEntry& entry : entries) {
        std::vector<u8> record;
        record.reserve(kEntrySize);
        putU32(record, static_cast<u32>(entry.info.type));
        putU32(record, entry.info.key);
        putU32(record, static_cast<u32>(entry.info.source));
        putU32(record, entry.info.width);
        putU32(record, entry.info.height);
        putU32(record, entry.info.pitch);
        putU32(record, entry.info.bitsPerPixel);
        putU32(record, entry.info.bytesPerPixel);
        putU32(record, entry.info.paletteCount);
        putU32(record, entry.info.pixelChunks);
        putU32(record, entry.info.rLoss);
        putU32(record, entry.info.gLoss);
        putU32(record, entry.info.bLoss);
        putU32(record, entry.info.aLoss);
        putU32(record, entry.info.rShift);
        putU32(record, entry.info.gShift);
        putU32(record, entry.info.bShift);
        putU32(record, entry.info.aShift);
        putU32(record, entry.info.rMask);
        putU32(record, entry.info.gMask);
        putU32(record, entry.info.bMask);
        putU32(record, entry.info.aMask);
        putU64(record, entry.dataOffset);
        putU64(record, entry.data.size());
        putU64(record, entry.palette.empty() ? 0 : entry.paletteOffset);
        putU64(record, entry.palette.size());
        putU64(record, entry.nameOffset);
        putU32(record, entry.info.audioLength);
        putU32(record, entry.info.volume);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingFrame& frame : frames) {
        std::vector<u8> record;
        record.reserve(kFrameSize);
        putU32(record, frame.imageIndex);
        putU32(record, frame.collisionIndex);
        putFloat(record, frame.duration);
        putI32(record, frame.xOffset);
        putI32(record, frame.yOffset);
        putU32(record, 0);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingSprite& sprite : sprites) {
        std::vector<u8> record;
        record.reserve(kSpriteSize);
        putU32(record, sprite.key);
        putU32(record, sprite.firstFrame);
        putU32(record, sprite.frameCount);
        putI32(record, sprite.xHandle);
        putI32(record, sprite.yHandle);
        putU32(record, sprite.nameOffset);
        putU32(record, static_cast<u32>(sprite.source));
        putU32(record, 0);
        putU64(record, 0);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingScene& scene : scenes) {
        std::vector<u8> record;
        record.reserve(kSceneSize);
        putU32(record, scene.key);
        putU32(record, scene.firstSpawn);
        putU32(record, scene.spawnCount);
        putU32(record, scene.nameOffset);
        putU32(record, 0);
        putU32(record, 0);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingSpawn& spawn : spawns) {
        std::vector<u8> record;
        record.reserve(kSpawnSize);
        putFloat(record, spawn.x);
        putFloat(record, spawn.y);
        putFloat(record, spawn.autoRespawn);
        putU32(record, spawn.flags);
        putU32(record, spawn.typeOffset);
        putU32(record, spawn.attr1Offset);
        putU32(record, spawn.attr2Offset);
        putU32(record, 0);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingCutscene& cutscene : cutscenes) {
        std::vector<u8> record;
        record.reserve(kCutsceneSize);
        putU32(record, cutscene.key);
        putU32(record, cutscene.firstTimestamp);
        putU32(record, cutscene.timestampCount);
        putU32(record, cutscene.pathOffset);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingCutsceneTimestamp& stamp : cutsceneTimestamps) {
        std::vector<u8> record;
        record.reserve(kCutsceneTimestampSize);
        putU32(record, stamp.time);
        putU32(record, stamp.firstCommand);
        putU32(record, stamp.commandCount);
        putU32(record, 0);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingCutsceneCommand& command : cutsceneCommands) {
        std::vector<u8> record;
        record.reserve(kCutsceneCommandSize);
        putU32(record, command.nameOffset);
        putU32(record, command.idOffset);
        putU32(record, command.resourceOffset);
        putU32(record, command.valueOffset);
        putU32(record, command.animOffset);
        putU32(record, command.objectOffset);
        putI32(record, command.x);
        putI32(record, command.y);
        putI32(record, command.z);
        putI32(record, command.fadeDelay);
        putI32(record, command.moveDuration);
        putI32(record, command.moveSpeed);
        putI32(record, command.delayBetweenLetters);
        putU32(record, (command.playThenKill ? 1u : 0u) |
                       (command.faceRight ? 2u : 0u));
        for (int i = 0; i < 4; ++i) putU32(record, 0);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingGameObject& object : gameObjects) {
        std::vector<u8> record;
        record.reserve(kGameObjectSize);
        putU32(record, object.key);
        putU32(record, object.nameOffset);
        putU32(record, object.firstSprite);
        putU32(record, object.spriteCount);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    for (const PendingGameObjectSprite& link : gameObjectSprites) {
        std::vector<u8> record;
        record.reserve(kGameObjectSpriteSize);
        putU32(record, link.animKey);
        putU32(record, link.spriteKey);
        putU32(record, link.spriteIndex);
        out.write(reinterpret_cast<const char*>(record.data()), record.size());
    }

    out.write(reinterpret_cast<const char*>(strings.data()), strings.size());
    out.write(reinterpret_cast<const char*>(blobs.data()), blobs.size());
    return out.good();
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: mmu_bake <output.pak> <asset_dir> [file.mmu ...] [--max-images N]\n");
        return 2;
    }
    const std::string output = argv[1];
    const std::string assetDir = argv[2];

    std::vector<std::string> files;
    std::size_t maxImages = 0;
    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--max-images" && i + 1 < argc) {
            maxImages = static_cast<std::size_t>(std::strtoul(argv[++i], nullptr, 0));
        } else {
            files.push_back(arg);
        }
    }
    if (files.empty()) {
        files = {"misc.mmu", "levels.mmu", "cutscenedefs.mmu", "guns.mmu",
                 "menus.mmu", "cutscenes.mmu", "gameobjects.mmu", "levelart.mmu"};
    }

    std::vector<PendingEntry> entries;
    std::vector<PendingFrame> frames;
    std::vector<PendingSprite> sprites;
    std::vector<std::string> spriteRoots;
    std::vector<PendingScene> scenes;
    std::vector<PendingSpawn> spawns;
    std::vector<PendingCutscene> cutscenes;
    std::vector<PendingCutsceneTimestamp> cutsceneTimestamps;
    std::vector<PendingCutsceneCommand> cutsceneCommands;
    std::vector<PendingGameObject> gameObjects;
    std::vector<PendingGameObjectSprite> gameObjectSprites;

    for (const std::string& filename : files) {
        const std::string path = assetDir + "/" + filename;
        AssetDatabase db;
        if (!db.loadFile(path)) {
            std::fprintf(stderr, "warning: could not decode %s\n", path.c_str());
            continue;
        }
        const PackedSource source = sourceFor(filename);
        const std::size_t before = entries.size();

        for (const SpriteAsset& sprite : db.sprites()) {
            PendingSprite packedSprite;
            packedSprite.key = sprite.key;
            packedSprite.xHandle = sprite.xHandle;
            packedSprite.yHandle = sprite.yHandle;
            packedSprite.source = source;
            packedSprite.firstFrame = static_cast<u32>(frames.size());

            for (const AnimationFrameAsset& frame : sprite.animation.frames) {
                PendingFrame packedFrame;
                packedFrame.imageIndex = static_cast<u32>(addSurface(
                    entries, frame.image, frame.imagePath, sprite.key, source));
                packedFrame.collisionIndex = addSurface(
                    entries, frame.collision, frame.collisionPath, sprite.key, source);
                packedFrame.duration = frame.duration;
                packedFrame.xOffset = frame.xOffset;
                packedFrame.yOffset = frame.yOffset;
                frames.push_back(packedFrame);
            }
            packedSprite.frameCount = static_cast<u32>(frames.size() - packedSprite.firstFrame);
            addSurface(entries, sprite.singleCollision, sprite.singleCollisionPath, sprite.key, source);
            sprites.push_back(packedSprite);
            spriteRoots.push_back(sprite.resRoot);
        }
        for (std::size_t i = 0; i < db.standaloneSurfaces().size(); ++i) {
            addSurface(entries, db.standaloneSurfaces()[i], filename + "#" + std::to_string(i),
                       static_cast<u32>(i), source);
        }
        for (const GameObjectAsset& object : db.gameObjects()) {
            for (const SoundAsset& sound : object.sounds) {
                addBytes(entries, sound.audioBuffer, sound.path, sound.key, source,
                         PackedEntryType::Audio, sound.audioLength, sound.volume);
                addBytes(entries, sound.musicStream, sound.path + "#stream", sound.key,
                         source, PackedEntryType::Music, sound.musicStreamLength, sound.volume);
            }
            PendingGameObject packedObject;
            packedObject.key = object.key;
            packedObject.firstSprite = static_cast<u32>(gameObjectSprites.size());
            const std::size_t linkCount =
                std::min(object.sprites.size(), object.spriteKeys.size());
            for (std::size_t i = 0; i < linkCount; ++i) {
                PendingGameObjectSprite link;
                link.animKey = object.spriteKeys[i];
                link.spriteKey = object.sprites[i];
                gameObjectSprites.push_back(link);
            }
            packedObject.spriteCount =
                static_cast<u32>(gameObjectSprites.size() - packedObject.firstSprite);
            gameObjects.push_back(std::move(packedObject));
        }
        for (const SceneAsset& scene : db.scenes()) {
            PendingScene packedScene;
            packedScene.key = scene.key;
            packedScene.firstSpawn = static_cast<u32>(spawns.size());
            for (const SpawnAsset& spawn : scene.gameObjects) {
                PendingSpawn packedSpawn;
                packedSpawn.x = spawn.x;
                packedSpawn.y = spawn.y;
                packedSpawn.autoRespawn = spawn.autoRespawn;
                packedSpawn.flags = (spawn.facingRight ? 1u : 0u)
                                  | (spawn.respawnOnDeath ? 2u : 0u)
                                  | (spawn.respawnOnScreen ? 4u : 0u)
                                  | (spawn.deleteOffScreen ? 8u : 0u)
                                  | (spawn.collectible ? 16u : 0u);
                packedSpawn.type = spawn.type;
                packedSpawn.attr1 = spawn.attr1;
                packedSpawn.attr2 = spawn.attr2;
                spawns.push_back(std::move(packedSpawn));
            }
            packedScene.spawnCount = static_cast<u32>(spawns.size() - packedScene.firstSpawn);
            if (scenes.size() < sizeof(kSceneNames) / sizeof(kSceneNames[0]))
                packedScene.name = kSceneNames[scenes.size()];
            scenes.push_back(packedScene);
        }
        if (source == PackedSource::CutsceneDefs) {
            for (std::size_t i = 0; i < db.cutscenes().size(); ++i) {
                PendingCutscene packedCutscene;
                packedCutscene.key = i < db.cutsceneKeys().size()
                                         ? db.cutsceneKeys()[i] : 0;
                packedCutscene.path = std::string(cutscenePathForKey(packedCutscene.key));
                packedCutscene.firstTimestamp =
                    static_cast<u32>(cutsceneTimestamps.size());
                for (const TimeStampAsset& stamp : db.cutscenes()[i].timestamps) {
                    PendingCutsceneTimestamp packedStamp;
                    packedStamp.time = stamp.time;
                    packedStamp.firstCommand =
                        static_cast<u32>(cutsceneCommands.size());
                    for (const CommandAsset& command : stamp.commands) {
                        PendingCutsceneCommand packedCommand;
                        packedCommand.name = command.name;
                        packedCommand.id = command.id;
                        packedCommand.resource = command.resource;
                        packedCommand.value = command.value;
                        packedCommand.anim = command.anim;
                        packedCommand.object = command.object;
                        packedCommand.x = command.x;
                        packedCommand.y = command.y;
                        packedCommand.z = command.z;
                        packedCommand.fadeDelay = command.fadeDelay;
                        packedCommand.moveDuration = command.moveDuration;
                        packedCommand.moveSpeed = command.moveSpeed;
                        packedCommand.delayBetweenLetters = command.delayBetweenLetters;
                        packedCommand.playThenKill = command.playThenKill;
                        packedCommand.faceRight = command.faceRight;
                        cutsceneCommands.push_back(std::move(packedCommand));
                    }
                    packedStamp.commandCount =
                        static_cast<u32>(cutsceneCommands.size()) -
                        packedStamp.firstCommand;
                    cutsceneTimestamps.push_back(packedStamp);
                }
                packedCutscene.timestampCount =
                    static_cast<u32>(cutsceneTimestamps.size()) -
                    packedCutscene.firstTimestamp;
                cutscenes.push_back(std::move(packedCutscene));
            }
        }
        std::printf("%-20s -> %zu images, %zu sprites, %zu scenes, %zu spawns\n",
                    filename.c_str(), entries.size() - before, sprites.size(),
                    scenes.size(), spawns.size());
        std::fflush(stdout);

        if (maxImages && entries.size() >= maxImages) {
            entries.resize(maxImages);
            // Drop frames/sprites that reference truncated image entries.
            frames.clear();
            sprites.clear();
            spriteRoots.clear();
            scenes.clear();
            spawns.clear();
            gameObjects.clear();
            gameObjectSprites.clear();
            break;
        }
    }

    // guns.mmu is decoded before the sprite archives, so resolve the
    // object -> sprite and object -> editor path links after all files are in.
    for (PendingGameObject& object : gameObjects) {
        if (object.firstSprite >= gameObjectSprites.size()) continue;
        const u32 firstSpriteKey =
            gameObjectSprites[object.firstSprite].spriteKey;
        for (const PendingEntry& entry : entries) {
            if (entry.info.type != PackedEntryType::Image ||
                entry.info.key != firstSpriteKey)
                continue;
            object.name = entry.name;
            break;
        }
        const std::size_t end = std::min<std::size_t>(
            gameObjectSprites.size(),
            static_cast<std::size_t>(object.firstSprite) + object.spriteCount);
        for (std::size_t i = object.firstSprite; i < end; ++i) {
            const u32 spriteKey = gameObjectSprites[i].spriteKey;
            for (std::size_t s = 0; s < sprites.size(); ++s) {
                if (sprites[s].key != spriteKey) continue;
                gameObjectSprites[i].spriteIndex = static_cast<u32>(s);
                break;
            }
        }
    }

    if (!writeArchive(output, entries, frames, sprites, spriteRoots, scenes, spawns,
                      cutscenes, cutsceneTimestamps, cutsceneCommands,
                      gameObjects, gameObjectSprites)) {
        std::fprintf(stderr, "failed to write %s\n", output.c_str());
        return 1;
    }
    std::printf("wrote %s: %zu entries, %zu sprites, %zu frames, %zu scenes, %zu spawns, "
                "%zu cutscenes, %zu timestamps, %zu commands\n",
                output.c_str(), entries.size(), sprites.size(), frames.size(),
                scenes.size(), spawns.size(), cutscenes.size(),
                cutsceneTimestamps.size(), cutsceneCommands.size());
    return 0;
}
