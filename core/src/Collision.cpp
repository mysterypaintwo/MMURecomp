#include "mmu/core/Collision.h"

#include <cmath>

namespace mmu {
namespace {

u8 directChannel(u32 pixel, u32 mask, u32 shift, u32 loss) {
    if (!mask) return 0;
    const u32 bits = (pixel & mask) >> shift;
    if (loss >= 8) return static_cast<u8>(bits);
    return static_cast<u8>((bits << loss) | (bits >> (8 - loss)));
}

// _ftol (0x6253a0): truncate toward zero.
i32 ftol(float value) { return static_cast<i32>(value); }

float absf(float v) { return v < 0.0f ? -v : v; }      // 0x5b4f70
float signOf(float v) { return v < 0.0f ? -1.0f : 1.0f; } // 0x5b4fa0 (0 -> +1)

// The 456x424 screen-space grid covers x in [-100, 356), y in [-100, 324).
bool insideGrid(i32 x, i32 y) { return x >= -100 && x < 356 && y >= -100 && y < 324; }

} // namespace

u32 collisionTypeForRgb(u8 r, u8 g, u8 b) {
    // Kept in the original test order: several colours share channels and
    // the first match wins.
    if (b == 0 && g == 0 && r == 0) return 0;
    if (b == 123 && g == 255 && r == 100) return 0x4000;
    if (b == 255 && g == 255 && r == 255) return 0x1;
    if (b == 192 && g == 192 && r == 192) return 0x40000;
    if (r == 0) {
        if (g == 80 && b == 40) return 0x800000;
        if (g == 40 && b == 80) return 0x1000000;
    }
    if (b == 87 && g == 122 && r == 185) return 0x100000;
    if (b == 255 && g == 255 && r == 0) return 0x10;
    if (r == 255 && g == 200) {
        if (b == 0) return 0x8;
        if (b == 50) return 0x8000;
        if (b == 100) return 0x10000;
    } else if (b == 100) {
        if (g == 100 && r == 100) return 0x400;
        if (g == 255 && r == 255) return 0x80000;
    }
    if (b == 255) {
        if (g == 0 && r == 255) return 0x20;
        if (g == 0 && r == 0) return 0x80000000u;
    }
    if (r == 255 && g == 100) {
        if (b == 0) return 0x400000;
    } else if (b == 0 && g == 255 && r == 255) {
        return 0x40;
    }
    if (r == 145 && g == 0 && b == 145) return 0x80;
    if (r == 110 && g == 0 && b == 210) return 0x100;
    if (r == 255) {
        if (g == 0 && b == 0) return 0x4;
        if (g == 123 && b == 0) return 0x800;
    }
    if (r == 50 && g == 80 && b == 255) return 0x4000000;
    if (r == 255 && g == 0 && b == 155) return 0x8000000;
    if (r == 100 && g == 0 && b == 0) return 0x20000000;
    if (r == 175 && g == 0 && b == 0) return 0x40000000;
    if (r == 110 && g == 110 && b == 0) return 0x200;
    if (r == 0 && g == 255 && b == 0) return 0x20000;
    if (r == 155 && g == 0 && b == 255) return 0x200000;
    if (r == 0) {
        if (g == 123 && b == 255) return 0x1000;
        if (g == 255 && b == 123) return 0x2000;
    }
    if (r == 50 && g == 80 && b == 130) return 0x2000000;
    if (r == 255 && g == 211 && b == 255) return 0x10000000;
    if (r == 0 && g == 0 && b == 127) return 0x2;
    return 0;
}

bool readPackedRgb(const PackedEntry& entry, i32 x, i32 y, u8& r, u8& g, u8& b) {
    r = g = b = 0;
    if (x < 0 || y < 0 || static_cast<u32>(x) >= entry.width ||
        static_cast<u32>(y) >= entry.height || entry.data.empty())
        return false;
    const bool indexed = entry.bitsPerPixel == 8 && entry.palette.size >= 3;
    const std::size_t bpp = indexed ? 1u : entry.bytesPerPixel;
    if (bpp == 0) return false;
    const std::size_t stride = entry.pitch ? entry.pitch : static_cast<std::size_t>(entry.width) * bpp;
    const std::size_t offset = static_cast<std::size_t>(y) * stride + static_cast<std::size_t>(x) * bpp;
    if (offset + bpp > entry.data.size) return false;
    const u8* px = entry.data.data + offset;
    if (indexed) {
        const std::size_t index = *px;
        if (index * 3 + 2 >= entry.palette.size) return false;
        r = entry.palette.data[index * 3];
        g = entry.palette.data[index * 3 + 1];
        b = entry.palette.data[index * 3 + 2];
        return true;
    }
    u32 pixel = px[0];
    if (bpp >= 2) pixel |= static_cast<u32>(px[1]) << 8;
    if (bpp >= 3) pixel |= static_cast<u32>(px[2]) << 16;
    if (bpp >= 4) pixel |= static_cast<u32>(px[3]) << 24;
    r = directChannel(pixel, entry.rMask, entry.rShift, entry.rLoss);
    g = directChannel(pixel, entry.gMask, entry.gShift, entry.gLoss);
    b = directChannel(pixel, entry.bMask, entry.bShift, entry.bLoss);
    return true;
}

TypeMap buildTypeMap(const PackedEntry& entry, bool flipX) {
    TypeMap map;
    if (entry.type != PackedEntryType::Image || entry.width == 0 || entry.height == 0)
        return map;
    map.width = entry.width;
    map.height = entry.height;
    map.types.assign(static_cast<std::size_t>(map.width) * map.height, 0);
    for (u32 y = 0; y < map.height; ++y) {
        for (u32 x = 0; x < map.width; ++x) {
            u8 r, g, b;
            if (!readPackedRgb(entry, static_cast<i32>(x), static_cast<i32>(y), r, g, b)) continue;
            const u32 dstX = flipX ? map.width - 1 - x : x;
            map.types[static_cast<std::size_t>(y) * map.width + dstX] = collisionTypeForRgb(r, g, b);
        }
    }
    return map;
}

u32 testShape(const CollisionScene& scene, const CollisionShape& shape, float screenX,
              float screenY, u32 typeMask, std::vector<CollisionContact>* contacts) {
    if (!scene.level || !scene.level->valid() || !shape.mask || !shape.mask->valid()) return 0;
    const float baseX = std::floor(screenX);
    const float baseY = std::floor(screenY);
    const i32 x0 = ftol(baseX - static_cast<float>(shape.handleX));
    const i32 y0 = ftol(baseY - static_cast<float>(shape.handleY));
    const i32 camX = ftol(scene.cameraX);
    const i32 camY = ftol(scene.cameraY);
    u32 hit = 0;
    for (u32 i = 0; i < shape.mask->width; ++i) {
        const i32 px = x0 + static_cast<i32>(i);
        for (u32 j = 0; j < shape.mask->height; ++j) {
            const i32 py = y0 + static_cast<i32>(j);
            if (!insideGrid(px, py)) continue;
            if (shape.mask->at(static_cast<i32>(i), static_cast<i32>(j)) == 0) continue;
            const u32 type = scene.level->at(camX + px, camY + py);
            if ((type & typeMask) == 0) continue;
            hit |= type & typeMask;
            if (!contacts) return hit;
            contacts->push_back(CollisionContact{static_cast<float>(px) - baseX,
                                                 static_cast<float>(py) - baseY, type});
        }
    }
    return hit;
}

u32 testShapeAt(const CollisionScene& scene, const CollisionShape& shape, float worldX,
                float worldY, u32 typeMask, std::vector<CollisionContact>* contacts) {
    return testShape(scene, shape, worldX - scene.cameraX, worldY - scene.cameraY, typeMask,
                     contacts);
}

float clampSweepScreenY(float y) {
    if (y > 0.0f) return y >= 224.0f ? 223.0f : y;
    return 1.0f;
}

SweepResult sweepMove(const CollisionScene& scene, const CollisionShape& shape, float x,
                      float y, float& vx, float& vy, float dt, float gravity, u32 typeMask,
                      bool stopOnHit) {
    SweepResult result;
    result.x = x;
    result.y = y;
    if (vx == 0.0f && vy == 0.0f) {
        result.completed = true;
        return result;
    }
    const float camX = scene.cameraX;
    const float camY = scene.cameraY;
    const auto blocked = [&](float worldX, float worldY, u32 mask) {
        const float sx = std::floor(worldX - camX);
        const float sy = clampSweepScreenY(std::floor(worldY - camY));
        return testShape(scene, shape, sx, sy, mask) != 0;
    };
    const float targetX = x + vx * dt;
    const float targetY = y + vy * dt;
    float curX = x, curY = y;
    u32 flags = 0;

    // One-way platforms only block a body moving with gravity that is not
    // already overlapping them.
    const bool falling = signOf(vy) == signOf(gravity);
    u32 mask = typeMask;
    if (falling) {
        if (blocked(x, y, collision::OneWayA)) mask &= ~collision::OneWayA;
        if (blocked(x, y, collision::OneWayB)) mask &= ~collision::OneWayB;
    } else {
        mask &= ~collision::OneWay;
    }
    const u32 noOneWay = mask & ~collision::OneWay;
    const float stepX = vx * dt;
    const float stepY = vy * dt;

    if (absf(vy) < absf(vx)) {
        // Horizontal-major.
        for (float i = 0.0f; absf(i) <= absf(stepX) + 1.0f; i += signOf(stepX)) {
            float offX = i;
            if (absf(targetX - x) < absf(i)) offX = targetX - x;
            const float offY = vx == 0.0f ? 0.0f : (vy / vx) * offX;
            const u32 stepMask = offY == 0.0f ? noOneWay : mask;
            if (!blocked(x + offX, y + offY, stepMask)) {
                curX = x + offX;
                curY = y + offY;
            } else {
                if (stopOnHit) {
                    result.flags = flags | (vx <= 0.0f ? 4u : 8u);
                    result.x = curX;
                    result.y = curY;
                    return result;
                }
                if (!blocked(x + offX, curY, noOneWay)) {
                    flags |= vy <= 0.0f ? 1u : 2u;
                    curX = x + offX;
                } else {
                    flags |= vx <= 0.0f ? 4u : 8u;
                    if (absf(stepY) == 0.0f) {
                        result.flags = flags;
                        result.x = curX;
                        result.y = curY;
                        return result;
                    }
                    if (blocked(curX, y + offY, mask)) {
                        result.flags = flags | (falling ? 2u : 1u);
                        result.x = curX;
                        result.y = curY;
                        return result;
                    }
                    curY = y + offY;
                }
            }
        }
    } else {
        // Vertical-major.
        bool advanced = false;
        for (float i = 0.0f; absf(i) <= absf(stepY) + 1.0f; i += signOf(stepY)) {
            float offY = i;
            if (absf(targetY - y) < absf(i)) offY = targetY - y;
            const float offX = vx == 0.0f ? 0.0f : (vx / vy) * offY;
            if (!blocked(x + offX, y + offY, mask)) {
                advanced = true;
                curX = x + offX;
                curY = y + offY;
                continue;
            }
            if (stopOnHit) {
                result.flags = flags | (vy <= 0.0f ? 1u : 2u);
                result.x = curX;
                result.y = curY;
                return result;
            }
            if (!blocked(curX, y + offY, mask)) {
                // Vertical motion alone is free: try to keep the x step too.
                if (!blocked(x + offX, curY, noOneWay)) {
                    advanced = true;
                    curX = x + offX;
                    curY = y + offY;
                } else {
                    flags |= vx <= 0.0f ? 4u : 8u;
                    curY = y + offY;
                }
                continue;
            }
            advanced = false;
            if (absf(stepX) == 0.0f || blocked(x + offX, curY, noOneWay)) {
                vy = 0.0f;
                result.flags = flags | (falling ? 2u : 1u);
                result.x = curX;
                result.y = curY;
                return result;
            }
            flags |= vy <= 0.0f ? 1u : 2u;
            curX = x + offX;
        }
        if (!advanced) vy = 0.0f;
    }

    if (absf(targetY - y) < absf(curY - y)) curY = targetY;
    if (absf(targetX - x) < absf(curX - x)) curX = targetX;
    result.completed = true;
    result.x = curX;
    result.y = curY;
    result.flags = flags;
    return result;
}

const TypeMap* CollisionMaskCache::get(const PackedArchive& pak, u32 imageIndex, bool flipX) {
    if (imageIndex >= pak.entries().size()) return nullptr;
    const u64 key = (static_cast<u64>(imageIndex) << 1) | (flipX ? 1u : 0u);
    auto it = cache_.find(key);
    if (it == cache_.end())
        it = cache_.emplace(key, buildTypeMap(pak.entries()[imageIndex], flipX)).first;
    return it->second.valid() ? &it->second : nullptr;
}

} // namespace mmu
