#pragma once

// Pixel-exact collision, reconstructed from MMU.exe.
//
// The original engine has no tiles or boxes. Every level owns a full-room
// collision image whose colours encode a 32-bit type mask, and every sprite
// frame owns a collision surface coloured the same way. A test places the
// object's current collision surface at its handle position and ORs together
// the level types under each of its non-black pixels.
//
//   0x445130  colour -> type table (collisionTypeForRgb)
//   0x5b4970  Environment::Load: level image -> per-pixel type map (TypeMap)
//   0x5b48c0  Environment::TypeAt(screen x, y) (+ camera, bounds-checked)
//   0x445b20  World::TestSurface(obj, surface, x, y, mask, contacts)
//   0x5b5220  MegaMan's pixel-stepping sweep mover (sweepMove)
//
// The engine also rasterises live objects into a 456x424 screen-space grid
// (0x445770 clears it each frame, cells hold {type, owner} lists). Object
// entries are not modelled yet; only the level Environment contributes.

#include "mmu/core/PackedArchive.h"
#include "mmu/core/Types.h"

#include <unordered_map>
#include <vector>

namespace mmu {

// Bits returned by 0x445130. Names are given only where a MegaMan code path
// pins down the meaning; the rest are listed by their exact source colour.
namespace collision {
constexpr u32 Solid = 0x00000001u;        // (255,255,255) white terrain
constexpr u32 SolidAlt = 0x00000002u;     // (0,0,127)   also in the default mover mask 3
constexpr u32 Hazard = 0x00000004u;       // (255,0,0)   joins the mover mask when +0x1a0 is set
constexpr u32 NoWalk = 0x00000008u;       // (255,200,0) Stand/Step/Run/Slide refuse input on it
constexpr u32 Ice = 0x00000010u;          // (0,255,255) keeps momentum, 80 px/s^2 slide-out
constexpr u32 Ladder = 0x00000020u;       // (255,0,255) climbable; ladder tops are one-way
constexpr u32 Type40 = 0x00000040u;       // (255,255,0)
constexpr u32 Type80 = 0x00000080u;       // (145,0,145)
constexpr u32 Type100 = 0x00000100u;      // (110,0,210)
constexpr u32 OneWayA = 0x00000200u;      // (110,110,0) jump-through, ignored while overlapping
constexpr u32 Type400 = 0x00000400u;      // (100,100,100)
constexpr u32 Type800 = 0x00000800u;      // (255,123,0)
constexpr u32 PushLeft = 0x00001000u;     // (0,123,255) Fall: vx = -275 while touching from above
constexpr u32 PushRight = 0x00002000u;    // (0,255,123) Fall: vx = +275
constexpr u32 StopRun = 0x00004000u;      // (100,255,123) Run sets vx = 0
constexpr u32 WindLeft = 0x00008000u;     // (255,200,50) Fall pushes -50 px/s
constexpr u32 WindRight = 0x00010000u;    // (255,200,100) Fall pushes +50 px/s
constexpr u32 OneWayB = 0x00020000u;      // (0,255,0)   jump-through, ignored while overlapping
constexpr u32 Type40000 = 0x00040000u;    // (192,192,192)
constexpr u32 Type80000 = 0x00080000u;    // (255,255,100)
constexpr u32 Type100000 = 0x00100000u;   // (185,122,87)
constexpr u32 Type200000 = 0x00200000u;   // (155,0,255)
constexpr u32 Type400000 = 0x00400000u;   // (255,100,0)
constexpr u32 GravityA = 0x00800000u;     // (0,80,40)   0x5b15d0 gravity-flip zone
constexpr u32 GravityB = 0x01000000u;     // (0,40,80)   0x5b15d0 gravity-flip zone
constexpr u32 Water = 0x02000000u;        // (50,80,130) gravity 375 while inside (0x43c840)
constexpr u32 Type4000000 = 0x04000000u;  // (50,80,255)
constexpr u32 Type8000000 = 0x08000000u;  // (255,0,155)
constexpr u32 Bounce = 0x10000000u;       // (255,211,255) forces vy = -60
constexpr u32 Type20000000 = 0x20000000u; // (100,0,0)
constexpr u32 Type40000000 = 0x40000000u; // (175,0,0)
constexpr u32 Type80000000 = 0x80000000u; // (0,0,255)

constexpr u32 OneWay = OneWayA | OneWayB;
} // namespace collision

// Exact port of 0x445130 (SDL_GetRGB channel order).
u32 collisionTypeForRgb(u8 r, u8 g, u8 b);

// Reads one pixel of a packed SDL surface as RGB (palette or channel masks).
// Out-of-range coordinates read as black, like 0x4458b0.
bool readPackedRgb(const PackedEntry& entry, i32 x, i32 y, u8& r, u8& g, u8& b);

// Per-pixel collision types for one surface (level room or sprite frame).
struct TypeMap {
    u32 width = 0;
    u32 height = 0;
    std::vector<u32> types;

    bool valid() const { return width != 0 && height != 0; }
    u32 at(i32 x, i32 y) const {
        if (x < 0 || y < 0 || static_cast<u32>(x) >= width || static_cast<u32>(y) >= height)
            return 0;
        return types[static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)];
    }
};

// 0x5b4970 / sprite collision surfaces. flipX mirrors horizontally, as the
// mover does with zoomSurface(-1, 1) for left-facing objects.
TypeMap buildTypeMap(const PackedEntry& entry, bool flipX = false);

// Placement of a collision surface: the surface's top-left is
// floor(position) - handle.
struct CollisionShape {
    const TypeMap* mask = nullptr;
    i32 handleX = 0;
    i32 handleY = 0;
};

struct CollisionContact {
    float dx = 0.0f; // pixel offset from floor(position)
    float dy = 0.0f;
    u32 type = 0;
};

// Shared inputs of every query: the room's type map and the camera, since the
// original tests in screen space and adds the truncated camera back in.
struct CollisionScene {
    const TypeMap* level = nullptr;
    float cameraX = 0.0f;
    float cameraY = 0.0f;
};

// 0x445b20 restricted to the level Environment. (screenX, screenY) is the
// object position relative to the camera. Returns the OR of matching types
// (non-zero on a hit); collects every hit pixel when contacts is non-null.
u32 testShape(const CollisionScene& scene, const CollisionShape& shape, float screenX,
              float screenY, u32 typeMask, std::vector<CollisionContact>* contacts = nullptr);

// Convenience: test at a world position (FUN_00445ea0 callers pass pos+offset).
u32 testShapeAt(const CollisionScene& scene, const CollisionShape& shape, float worldX,
                float worldY, u32 typeMask, std::vector<CollisionContact>* contacts = nullptr);

// FUN_005b4f30: screen-space y used by the mover is clamped to [1, 223].
float clampSweepScreenY(float y);

struct SweepResult {
    bool completed = false; // first byte of the 0x4201a0 result struct
    float x = 0.0f;         // resolved position
    float y = 0.0f;
    u32 flags = 0;          // 1/2 = blocked moving up/down, 4/8 = blocked left/right
};

// Port of 0x5b5220. Moves (x, y) by velocity*dt one pixel at a time along the
// dominant axis, testing typeMask at every step. vy is zeroed when a
// vertical-major sweep cannot advance (the caller's landing/ceiling test).
// gravity is the object's current gravity (sign decides which one-way
// platforms apply). stopOnHit mirrors param_9.
SweepResult sweepMove(const CollisionScene& scene, const CollisionShape& shape, float x,
                      float y, float& vx, float& vy, float dt, float gravity, u32 typeMask,
                      bool stopOnHit = false);

// Lazily built, flip-aware collision masks for packed sprite frames.
class CollisionMaskCache {
public:
    const TypeMap* get(const PackedArchive& pak, u32 imageIndex, bool flipX);
    void clear() { cache_.clear(); }

private:
    std::unordered_map<u64, TypeMap> cache_;
};

} // namespace mmu
