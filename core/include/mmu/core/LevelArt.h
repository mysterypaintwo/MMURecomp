#pragma once

#include "mmu/core/PackedArchive.h"

#include <vector>

namespace mmu {

// A scene's background art. The original levelart.mmu stores each animated
// background as several full-width images; the runtime cycles them like any
// other sprite animation. Width/height are those of a single frame image.
struct SceneArt {
    std::vector<u32> frames; // packed image indices, in animation order
    float frameDuration = 0.15f;
    u32 width = 256;
    u32 height = 224;
    bool empty() const { return frames.empty(); }
};

// Matches the scene's editor-side .xscn name against packed levelart paths,
// e.g. "\Levels\YoyoMan_Section_1.xscn" ->
//   "..\Levels\YoyoMan\Sprites\../Art/Section1_*.png".
// Returns an empty result for v2/v3 archives or unmatched scenes.
SceneArt resolveSceneArt(const PackedArchive& pak, const PackedScene& scene);

// The original levelart.mmu also ships one 8-bit indexed collision image per
// room section. White is solid, red is a solid hazard, black is open space;
// the remaining palette entries mark ladders/water and are not collidable.
// Coordinates are the same logical pixels used by PackedSpawn and WorldObject.
struct CollisionMap {
    static constexpr u32 kNoImage = 0xFFFFFFFFu;

    u32 imageIndex = kNoImage;
    u32 width = 0;
    u32 height = 0;
    std::vector<u8> solid; // width * height, row-major from the packed image

    bool valid() const { return imageIndex != kNoImage && !solid.empty(); }
    bool solidAt(float x, float y) const;
    bool solidAt(i32 x, i32 y) const;
};

// Lookup is split from bitmap construction so coverage queries over all 791
// scenes do not decode 336 full-room collision images just to count matches.
u32 findSceneCollisionImage(const PackedArchive& pak, const PackedScene& scene);
bool collisionImageLoadable(const PackedEntry& entry);
CollisionMap resolveSceneCollision(const PackedArchive& pak, const PackedScene& scene);

} // namespace mmu
