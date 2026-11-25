#pragma once

#include "mmu/core/EnemyState.h"
#include "mmu/core/LevelArt.h"
#include "mmu/core/PackedArchive.h"
#include "mmu/core/PlayerState.h"
#include "mmu/core/Types.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mmu {

// Case-insensitive ASCII helper shared by the runtime and the debug tools.
std::string toLowerAscii(std::string_view text);

// Scene spawns reference objects by string type name. The original databases
// have no explicit type->sprite table, so resolve it from the packed image
// paths (e.g. type "EZBlock" -> ".../Platforms/EZBlock/Art/EZBlock.png").
using TypeImageMap = std::unordered_map<std::string, u32>;
TypeImageMap resolveTypeImages(const PackedArchive& pak);

// A live object instantiated from one PackedSpawn. This is deliberately a
// plain data struct: the real gameplay logic will operate on these records.
struct WorldObject {
    u32 id = 0;
    std::string type;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float autoRespawn = 0.0f;
    bool facingRight = false;
    bool respawnOnDeath = false;
    bool respawnOnScreen = false;
    bool deleteOffScreen = false;
    bool collectible = false;
    bool active = true;
    std::string attr1;
    std::string attr2;
    u32 imageIndex = 0xFFFFFFFFu; // resolved art, if any

    // Runtime-spawned projectile metadata. Scene objects leave this as None.
    SpawnedObjectKind projectile = SpawnedObjectKind::None;
    float projectileAge = 0.0f;
    float delay = 0.0f;
    float lifetime = 0.0f;
    float targetX = 0.0f;
    float targetY = 0.0f;
    float damage = 0.0f;
    u32 sequence = 0;
    u32 ownerIndex = 0xFFFFFFFFu;

    // Per-projectile state scratch. WilyMachinePillar uses phases
    // Follow=0, Drop=1, Shoot=2, Lift=3; spawnedCount and phaseTime drive its
    // recovered block volley. Other projectile kinds leave these untouched.
    u32 projectilePhase = 0;
    float phaseTime = 0.0f;
    u32 spawnedCount = 0;
};

// Simple follow camera in the game's logical 256x224 coordinate space. This
// blend/clamp is harness policy, not the reconstructed Camera vtable; exact
// Camera/CameraTarget VAs, slots and operands are in CameraEvidenceTable.h.
struct Camera {
    float x = 0.0f;   // top-left of the viewport
    float y = 0.0f;
    float width = 256.0f;
    float height = 224.0f;

    void follow(float targetX, float targetY, float blend);
    void clamp(float worldWidth, float worldHeight);
};

// Platform-neutral world state for one scene. No SDL, no ImGui.
class World {
public:
    void clear();
    bool loadScene(const PackedScene& scene, const TypeImageMap& typeImages);
    // Also resolves the scene's packed collision image and attaches it to the
    // player controller.
    bool loadScene(const PackedArchive& pak, const PackedScene& scene,
                   const TypeImageMap& typeImages);

    // Advances the simulation by one fixed step. Player input is expressed as
    // a desired velocity in logical pixels per second.
    void step(float dt, float playerVX, float playerVY);
    // State-driven step: advances the recovered MegaMan controller and syncs
    // the resulting position/facing into the player WorldObject.
    void step(float dt, const PlayerInput& input);

    PlayerController& controller() { return controller_; }
    const PlayerController& controller() const { return controller_; }
    const CollisionMap& collision() const { return collision_; }
    // Per-pixel collision types of the loaded room (empty without an image).
    const TypeMap& levelTypes() const { return levelTypes_; }
    // MegaMan's guns.mmu object (animations + GameParams), if present.
    const PackedGameObject* playerObject() const { return megaman_; }
    const PackedArchive* archive() const { return pak_; }
    const std::vector<EnemyController>& enemies() const { return enemies_; }
    std::vector<EnemyController>& enemies() { return enemies_; }

    std::vector<WorldObject>& objects() { return objects_; }
    const std::vector<WorldObject>& objects() const { return objects_; }
    WorldObject* player();
    const WorldObject* player() const;
    Camera& camera() { return camera_; }
    const Camera& camera() const { return camera_; }
    bool hasScene() const { return hasScene_; }
    u32 sceneKey() const { return sceneKey_; }
    float time() const { return time_; }
    float worldWidth() const { return worldWidth_; }
    float worldHeight() const { return worldHeight_; }
    std::size_t projectileCount() const;

    // Debug tool: teleport the player to a world-space position (logical
    // pixels), zero its velocity and resync the player WorldObject. Not part of
    // the original game; used by the front-end's right-drag "Debug Tools" mode.
    void debugPlacePlayer(float x, float y);

    // Adds a live object from a core spawn event. The type name and image are
    // resolved from the same TypeImageMap used for scene spawns.
    void spawnProjectile(const SpawnedObjectEvent& event);
    // Sends a 0xf/0x20-style event to the already-spawned object it addresses.
    void routeProjectileCue(const SpawnedObjectEvent& event);

private:
    static constexpr u32 kNoObject = 0xFFFFFFFFu;
    void stepProjectiles(float dt);

    std::vector<WorldObject> objects_;
    std::vector<EnemyController> enemies_;
    CollisionMap collision_;
    PlayerController controller_;
    Camera camera_;
    bool hasScene_ = false;
    u32 sceneKey_ = 0;
    u32 playerIndex_ = kNoObject;
    float time_ = 0.0f;
    float worldWidth_ = 256.0f;
    float worldHeight_ = 224.0f;
    u32 nextId_ = 1;
    TypeImageMap typeImages_;
    // Reconstructed collision: the room's per-pixel types (0x5b4970) and the
    // player's per-frame collision surfaces, plus MegaMan's game object.
    const PackedArchive* pak_ = nullptr;
    const PackedGameObject* megaman_ = nullptr;
    TypeMap levelTypes_;
    CollisionMaskCache masks_;
    float gravity_ = kDefaultGravity;
};

} // namespace mmu
