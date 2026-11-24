#include "mmu/core/AssetDatabase.h"
#include "mmu/core/PackedArchive.h"
#include "mmu/core/ProtoReader.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <utility>

namespace mmu {
namespace {

using Field = ProtoReader::Field;

void assign(std::string& target, std::string_view source) {
    target.assign(source.data(), source.size());
}

bool parseColor(ProtoReader r, ColorAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        u32 value = 0;
        if ((f.number == 1 || f.number == 2 || f.number == 3) &&
            f.wire == ProtoReader::Varint && r.readU32(value)) {
            if (f.number == 1) out.r = static_cast<u8>(value);
            if (f.number == 2) out.g = static_cast<u8>(value);
            if (f.number == 3) out.b = static_cast<u8>(value);
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parsePalette(ProtoReader r, std::vector<ColorAsset>& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 2 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub)) return false;
            out.emplace_back();
            if (!parseColor(sub, out.back())) return false;
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parsePixelFormat(ProtoReader r, SurfaceAsset& out) {
    out.hasPixelFormat = true;
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 1 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub) || !parsePalette(sub, out.palette)) return false;
            continue;
        }
        if (f.wire == ProtoReader::Varint) {
            u32 value = 0;
            if (!r.readU32(value)) return false;
            switch (f.number) {
            case 2: out.bitsPerPixel = value; break;
            case 3: out.bytesPerPixel = value; break;
            case 4: out.rLoss = value; break;
            case 5: out.gLoss = value; break;
            case 6: out.bLoss = value; break;
            case 7: out.aLoss = value; break;
            case 8: out.rShift = value; break;
            case 9: out.gShift = value; break;
            case 10: out.bShift = value; break;
            case 11: out.aShift = value; break;
            case 12: out.rMask = value; break;
            case 13: out.gMask = value; break;
            case 14: out.bMask = value; break;
            case 15: out.aMask = value; break;
            default: break; // newer field: value already consumed
            }
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseSurface(ProtoReader r, SurfaceAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 2 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub) || !parsePixelFormat(sub, out)) return false;
            continue;
        }
        if (f.number == 6 && f.wire == ProtoReader::LengthDelimited) {
            ByteView pixels;
            if (!r.readBytes(pixels)) return false;
            out.pixels.insert(out.pixels.end(), pixels.data, pixels.data + pixels.size);
            out.pixelChunks++;
            continue;
        }
        if (f.wire == ProtoReader::Varint) {
            u32 value = 0;
            if (!r.readU32(value)) return false;
            switch (f.number) {
            case 1: out.flags = value; break;
            case 3: out.width = value; break;
            case 4: out.height = value; break;
            case 5: out.pitch = value; break;
            default: break;
            }
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseAnimationFrame(ProtoReader r, AnimationFrameAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.wire == ProtoReader::LengthDelimited && (f.number == 1 || f.number == 6)) {
            ProtoReader sub;
            if (!r.readMessage(sub)) return false;
            SurfaceAsset& surface = f.number == 1 ? out.image : out.collision;
            if (!parseSurface(sub, surface)) return false;
            continue;
        }
        std::string_view text;
        if (f.wire == ProtoReader::LengthDelimited && r.readString(text)) {
            if (f.number == 2) assign(out.imagePath, text);
            else if (f.number == 7) assign(out.collisionPath, text);
            continue;
        }
        float duration = 0.0f;
        i32 offset = 0;
        if (f.number == 3 && f.wire == ProtoReader::Fixed32 && r.readFloat(duration)) {
            out.duration = duration;
        } else if ((f.number == 4 || f.number == 5) && f.wire == ProtoReader::Varint && r.readI32(offset)) {
            if (f.number == 4) out.xOffset = offset;
            else out.yOffset = offset;
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseAnimation(ProtoReader r, AnimationAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 2 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub)) return false;
            out.frames.emplace_back();
            if (!parseAnimationFrame(sub, out.frames.back())) return false;
            continue;
        }
        i32 value = 0;
        if (f.wire == ProtoReader::Varint && r.readI32(value)) {
            if (f.number == 1) out.mode = value;
            else if (f.number == 3) out.framesCount = value;
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseMisc(ProtoReader r, SpriteAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 1 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub) || !parseSurface(sub, out.singleCollision)) return false;
            continue;
        }
        std::string_view text;
        if (f.number == 2 && f.wire == ProtoReader::LengthDelimited && r.readString(text)) {
            assign(out.singleCollisionPath, text);
            continue;
        }
        i32 value = 0;
        if ((f.number == 3 || f.number == 4) && f.wire == ProtoReader::Varint && r.readI32(value)) {
            if (f.number == 3) out.xHandle = value;
            else out.yHandle = value;
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseSprite(ProtoReader r, SpriteAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 3 && f.wire == ProtoReader::LengthDelimited) {
            // res_root is a string, not a nested message.
            std::string_view text;
            if (!r.readString(text)) return false;
            assign(out.resRoot, text);
            continue;
        }
        if (f.wire != ProtoReader::LengthDelimited) {
            u32 key = 0;
            if (f.number == 4 && r.readU32(key)) out.key = key;
            else if (!r.skipField()) return false;
            continue;
        }
        ProtoReader sub;
        if (!r.readMessage(sub)) return false;
        if (f.number == 1) {
            if (!parseAnimation(sub, out.animation)) return false;
        } else if (f.number == 2) {
            if (!parseMisc(sub, out)) return false;
        } else if (!sub.skipField()) return false;
    }
    return true;
}

bool parseGameParam(ProtoReader r, GameParamAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        u32 key = 0;
        float param = 0.0f;
        if (f.number == 1 && f.wire == ProtoReader::Varint && r.readU32(key)) out.key = key;
        else if (f.number == 2 && f.wire == ProtoReader::Fixed32 && r.readFloat(param)) out.param = param;
        else if (!r.skipField()) return false;
    }
    return true;
}

bool parseSound(ProtoReader r, SoundAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.wire == ProtoReader::LengthDelimited && (f.number == 2 || f.number == 4 || f.number == 7)) {
            std::string_view text;
            ByteView bytes;
            if (f.number == 2 && r.readString(text)) assign(out.path, text);
            else if (f.number == 4 && r.readBytes(bytes)) out.audioBuffer.assign(bytes.data, bytes.data + bytes.size);
            else if (f.number == 7 && r.readBytes(bytes)) out.musicStream.assign(bytes.data, bytes.data + bytes.size);
            else return false;
            continue;
        }
        if (f.number == 3 && f.wire == ProtoReader::Varint) {
            i32 allocated = 0;
            if (!r.readI32(allocated)) return false;
            out.allocated = allocated;
            continue;
        }
        if (f.wire == ProtoReader::Varint) {
            u32 value = 0;
            if (!r.readU32(value)) return false;
            switch (f.number) {
            case 1: out.key = value; break;
            case 5: out.audioLength = value; break;
            case 6: out.volume = value; break;
            case 8: out.musicStreamLength = value; break;
            default: break;
            }
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseGameObject(ProtoReader r, GameObjectAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.wire == ProtoReader::LengthDelimited && (f.number == 2 || f.number == 3)) {
            ProtoReader sub;
            if (!r.readMessage(sub)) return false;
            if (f.number == 2) {
                out.gameParams.emplace_back();
                if (!parseGameParam(sub, out.gameParams.back())) return false;
            } else {
                out.sounds.emplace_back();
                if (!parseSound(sub, out.sounds.back())) return false;
            }
            continue;
        }
        if (f.wire == ProtoReader::Varint) {
            u32 value = 0;
            if (!r.readU32(value)) return false;
            if (f.number == 1) out.key = value;
            else if (f.number == 4) out.sprites.push_back(value);
            else if (f.number == 5) out.spriteKeys.push_back(value);
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseSpawn(ProtoReader r, SpawnAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        std::string_view text;
        if (f.wire == ProtoReader::LengthDelimited && f.number >= 9 && f.number <= 11 && r.readString(text)) {
            if (f.number == 9) assign(out.type, text);
            else if (f.number == 10) assign(out.attr1, text);
            else assign(out.attr2, text);
            continue;
        }
        float floating = 0.0f;
        bool flag = false;
        if (f.wire == ProtoReader::Fixed32) {
            if (!r.readFloat(floating)) return false;
            if (f.number == 1) out.x = floating;
            else if (f.number == 2) out.y = floating;
            else if (f.number == 8) out.autoRespawn = floating;
        } else if (f.wire == ProtoReader::Varint) {
            if (!r.readBool(flag)) return false;
            switch (f.number) {
            case 3: out.facingRight = flag; break;
            case 4: out.respawnOnDeath = flag; break;
            case 5: out.respawnOnScreen = flag; break;
            case 6: out.deleteOffScreen = flag; break;
            case 7: out.collectible = flag; break;
            default: break;
            }
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseScene(ProtoReader r, SceneAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 1 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub)) return false;
            out.gameObjects.emplace_back();
            if (!parseSpawn(sub, out.gameObjects.back())) return false;
            continue;
        }
        u32 key = 0;
        if (f.number == 2 && f.wire == ProtoReader::Varint && r.readU32(key)) out.key = key;
        else if (!r.skipField()) return false;
    }
    return true;
}

bool parseCommand(ProtoReader r, CommandAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.wire == ProtoReader::LengthDelimited) {
            std::string_view text;
            if (!r.readString(text)) return false;
            switch (f.number) {
            case 1: assign(out.name, text); break;
            case 2: assign(out.id, text); break;
            case 3: assign(out.resource, text); break;
            case 8: assign(out.value, text); break;
            case 10: assign(out.anim, text); break;
            case 12: assign(out.object, text); break;
            default: break;
            }
            continue;
        }
        if ((f.number == 9 || f.number == 14) && f.wire == ProtoReader::Varint) {
            bool flag = false;
            if (!r.readBool(flag)) return false;
            if (f.number == 9) out.playThenKill = flag; else out.faceRight = flag;
            continue;
        }
        i32 value = 0;
        if (f.wire == ProtoReader::Varint && r.readI32(value)) {
            switch (f.number) {
            case 4: out.x = value; break;
            case 5: out.y = value; break;
            case 6: out.z = value; break;
            case 7: out.fadeDelay = value; break;
            case 11: out.moveDuration = value; break;
            case 13: out.moveSpeed = value; break;
            case 15: out.delayBetweenLetters = value; break;
            default: break;
            }
        } else if (!r.skipField()) return false;
    }
    return true;
}

bool parseTimeStamp(ProtoReader r, TimeStampAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 1 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub)) return false;
            out.commands.emplace_back();
            if (!parseCommand(sub, out.commands.back())) return false;
            continue;
        }
        u32 time = 0;
        if (f.number == 2 && f.wire == ProtoReader::Varint && r.readU32(time)) out.time = time;
        else if (!r.skipField()) return false;
    }
    return true;
}

bool parseCutScene(ProtoReader r, CutSceneAsset& out) {
    while (!r.done()) {
        Field f;
        if (!r.next(f)) return false;
        if (f.number == 1 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!r.readMessage(sub)) return false;
            out.timestamps.emplace_back();
            if (!parseTimeStamp(sub, out.timestamps.back())) return false;
        } else if (!r.skipField()) return false;
    }
    return true;
}

} // namespace

AssetKind kindForFilename(const std::string& filename) {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c);
    });
    if (lower == "guns.mmu") return AssetKind::Guns;
    if (lower == "levels.mmu") return AssetKind::Level;
    if (lower == "cutscenedefs.mmu") return AssetKind::Cutscenes;
    if (lower == "misc.mmu") return AssetKind::PNG;
    if (lower == "cutscenes.mmu" || lower == "gameobjects.mmu" ||
        lower == "levelart.mmu" || lower == "menus.mmu") return AssetKind::AllSprites;
    return AssetKind::Unknown;
}

const char* kindName(AssetKind kind) {
    switch (kind) {
    case AssetKind::AllSprites: return "AllSprites";
    case AssetKind::Guns: return "Guns";
    case AssetKind::Level: return "Level";
    case AssetKind::Cutscenes: return "Database";
    case AssetKind::PNG: return "PNG";
    default: return "Unknown";
    }
}

bool AssetDatabase::decodeAllSprites(ProtoReader reader, const std::string& source) {
    while (!reader.done()) {
        Field f;
        if (!reader.next(f)) return false;
        if (f.number != 1 || f.wire != ProtoReader::LengthDelimited) {
            if (!reader.skipField()) return false;
            continue;
        }
        ProtoReader sub;
        if (!reader.readMessage(sub)) return false;
        sprites_.emplace_back();
        SpriteAsset& sprite = sprites_.back();
        sprite.source = source;
        if (!parseSprite(sub, sprite)) return false;
        stats_.sprites++;
        stats_.animationFrames += sprite.animation.frames.size();
        for (const AnimationFrameAsset& frame : sprite.animation.frames)
            stats_.pixelBytes += frame.image.pixels.size();
    }
    return true;
}

bool AssetDatabase::decodeGuns(ProtoReader reader) {
    while (!reader.done()) {
        Field f;
        if (!reader.next(f)) return false;
        if (f.number != 1 || f.wire != ProtoReader::LengthDelimited) {
            if (!reader.skipField()) return false;
            continue;
        }
        ProtoReader sub;
        if (!reader.readMessage(sub)) return false;
        gameObjects_.emplace_back();
        if (!parseGameObject(sub, gameObjects_.back())) return false;
        stats_.gameObjects++;
        stats_.gameParams += gameObjects_.back().gameParams.size();
        stats_.sounds += gameObjects_.back().sounds.size();
    }
    return true;
}

bool AssetDatabase::decodeLevel(ProtoReader reader) {
    while (!reader.done()) {
        Field f;
        if (!reader.next(f)) return false;
        if (f.number != 1 || f.wire != ProtoReader::LengthDelimited) {
            if (!reader.skipField()) return false;
            continue;
        }
        ProtoReader sub;
        if (!reader.readMessage(sub)) return false;
        scenes_.emplace_back();
        if (!parseScene(sub, scenes_.back())) return false;
        stats_.scenes++;
        stats_.spawns += scenes_.back().gameObjects.size();
    }
    return true;
}

bool AssetDatabase::decodeCutscenes(ProtoReader reader) {
    while (!reader.done()) {
        Field f;
        if (!reader.next(f)) return false;
        if (f.number == 2 && f.wire == ProtoReader::Varint) {
            u32 key = 0;
            if (!reader.readU32(key)) return false;
            cutsceneKeys_.push_back(key);
            continue;
        }
        if (f.number != 1 || f.wire != ProtoReader::LengthDelimited) {
            if (!reader.skipField()) return false;
            continue;
        }
        ProtoReader sub;
        if (!reader.readMessage(sub)) return false;
        cutscenes_.emplace_back();
        if (!parseCutScene(sub, cutscenes_.back())) return false;
        stats_.cutscenes++;
        for (const TimeStampAsset& stamp : cutscenes_.back().timestamps) {
            stats_.timestamps++;
            stats_.commands += stamp.commands.size();
        }
    }
    return true;
}

bool AssetDatabase::decodePng(ProtoReader reader) {
    while (!reader.done()) {
        Field f;
        if (!reader.next(f)) return false;
        if (f.number == 1 && f.wire == ProtoReader::LengthDelimited) {
            ProtoReader sub;
            if (!reader.readMessage(sub)) return false;
            surfaces_.emplace_back();
            if (!parseSurface(sub, surfaces_.back())) return false;
            stats_.surfaces++;
            stats_.pixelBytes += surfaces_.back().pixels.size();
        } else if (!reader.skipField()) return false;
    }
    return true;
}

bool AssetDatabase::load(ByteView bytes, AssetKind kind, const std::string& source) {
    switch (kind) {
    case AssetKind::AllSprites: return decodeAllSprites(ProtoReader(bytes), source);
    case AssetKind::Guns: return decodeGuns(ProtoReader(bytes));
    case AssetKind::Level: return decodeLevel(ProtoReader(bytes));
    case AssetKind::Cutscenes: return decodeCutscenes(ProtoReader(bytes));
    case AssetKind::PNG: return decodePng(ProtoReader(bytes));
    default: return false;
    }
}

bool AssetDatabase::loadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    file.seekg(0, std::ios::end);
    const std::streamoff length = file.tellg();
    if (length <= 0) return false;
    file.seekg(0, std::ios::beg);
    std::vector<u8> bytes(static_cast<std::size_t>(length));
    file.read(reinterpret_cast<char*>(bytes.data()), length);
    if (!file) return false;
    const std::size_t slash = path.find_last_of("/\\");
    const std::string filename = slash == std::string::npos ? path : path.substr(slash + 1);
    return load(ByteView(bytes.data(), bytes.size()), kindForFilename(filename), filename);
}

bool AssetDatabase::loadPackedCutscenes(const PackedArchive& pak) {
    if (!pak.valid()) return false;
    cutscenes_.clear();
    cutsceneKeys_.clear();
    cutscenePaths_.clear();

    cutscenes_.reserve(pak.cutscenes().size());
    cutsceneKeys_.reserve(pak.cutscenes().size());
    cutscenePaths_.reserve(pak.cutscenes().size());
    for (const PackedCutscene& packed : pak.cutscenes()) {
        CutSceneAsset scene;
        scene.timestamps.reserve(packed.timestampCount);
        for (u32 stampIndex = 0; stampIndex < packed.timestampCount; ++stampIndex) {
            const PackedCutsceneTimestamp& stamp = packed.timestamps[stampIndex];
            TimeStampAsset decodedStamp;
            decodedStamp.time = stamp.time;
            decodedStamp.commands.reserve(stamp.commandCount);
            for (u32 commandIndex = 0; commandIndex < stamp.commandCount; ++commandIndex) {
                const PackedCutsceneCommand& packedCommand =
                    stamp.commands[commandIndex];
                CommandAsset command;
                assign(command.name, packedCommand.name);
                assign(command.id, packedCommand.id);
                assign(command.resource, packedCommand.resource);
                assign(command.value, packedCommand.value);
                assign(command.anim, packedCommand.anim);
                assign(command.object, packedCommand.object);
                command.x = packedCommand.x;
                command.y = packedCommand.y;
                command.z = packedCommand.z;
                command.fadeDelay = packedCommand.fadeDelay;
                command.moveDuration = packedCommand.moveDuration;
                command.moveSpeed = packedCommand.moveSpeed;
                command.delayBetweenLetters = packedCommand.delayBetweenLetters;
                command.playThenKill = packedCommand.playThenKill;
                command.faceRight = packedCommand.faceRight;
                decodedStamp.commands.push_back(std::move(command));
            }
            scene.timestamps.push_back(std::move(decodedStamp));
        }
        cutscenes_.push_back(std::move(scene));
        cutsceneKeys_.push_back(packed.key);
        cutscenePaths_.emplace_back(packed.path);
    }

    stats_.cutscenes = cutscenes_.size();
    stats_.timestamps = 0;
    stats_.commands = 0;
    for (const CutSceneAsset& scene : cutscenes_) {
        stats_.timestamps += scene.timestamps.size();
        for (const TimeStampAsset& stamp : scene.timestamps)
            stats_.commands += stamp.commands.size();
    }
    return true;
}

void AssetDatabase::clear() {
    sprites_.clear();
    gameObjects_.clear();
    scenes_.clear();
    cutscenes_.clear();
    cutsceneKeys_.clear();
    cutscenePaths_.clear();
    surfaces_.clear();
    stats_ = AssetStats{};
}

} // namespace mmu
