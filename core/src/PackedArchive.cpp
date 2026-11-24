#include "mmu/core/PackedArchive.h"

#include <cstring>
#include <utility>

namespace mmu {
namespace {

constexpr std::size_t kHeaderSizeV2 = 96;
constexpr std::size_t kHeaderSizeV3 = 128;
constexpr std::size_t kHeaderSizeV5 = 192;
constexpr std::size_t kEntrySize = 136;
constexpr std::size_t kSpriteSize = 40;
constexpr std::size_t kFrameSize = 24;
constexpr std::size_t kSceneSizeV3 = 16;
constexpr std::size_t kSceneSizeV4 = 24;
constexpr std::size_t kSpawnSize = 32;
constexpr std::size_t kCutsceneSize = 16;
constexpr std::size_t kCutsceneTimestampSize = 16;
constexpr std::size_t kCutsceneCommandSize = 72;
constexpr std::size_t kGameObjectSize = 16;
constexpr std::size_t kGameObjectSpriteSize = 12;
constexpr u32 kNoIndex = 0xFFFFFFFFu;

u32 readU32(const u8* p) {
    return static_cast<u32>(p[0]) | static_cast<u32>(p[1]) << 8
         | static_cast<u32>(p[2]) << 16 | static_cast<u32>(p[3]) << 24;
}

i32 readI32(const u8* p) { return static_cast<i32>(readU32(p)); }

float readFloat(const u8* p) {
    const u32 bits = readU32(p);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

u64 readU64(const u8* p) {
    u64 value = 0;
    for (unsigned i = 0; i < 8; ++i)
        value |= static_cast<u64>(p[i]) << (i * 8);
    return value;
}

bool inRange(u64 offset, u64 size, std::size_t total) {
    return offset <= total && size <= total - offset;
}

} // namespace

const char* packedSourceName(PackedSource source) {
    switch (source) {
    case PackedSource::Misc: return "misc.mmu";
    case PackedSource::Levels: return "levels.mmu";
    case PackedSource::CutsceneDefs: return "cutscenedefs.mmu";
    case PackedSource::Guns: return "guns.mmu";
    case PackedSource::Menus: return "menus.mmu";
    case PackedSource::Cutscenes: return "cutscenes.mmu";
    case PackedSource::GameObjects: return "gameobjects.mmu";
    case PackedSource::LevelArt: return "levelart.mmu";
    default: return "unknown";
    }
}

const char* packedEntryTypeName(PackedEntryType type) {
    switch (type) {
    case PackedEntryType::Image: return "image";
    case PackedEntryType::Audio: return "audio";
    case PackedEntryType::Music: return "music";
    default: return "unknown";
    }
}

std::string_view PackedArchive::stringAt(u32 offset) const {
    if (offset >= strings_.size) return {};
    const u8* start = strings_.data + offset;
    const u8* end = start;
    while (end < strings_.data + strings_.size && *end != 0) ++end;
    return std::string_view(reinterpret_cast<const char*>(start),
                            static_cast<std::size_t>(end - start));
}

bool PackedArchive::load(ByteView bytes) {
    clear();
    if (bytes.size < kHeaderSizeV2 || readU64(bytes.data) != kMagic) return false;
    const u32 version = readU32(bytes.data + 8);
    if (version < 2 || version > kVersion) return false;
    if (version >= 3 && bytes.size < kHeaderSizeV3) return false;
    if (version >= 5 && bytes.size < kHeaderSizeV5) return false;
    version_ = version;

    const u32 entryCount = readU32(bytes.data + 12);
    const u64 entriesOffset = readU64(bytes.data + 16);
    const u64 stringsOffset = readU64(bytes.data + 24);
    const u64 stringsSize = readU64(bytes.data + 32);
    const u64 dataOffset = readU64(bytes.data + 40);
    const u64 dataSize = readU64(bytes.data + 48);
    const u64 spriteTableOffset = readU64(bytes.data + 56);
    const u64 frameTableOffset = readU64(bytes.data + 64);
    const u32 spriteCount = readU32(bytes.data + 72);
    const u32 frameCount = readU32(bytes.data + 76);

    if (entryCount > 1'000'000 || spriteCount > 1'000'000 || frameCount > 2'000'000)
        return false;
    if (!inRange(entriesOffset, static_cast<u64>(entryCount) * kEntrySize, bytes.size)) return false;
    if (!inRange(stringsOffset, stringsSize, bytes.size)) return false;
    if (!inRange(dataOffset, dataSize, bytes.size)) return false;
    if (!inRange(spriteTableOffset, static_cast<u64>(spriteCount) * kSpriteSize, bytes.size)) return false;
    if (!inRange(frameTableOffset, static_cast<u64>(frameCount) * kFrameSize, bytes.size)) return false;

    strings_ = bytes.subview(static_cast<std::size_t>(stringsOffset),
                             static_cast<std::size_t>(stringsSize));
    entries_.reserve(entryCount);
    for (u32 i = 0; i < entryCount; ++i) {
        const u8* e = bytes.data + entriesOffset + static_cast<std::size_t>(i) * kEntrySize;
        const u32 typeValue = readU32(e);
        if (typeValue < static_cast<u32>(PackedEntryType::Image) ||
            typeValue > static_cast<u32>(PackedEntryType::Music)) return false;

        PackedEntry entry;
        entry.type = static_cast<PackedEntryType>(typeValue);
        entry.key = readU32(e + 4);
        entry.source = static_cast<PackedSource>(readU32(e + 8));
        entry.width = readU32(e + 12);
        entry.height = readU32(e + 16);
        entry.pitch = readU32(e + 20);
        entry.bitsPerPixel = readU32(e + 24);
        entry.bytesPerPixel = readU32(e + 28);
        entry.paletteCount = readU32(e + 32);
        entry.pixelChunks = readU32(e + 36);
        entry.rLoss = readU32(e + 40);
        entry.gLoss = readU32(e + 44);
        entry.bLoss = readU32(e + 48);
        entry.aLoss = readU32(e + 52);
        entry.rShift = readU32(e + 56);
        entry.gShift = readU32(e + 60);
        entry.bShift = readU32(e + 64);
        entry.aShift = readU32(e + 68);
        entry.rMask = readU32(e + 72);
        entry.gMask = readU32(e + 76);
        entry.bMask = readU32(e + 80);
        entry.aMask = readU32(e + 84);
        entry.audioLength = readU32(e + 128);
        entry.volume = readU32(e + 132);

        const u64 blobOffset = readU64(e + 88);
        const u64 blobSize = readU64(e + 96);
        const u64 paletteOffset = readU64(e + 104);
        const u64 paletteSize = readU64(e + 112);
        const u64 nameOffset = readU64(e + 120);
        if (!inRange(dataOffset + blobOffset, blobSize, bytes.size)) return false;
        entry.data = bytes.subview(static_cast<std::size_t>(dataOffset + blobOffset),
                                   static_cast<std::size_t>(blobSize));
        if (paletteSize) {
            if (!inRange(dataOffset + paletteOffset, paletteSize, bytes.size)) return false;
            entry.palette = bytes.subview(static_cast<std::size_t>(dataOffset + paletteOffset),
                                          static_cast<std::size_t>(paletteSize));
        }
        entry.name = stringAt(static_cast<u32>(nameOffset));
        entries_.push_back(entry);
    }

    frames_.reserve(frameCount);
    for (u32 i = 0; i < frameCount; ++i) {
        const u8* f = bytes.data + frameTableOffset + static_cast<std::size_t>(i) * kFrameSize;
        PackedFrame frame;
        frame.imageIndex = readU32(f);
        frame.collisionIndex = readU32(f + 4);
        frame.duration = readFloat(f + 8);
        frame.xOffset = readI32(f + 12);
        frame.yOffset = readI32(f + 16);
        if (frame.imageIndex >= entryCount && frame.imageIndex != kNoIndex) return false;
        frames_.push_back(frame);
    }

    sprites_.reserve(spriteCount);
    for (u32 i = 0; i < spriteCount; ++i) {
        const u8* s = bytes.data + spriteTableOffset + static_cast<std::size_t>(i) * kSpriteSize;
        PackedSprite sprite;
        sprite.key = readU32(s);
        const u32 firstFrame = readU32(s + 4);
        sprite.frameCount = readU32(s + 8);
        sprite.xHandle = readI32(s + 12);
        sprite.yHandle = readI32(s + 16);
        const u32 nameOffset = readU32(s + 20);
        sprite.source = static_cast<PackedSource>(readU32(s + 24));
        sprite.resRoot = stringAt(nameOffset);
        if (sprite.frameCount) {
            if (!inRange(firstFrame, sprite.frameCount, frames_.size())) return false;
            sprite.frames = frames_.data() + firstFrame;
        }
        sprites_.push_back(sprite);
    }

    if (version >= 3) {
        const u64 sceneTableOffset = readU64(bytes.data + 88);
        const u64 spawnTableOffset = readU64(bytes.data + 96);
        const u32 sceneCount = readU32(bytes.data + 104);
        const u32 spawnCount = readU32(bytes.data + 108);
        const std::size_t sceneSize = version >= 4 ? kSceneSizeV4 : kSceneSizeV3;
        if (sceneCount > 1'000'000 || spawnCount > 2'000'000) return false;
        if (!inRange(sceneTableOffset, static_cast<u64>(sceneCount) * sceneSize, bytes.size)) return false;
        if (!inRange(spawnTableOffset, static_cast<u64>(spawnCount) * kSpawnSize, bytes.size)) return false;

        spawns_.reserve(spawnCount);
        for (u32 i = 0; i < spawnCount; ++i) {
            const u8* s = bytes.data + spawnTableOffset + static_cast<std::size_t>(i) * kSpawnSize;
            PackedSpawn spawn;
            spawn.x = readFloat(s);
            spawn.y = readFloat(s + 4);
            spawn.autoRespawn = readFloat(s + 8);
            const u32 flags = readU32(s + 12);
            spawn.facingRight = (flags & (1u << 0)) != 0;
            spawn.respawnOnDeath = (flags & (1u << 1)) != 0;
            spawn.respawnOnScreen = (flags & (1u << 2)) != 0;
            spawn.deleteOffScreen = (flags & (1u << 3)) != 0;
            spawn.collectible = (flags & (1u << 4)) != 0;
            spawn.type = stringAt(readU32(s + 16));
            spawn.attr1 = stringAt(readU32(s + 20));
            spawn.attr2 = stringAt(readU32(s + 24));
            spawns_.push_back(spawn);
        }

        scenes_.reserve(sceneCount);
        for (u32 i = 0; i < sceneCount; ++i) {
            const u8* s = bytes.data + sceneTableOffset + static_cast<std::size_t>(i) * sceneSize;
            PackedScene scene;
            scene.key = readU32(s);
            const u32 firstSpawn = readU32(s + 4);
            scene.spawnCount = readU32(s + 8);
            if (version >= 4) scene.name = stringAt(readU32(s + 12));
            if (scene.spawnCount) {
                if (!inRange(firstSpawn, scene.spawnCount, spawns_.size())) return false;
                scene.spawns = spawns_.data() + firstSpawn;
            }
            scenes_.push_back(scene);
        }
    }

    if (version >= 5) {
        // v5 extends the old reserved slot at +120 with two table offsets and
        // three counts. Bytes 0..127 are unchanged from v4.
        const u64 cutsceneTableOffset = readU64(bytes.data + 120);
        const u64 timestampTableOffset = readU64(bytes.data + 128);
        const u64 commandTableOffset = readU64(bytes.data + 136);
        const u32 cutsceneCount = readU32(bytes.data + 144);
        const u32 timestampCount = readU32(bytes.data + 148);
        const u32 commandCount = readU32(bytes.data + 152);
        // These two fields occupy the v5 reserved6/reserved7 slots left by
        // the cutscene-table extension. Older v5 archives have count 0.
        const u32 gameObjectCount = readU32(bytes.data + 156);
        const u64 gameObjectOffset = readU64(bytes.data + 160);
        const u64 gameObjectSpriteOffset = readU64(bytes.data + 168);
        if (cutsceneCount > 100'000 || timestampCount > 1'000'000 ||
            commandCount > 2'000'000 || gameObjectCount > 100'000)
            return false;
        if (!inRange(cutsceneTableOffset,
                     static_cast<u64>(cutsceneCount) * kCutsceneSize, bytes.size))
            return false;
        if (!inRange(timestampTableOffset,
                     static_cast<u64>(timestampCount) * kCutsceneTimestampSize,
                     bytes.size))
            return false;
        if (!inRange(commandTableOffset,
                     static_cast<u64>(commandCount) * kCutsceneCommandSize,
                     bytes.size))
            return false;
        if (!inRange(gameObjectOffset,
                     static_cast<u64>(gameObjectCount) * kGameObjectSize,
                     bytes.size))
            return false;

        gameObjects_.reserve(gameObjectCount);
        u64 gameObjectSpriteEnd = 0;
        for (u32 i = 0; i < gameObjectCount; ++i) {
            const u8* o = bytes.data + gameObjectOffset +
                          static_cast<std::size_t>(i) * kGameObjectSize;
            PackedGameObject object;
            object.key = readU32(o);
            object.nameOffset = readU32(o + 4);
            object.firstSprite = readU32(o + 8);
            object.spriteCount = readU32(o + 12);
            object.name = stringAt(object.nameOffset);
            const u64 end = static_cast<u64>(object.firstSprite) +
                            object.spriteCount;
            if (end > 10'000'000 || end < gameObjectSpriteEnd) return false;
            gameObjectSpriteEnd = end;
            gameObjects_.push_back(std::move(object));
        }
        if (!inRange(gameObjectSpriteOffset,
                     gameObjectSpriteEnd * kGameObjectSpriteSize, bytes.size))
            return false;
        gameObjectSprites_.reserve(static_cast<std::size_t>(gameObjectSpriteEnd));
        for (u32 i = 0; i < gameObjectSpriteEnd; ++i) {
            const u8* l = bytes.data + gameObjectSpriteOffset +
                          static_cast<std::size_t>(i) * kGameObjectSpriteSize;
            PackedGameObjectSprite link;
            link.animKey = readU32(l);
            link.spriteKey = readU32(l + 4);
            link.spriteIndex = readU32(l + 8);
            gameObjectSprites_.push_back(link);
        }

        cutsceneCommands_.reserve(commandCount);
        for (u32 i = 0; i < commandCount; ++i) {
            const u8* c = bytes.data + commandTableOffset +
                          static_cast<std::size_t>(i) * kCutsceneCommandSize;
            PackedCutsceneCommand command;
            command.name = stringAt(readU32(c + 0));
            command.id = stringAt(readU32(c + 4));
            command.resource = stringAt(readU32(c + 8));
            command.value = stringAt(readU32(c + 12));
            command.anim = stringAt(readU32(c + 16));
            command.object = stringAt(readU32(c + 20));
            command.x = readI32(c + 24);
            command.y = readI32(c + 28);
            command.z = readI32(c + 32);
            command.fadeDelay = readI32(c + 36);
            command.moveDuration = readI32(c + 40);
            command.moveSpeed = readI32(c + 44);
            command.delayBetweenLetters = readI32(c + 48);
            const u32 flags = readU32(c + 52);
            command.playThenKill = (flags & 1u) != 0;
            command.faceRight = (flags & 2u) != 0;
            cutsceneCommands_.push_back(std::move(command));
        }

        cutsceneTimestamps_.reserve(timestampCount);
        for (u32 i = 0; i < timestampCount; ++i) {
            const u8* s = bytes.data + timestampTableOffset +
                          static_cast<std::size_t>(i) * kCutsceneTimestampSize;
            PackedCutsceneTimestamp stamp;
            stamp.time = readU32(s);
            const u32 firstCommand = readU32(s + 4);
            stamp.commandCount = readU32(s + 8);
            if (!inRange(firstCommand, stamp.commandCount, cutsceneCommands_.size()))
                return false;
            if (stamp.commandCount)
                stamp.commands = cutsceneCommands_.data() + firstCommand;
            cutsceneTimestamps_.push_back(stamp);
        }

        cutscenes_.reserve(cutsceneCount);
        for (u32 i = 0; i < cutsceneCount; ++i) {
            const u8* s = bytes.data + cutsceneTableOffset +
                          static_cast<std::size_t>(i) * kCutsceneSize;
            PackedCutscene cutscene;
            cutscene.key = readU32(s);
            const u32 firstTimestamp = readU32(s + 4);
            cutscene.timestampCount = readU32(s + 8);
            cutscene.path = stringAt(readU32(s + 12));
            if (!inRange(firstTimestamp, cutscene.timestampCount,
                         cutsceneTimestamps_.size()))
                return false;
            if (cutscene.timestampCount)
                cutscene.timestamps = cutsceneTimestamps_.data() + firstTimestamp;
            cutscenes_.push_back(cutscene);
        }
    }

    valid_ = true;
    return true;
}

void PackedArchive::clear() {
    entries_.clear();
    sprites_.clear();
    frames_.clear();
    scenes_.clear();
    spawns_.clear();
    cutscenes_.clear();
    cutsceneTimestamps_.clear();
    cutsceneCommands_.clear();
    gameObjects_.clear();
    gameObjectSprites_.clear();
    strings_ = {};
    version_ = 0;
    valid_ = false;
}

const PackedEntry* PackedArchive::find(u32 key, PackedEntryType type) const {
    for (const PackedEntry& entry : entries_)
        if (entry.key == key && entry.type == type) return &entry;
    return nullptr;
}

const PackedSprite* PackedArchive::findSprite(u32 key) const {
    for (const PackedSprite& sprite : sprites_)
        if (sprite.key == key) return &sprite;
    return nullptr;
}

const PackedScene* PackedArchive::findScene(u32 key) const {
    for (const PackedScene& scene : scenes_)
        if (scene.key == key) return &scene;
    return nullptr;
}

const PackedGameObject* PackedArchive::findGameObject(u32 key) const {
    for (const PackedGameObject& object : gameObjects_)
        if (object.key == key) return &object;
    return nullptr;
}

std::size_t PackedArchive::imageCount() const {
    std::size_t count = 0;
    for (const PackedEntry& entry : entries_)
        if (entry.type == PackedEntryType::Image) ++count;
    return count;
}

} // namespace mmu
