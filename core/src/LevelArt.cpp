#include "mmu/core/LevelArt.h"

#include "mmu/core/World.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace mmu {
namespace {

std::string basename(std::string_view path) {
    std::size_t start = 0;
    for (std::size_t i = 0; i < path.size(); ++i)
        if (path[i] == '\\' || path[i] == '/') start = i + 1;
    std::string out = toLowerAscii(path.substr(start));
    const std::size_t dot = out.rfind('.');
    if (dot != std::string::npos) out.resize(dot);
    return out;
}

std::string levelDir(std::string_view path) {
    const std::string lower = toLowerAscii(path);
    const std::size_t from = lower.find("\\levels\\");
    if (from == std::string::npos) return {};
    const std::size_t start = from + 8;
    const std::size_t end = lower.find_first_of("\\/", start);
    if (end == std::string::npos) return {};
    return lower.substr(start, end - start);
}

bool dirCompatible(const std::string& artDir, const std::string& hint) {
    if (artDir.empty() || hint.empty()) return false;
    return artDir == hint || hint.find(artDir) != std::string::npos ||
           artDir.find(hint) != std::string::npos;
}

// "Section_02" and "SectionAlt01" both mean "section 2"/"alt 1" to the
// collision baker. Strip leading zeroes from every digit run, not just the
// whole token, so "alt01" and "alt1" compare equal.
std::string stripZeros(std::string text) {
    const std::size_t digit = text.find_first_of("0123456789");
    if (digit == std::string::npos) return text;
    std::size_t first = digit;
    while (first < text.size() && text[first] == '0') ++first;
    if (first == digit) return text;
    if (first == text.size()) return text.substr(0, digit) + "0";
    return text.substr(0, digit) + text.substr(first);
}

std::string sectionToken(std::string_view base) {
    const std::size_t at = base.find("section");
    if (at == std::string::npos) return {};
    std::size_t start = at + 7;
    while (start < base.size() &&
           !std::isalnum(static_cast<unsigned char>(base[start]))) ++start;
    std::size_t end = start;
    while (end < base.size() && std::isalnum(static_cast<unsigned char>(base[end]))) ++end;
    return stripZeros(std::string(base.substr(start, end - start)));
}

struct CollisionCandidate {
    std::string hint;     // directory hint (endless, super, wily4, ...)
    std::string base;     // normalized scene name to compare with the image
    int priority = 0;     // aliases outrank the literal scene name
};

bool startsWith(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
}

std::string sectionHint(std::string_view base) {
    const std::size_t at = base.find("section");
    if (at == std::string::npos) return std::string(base);
    std::string hint(base.substr(0, at));
    while (!hint.empty() && (hint.back() == '_' || hint.back() == '-')) hint.pop_back();
    return hint;
}

std::vector<CollisionCandidate> collisionCandidates(const std::string& sceneBase) {
    std::vector<CollisionCandidate> out;
    out.push_back({sectionHint(sceneBase), sceneBase, 0});

    // Wily 4 refights abbreviate the Robot Master name in the .xscn file but
    // spell it out in the packed collision image.
    static constexpr std::array<std::pair<const char*, const char*>, 9> kWily4 = {{
        {"cwoman", "cometwoman"}, {"gman", "glueman"},   {"jman", "jetman"},
        {"nman", "nailman"},      {"rman", "rainbowman"}, {"taman", "tankman"},
        {"trman", "trinitroman"}, {"ykman", "yokuman"},   {"yyman", "yoyoman"},
    }};
    if (startsWith(sceneBase, "wily4_section_")) {
        const std::string token = sectionToken(sceneBase);
        for (const auto& alias : kWily4) {
            const std::string from(alias.first);
            if (startsWith(token, from) && token.size() > from.size()) {
                out.push_back({"wily4",
                               std::string("wily4_") + alias.second + "section" +
                                   token.substr(from.size()),
                               2});
            }
        }
    }

    // The editor calls him NitroMan; the art and collision baker use the full
    // TrinitroMan name.
    if (startsWith(sceneBase, "nitroman")) {
        out.push_back({"trinitroman",
                       std::string("trinitroman") + sceneBase.substr(std::strlen("nitroman")),
                       2});
    }

    // Gauntlet rooms reuse the Endless collision surfaces verbatim.
    if (startsWith(sceneBase, "endless") || startsWith(sceneBase, "gauntlet")) {
        const std::size_t prefix = startsWith(sceneBase, "endless") ? 7 : 8;
        out.push_back({"endless", std::string("endless") + sceneBase.substr(prefix), 2});

        // The two 3H3H rooms are named _001A/_001B in .xscn files but _001/_002
        // in the packed collision images.
        const std::size_t at = sceneBase.find("3h3h_");
        if (at != std::string::npos && !sceneBase.empty()) {
            const char variant = sceneBase.back();
            if (variant == 'a' || variant == 'b')
                out.push_back({"endless",
                               std::string("endless3h3h_") +
                                   (variant == 'a' ? "001" : "002"),
                               3});
        }
    }

    // Super Robot Master stages share a small set of full-room surfaces.
    if (startsWith(sceneBase, "super")) {
        const std::string rest = sceneBase.substr(5);
        const std::size_t at = rest.find("section");
        std::string boss = rest.substr(0, at == std::string::npos ? rest.size() : at);
        while (!boss.empty() && (boss.back() == '_' || boss.back() == '-')) boss.pop_back();
        const std::string token = sectionToken(sceneBase);
        std::string room;
        if (boss == "yoyoman") room = "superyoyomanroom";
        else if (boss == "yokuman") room = "superyokumanroom";
        else if (boss == "glueman") room = "supergluemanroom";
        else if (boss == "jetman")
            room = token == "1" ? "superjetmanintro" : "superjetmanroom";
        else if (boss == "whirlpoolman")
            room = token == "1" ? "superwhirlpoolmanintro" : "superwhirlpoolmanroom";
        else if (boss == "trinitroman")
            room = token.find("nor") != std::string::npos ? "normaltrinitromanroom"
                                                          : "supertrinitromanroom";
        else if (boss == "tankman" || boss == "rainbowman" || boss == "nailman" ||
                 boss == "cometwoman" || boss == "tankrainbowcometnailman")
            room = "supertankrainbowcometnailmanroom";
        if (!room.empty()) out.push_back({"super", room, 3});
    }
    return out;
}

int scoreCollisionCandidate(const CollisionCandidate& candidate,
                            const std::string& colBase, const std::string& colDir) {
    if (!dirCompatible(colDir, candidate.hint)) return -1;

    const bool mapped = candidate.base.find("section") != std::string::npos;
    const std::string token = mapped ? sectionToken(candidate.base) : std::string();
    int best = -1;
    auto scoreName = [&](const std::string& name) {
        int local = -1;
        if (mapped) {
            if (!token.empty() && sectionToken(name) == token) local = 2;
        } else if (name == candidate.base) {
            local = 2;
        } else if (name.compare(0, candidate.base.size() + 1,
                                candidate.base + "_") == 0) {
            local = 1;
        }
        best = std::max(best, local);
    };
    scoreName(colBase);
    if (startsWith(colBase, "coll")) scoreName(colBase.substr(4));
    return best < 0 ? -1 : candidate.priority * 10 + best;
}

int trailingNumber(std::string_view base) {
    const std::size_t lastUnderscore = base.rfind('_');
    if (lastUnderscore == std::string::npos) return 0;
    const std::string tail(base.substr(lastUnderscore + 1));
    if (tail.empty() || !std::isdigit(static_cast<unsigned char>(tail.front()))) return 0;
    return std::atoi(tail.c_str());
}

} // namespace

SceneArt resolveSceneArt(const PackedArchive& pak, const PackedScene& scene) {
    SceneArt art;
    if (scene.name.empty()) return art;

    const std::string sceneBase = basename(scene.name);
    const bool mapped = sceneBase.find("section") != std::string::npos;
    const std::string sceneToken = sectionToken(sceneBase);
    std::string hint;
    if (mapped) {
        const std::size_t at = sceneBase.find("section");
        hint = sceneBase.substr(0, at);
        while (!hint.empty() && (hint.back() == '_' || hint.back() == '-'))
            hint.pop_back();
    } else {
        hint = sceneBase;
    }

    std::vector<std::pair<u32, std::string>> matches; // (entry index, art basename)
    for (std::size_t i = 0; i < pak.entries().size(); ++i) {
        const PackedEntry& entry = pak.entries()[i];
        if (entry.source != PackedSource::LevelArt ||
            entry.type != PackedEntryType::Image || entry.name.empty())
            continue;
        const std::string lower = toLowerAscii(entry.name);
        if (lower.find("/art/") == std::string::npos &&
            lower.find("\\art\\") == std::string::npos)
            continue;
        if (lower.find("collision") != std::string::npos) continue;

        const std::string artBase = basename(entry.name);
        const std::string artDir = levelDir(entry.name);
        bool matched = false;
        if (mapped) {
            matched = !sceneToken.empty() && sectionToken(artBase) == sceneToken &&
                      dirCompatible(artDir, hint);
        } else {
            matched = (artBase == sceneBase ||
                       artBase.compare(0, sceneBase.size() + 1, sceneBase + "_") == 0) &&
                      dirCompatible(artDir, hint);
            if (!matched && sceneBase.compare(0, 8, "gauntlet") == 0) {
                const std::string endless = "endless" + sceneBase.substr(8);
                matched = (artBase == endless ||
                           artBase.compare(0, endless.size() + 1, endless + "_") == 0) &&
                          dirCompatible(artDir, "endless");
            }
        }
        if (matched) matches.emplace_back(static_cast<u32>(i), artBase);
    }

    std::sort(matches.begin(), matches.end(),
              [](const auto& a, const auto& b) {
                  const int na = trailingNumber(a.second);
                  const int nb = trailingNumber(b.second);
                  return na != nb ? na < nb : a.first < b.first;
              });
    art.frames.reserve(matches.size());
    for (const auto& match : matches) art.frames.push_back(match.first);
    if (!art.frames.empty()) {
        const PackedEntry& first = pak.entries()[art.frames.front()];
        art.width = first.width;
        art.height = first.height;
    }
    return art;
}

bool CollisionMap::solidAt(i32 x, i32 y) const {
    if (!valid()) return false;
    // Allow a small player box to straddle the image edge; beyond that the
    // scene is bounded by walls.
    if (x < -8 || x >= static_cast<i32>(width) + 8) return true;
    if (y < 0) return false;
    if (static_cast<u32>(y) >= height) return true;          // scene floor
    return solid[static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)] != 0;
}

bool CollisionMap::solidAt(float x, float y) const {
    return solidAt(static_cast<i32>(std::floor(x)), static_cast<i32>(std::floor(y)));
}

u32 findSceneCollisionImage(const PackedArchive& pak, const PackedScene& scene) {
    if (scene.name.empty()) return CollisionMap::kNoImage;

    const std::string sceneBase = basename(scene.name);
    const std::vector<CollisionCandidate> candidates = collisionCandidates(sceneBase);

    u32 bestIndex = CollisionMap::kNoImage;
    int bestScore = -1;
    for (std::size_t i = 0; i < pak.entries().size(); ++i) {
        const PackedEntry& entry = pak.entries()[i];
        if (entry.source != PackedSource::LevelArt ||
            entry.type != PackedEntryType::Image || entry.name.empty())
            continue;
        const std::string lower = toLowerAscii(entry.name);
        if (lower.find("collision") == std::string::npos) continue;

        const std::string colBase = basename(entry.name);
        const std::string colDir = levelDir(entry.name);
        for (const CollisionCandidate& candidate : candidates) {
            const int score = scoreCollisionCandidate(candidate, colBase, colDir);
            if (score > bestScore) {
                bestScore = score;
                bestIndex = static_cast<u32>(i);
            }
        }
    }
    return bestIndex;
}

bool collisionImageLoadable(const PackedEntry& entry) {
    if (entry.data.empty() || entry.width == 0 || entry.height == 0) return false;
    const bool indexed = entry.bitsPerPixel == 8 && entry.palette.size >= 3;
    const std::size_t bpp = indexed ? 1u : entry.bytesPerPixel;
    if (bpp == 0) return false;
    const std::size_t stride = entry.pitch ? entry.pitch
                                           : static_cast<std::size_t>(entry.width) * bpp;
    return stride >= static_cast<std::size_t>(entry.width) * bpp;
}

u8 directChannel(u32 pixel, u32 mask, u32 shift, u32 loss) {
    if (!mask) return 255;
    const u32 bits = (pixel & mask) >> shift;
    if (loss >= 8) return static_cast<u8>(bits);
    return static_cast<u8>((bits << loss) | (bits >> (8 - loss)));
}

CollisionMap resolveSceneCollision(const PackedArchive& pak, const PackedScene& scene) {
    CollisionMap map;
    const u32 bestIndex = findSceneCollisionImage(pak, scene);
    if (bestIndex == CollisionMap::kNoImage) return map;

    const PackedEntry& entry = pak.entries()[bestIndex];
    if (!collisionImageLoadable(entry)) return map;

    const bool indexed = entry.bitsPerPixel == 8 && entry.palette.size >= 3;
    const std::size_t bpp = indexed ? 1u : entry.bytesPerPixel;
    const std::size_t stride = entry.pitch ? entry.pitch
                                           : static_cast<std::size_t>(entry.width) * bpp;

    map.imageIndex = bestIndex;
    map.width = entry.width;
    map.height = entry.height;
    map.solid.assign(static_cast<std::size_t>(entry.width) * entry.height, 0);
    for (u32 y = 0; y < entry.height; ++y) {
        const u8* row = entry.data.data + static_cast<std::size_t>(y) * stride;
        for (u32 x = 0; x < entry.width; ++x) {
            const u8* px = row + static_cast<std::size_t>(x) * bpp;
            if (px + bpp > entry.data.data + entry.data.size) continue;

            u8 r = 0, g = 0, b = 0;
            if (indexed) {
                const u32 index = *px;
                if (index * 3 + 2 >= entry.palette.size) continue;
                r = entry.palette.data[index * 3];
                g = entry.palette.data[index * 3 + 1];
                b = entry.palette.data[index * 3 + 2];
            } else {
                u32 pixel = px[0];
                if (bpp >= 2) pixel |= static_cast<u32>(px[1]) << 8;
                if (bpp >= 3) pixel |= static_cast<u32>(px[2]) << 16;
                if (bpp >= 4) pixel |= static_cast<u32>(px[3]) << 24;
                r = directChannel(pixel, entry.rMask, entry.rShift, entry.rLoss);
                g = directChannel(pixel, entry.gMask, entry.gShift, entry.gLoss);
                b = directChannel(pixel, entry.bMask, entry.bShift, entry.bLoss);
            }
            // Solid white terrain and red hazard tiles both block movement.
            const bool blocking = (r == 255 && g == 255 && b == 255) ||
                                  (r == 255 && g == 0 && b == 0);
            map.solid[static_cast<std::size_t>(y) * entry.width + x] = blocking ? 1 : 0;
        }
    }
    return map;
}

} // namespace mmu
