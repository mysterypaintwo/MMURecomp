#include "mmu/core/World.h"

#include <algorithm>
#include <cmath>

namespace mmu {
namespace {

// Gun objects decoded from Assets/guns.mmu:
//   0xb4cb8fba WilyMachineShot  speed 0x57469074=250, damage 0xc167f1a0=4
//   0xec182c21 WilyMachineBlock speed 0x57469074=230, damage 0xc167f1a0=4
constexpr float kWilyShotSpeed = 250.0f;
constexpr float kWilyBlockSpeed = 230.0f;
constexpr float kWilyBlockDamage = 4.0f;
constexpr float kWilyPillarShootTimer = 0.35f;   // 0xde1a9da5
constexpr float kWilyPillarBlockInterval = 0.6f; // 0x2e46d6f0
constexpr float kWilyPillarDropSpeed = 80.0f;    // 0x643fe84d
constexpr float kWilyPillarLiftSpeed = 50.0f;    // 0x0166019a
constexpr float kWilyPillarDropDuration = 2.0f;  // 0xc4cfff3e
constexpr float kWilyPillarLiftDuration = 1.0f;  // harness-visible lift window
constexpr float kWilyBlockFlyTime = 1.0f;
constexpr float kWilyBlockWaitTime = 0.5f;
constexpr float kWilyBlockReturnTime = 1.0f;
constexpr float kWilyBlockLifetime = 4.0f;

} // namespace

std::string toLowerAscii(std::string_view text) {
    std::string out(text);
    for (char& c : out)
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    return out;
}

TypeImageMap resolveTypeImages(const PackedArchive& pak) {
    TypeImageMap images;
    if (!pak.valid()) return images;

    std::unordered_map<std::string, int> scores;
    for (const PackedSpawn& spawn : pak.spawns())
        if (!spawn.type.empty()) scores.emplace(toLowerAscii(spawn.type), -1);
    // These are spawned at runtime, so they may never appear in levels.mmu.
    // Seed their exact type names so their art resolves from the image paths.
    for (const char* type : {"wilymachineshot", "wilymachinelightning",
                             "wilymachinepillar", "wilymachineblock",
                             "enemyshot", "enemyexplosion"})
        scores.emplace(type, -1);

    for (std::size_t i = 0; i < pak.entries().size(); ++i) {
        const PackedEntry& entry = pak.entries()[i];
        if (entry.type != PackedEntryType::Image || entry.name.empty()) continue;
        const std::string lower = toLowerAscii(entry.name);
        if (lower.find("collision") != std::string::npos) continue;

        std::vector<std::string> parts;
        std::string part;
        for (char c : lower) {
            if (c == '\\' || c == '/' || c == ' ') {
                if (!part.empty()) parts.push_back(part);
                part.clear();
            } else {
                part.push_back(c);
            }
        }
        if (!part.empty()) parts.push_back(part);
        if (parts.empty()) continue;

        std::string base = parts.back();
        const std::size_t baseDot = base.find('.');
        if (baseDot != std::string::npos) base.resize(baseDot);
        const bool art = lower.find("\\art\\") != std::string::npos ||
                         lower.find("/art/") != std::string::npos;

        for (const std::string& candidate : parts) {
            std::string key = candidate;
            const std::size_t dot = key.find('.');
            if (dot != std::string::npos) key.resize(dot);
            if (key.empty()) continue;
            auto it = scores.find(key);
            if (it == scores.end()) continue;
            const int score = (base == key ? 100 : 0) + (art ? 10 : 0);
            if (score > it->second) {
                it->second = score;
                images[key] = static_cast<u32>(i);
            }
        }
    }
    return images;
}

void Camera::follow(float targetX, float targetY, float blend) {
    const float desiredX = targetX - width * 0.5f;
    const float desiredY = targetY - height * 0.5f;
    x += (desiredX - x) * blend;
    y += (desiredY - y) * blend;
}

void Camera::clamp(float worldWidth, float worldHeight) {
    const float maxX = std::max(0.0f, worldWidth - width);
    const float maxY = std::max(0.0f, worldHeight - height);
    x = std::min(std::max(0.0f, x), maxX);
    y = std::min(std::max(0.0f, y), maxY);
}

WorldObject* World::player() {
    return playerIndex_ == kNoObject || playerIndex_ >= objects_.size()
               ? nullptr
               : &objects_[playerIndex_];
}

const WorldObject* World::player() const {
    return playerIndex_ == kNoObject || playerIndex_ >= objects_.size()
               ? nullptr
               : &objects_[playerIndex_];
}

std::size_t World::projectileCount() const {
    std::size_t count = 0;
    for (const WorldObject& object : objects_)
        if (object.active && object.projectile != SpawnedObjectKind::None) ++count;
    return count;
}

void World::spawnProjectile(const SpawnedObjectEvent& event) {
    if (event.kind == SpawnedObjectKind::None) return;

    WorldObject object;
    object.id = nextId_++;
    switch (event.kind) {
    case SpawnedObjectKind::WilyMachineShot:
        object.type = "WilyMachineShot";
        break;
    case SpawnedObjectKind::WilyMachineLightning:
        object.type = "WilyMachineLightning";
        break;
    case SpawnedObjectKind::WilyMachinePillar:
        object.type = "WilyMachinePillar";
        break;
    case SpawnedObjectKind::WilyMachineBlock:
        object.type = "WilyMachineBlock";
        break;
    case SpawnedObjectKind::EnemyShot:
        object.type = "EnemyShot";
        break;
    case SpawnedObjectKind::EnemyExplosion:
        object.type = "EnemyExplosion";
        break;
    default:
        return;
    }
    object.x = event.x;
    object.y = event.y;
    object.vx = event.vx;
    object.vy = event.vy;
    object.projectile = event.kind;
    object.delay = event.delay;
    object.lifetime = event.lifetime;
    object.targetX = event.targetX;
    object.targetY = event.targetY;
    object.damage = event.damage;
    object.sequence = event.sequence;
    object.ownerIndex = event.ownerObjectIndex;
    // WilyMachinePillar starts in Follow. WilyMachineBlock starts in Fly and
    // keeps its horizontal direction in facingRight for the Return phase.
    if (object.projectile == SpawnedObjectKind::WilyMachinePillar) {
        object.projectilePhase = 0;
        object.phaseTime = 0.0f;
        object.spawnedCount = 0;
    } else if (object.projectile == SpawnedObjectKind::WilyMachineBlock) {
        object.projectilePhase = 0;
        object.phaseTime = 0.0f;
        object.spawnedCount = 0;
        object.facingRight = event.targetX >= event.x;
    }
    const auto found = typeImages_.find(toLowerAscii(object.type));
    if (found != typeImages_.end()) object.imageIndex = found->second;
    objects_.push_back(object);
}

void World::routeProjectileCue(const SpawnedObjectEvent& event) {
    for (WorldObject& object : objects_) {
        if (!object.active || object.projectile != event.kind) continue;
        if (event.ownerObjectIndex != kNoObject &&
            object.ownerIndex != event.ownerObjectIndex)
            continue;
        if (event.kind == SpawnedObjectKind::WilyMachinePillar) {
            // WilyMachinePillar's event handler at 0x56bdd0: 0xf = Drop,
            // 0x20 = Follow. Reset the phase timer so the new state gets a
            // full window, and restart the block count when returning to
            // Follow so the next cycle can fire another volley.
            if (event.event == 0x0fu) {
                object.projectilePhase = 1;
                object.phaseTime = 0.0f;
            } else if (event.event == 0x20u) {
                object.projectilePhase = 0;
                object.phaseTime = 0.0f;
                object.spawnedCount = 0;
            }
        }
        return;
    }
}

void World::stepProjectiles(float dt) {
    // Blocks are spawned by the pillar's Shoot phase. Collect them here and
    // append after the loop so a push_back cannot invalidate object references.
    std::vector<SpawnedObjectEvent> pendingBlocks;

    for (WorldObject& object : objects_) {
        if (!object.active || object.projectile == SpawnedObjectKind::None) continue;
        object.projectileAge += dt;
        if (object.delay > 0.0f) {
            object.delay -= dt;
            object.vx = 0.0f;
            object.vy = 0.0f;
            continue;
        }

        switch (object.projectile) {
        case SpawnedObjectKind::WilyMachineShot: {
            // The three shots wait 1/2/3s, then home on the player handle.
            // Gun object 0xb4cb8fba supplies speed 250 and damage 4.
            const float dx = object.targetX - object.x;
            const float dy = object.targetY - object.y;
            const float length = std::sqrt(dx * dx + dy * dy);
            if (length > 0.001f) {
                object.vx = dx / length * kWilyShotSpeed;
                object.vy = dy / length * kWilyShotSpeed;
            }
            break;
        }
        case SpawnedObjectKind::WilyMachineLightning:
            // A vertical strike column follows the targeted x for its short
            // active window; it has no independent travel.
            object.vx = 0.0f;
            object.vy = 0.0f;
            object.x += (object.targetX - object.x) * std::min(1.0f, dt * 20.0f);
            break;
        case SpawnedObjectKind::WilyMachinePillar: {
            // 0x56bdd0 dispatches 0xf to Drop and 0x20 back to Follow. The
            // pillar's own update loop is Follow -> Drop -> Shoot (three
            // blocks, 0.6s apart after a 0.35s initial timer) -> Lift.
            object.phaseTime += dt;
            switch (object.projectilePhase) {
            case 0: // Follow
                object.vy = 0.0f;
                object.x += (object.targetX - object.x) * std::min(1.0f, dt * 6.0f);
                break;
            case 1: // Drop
                object.vy = kWilyPillarDropSpeed;
                if (object.phaseTime >= kWilyPillarDropDuration) {
                    object.projectilePhase = 2;
                    object.phaseTime = 0.0f;
                    object.spawnedCount = 0;
                }
                break;
            case 2: { // Shoot
                object.vy = 0.0f;
                const float nextDelay = object.spawnedCount == 0
                                            ? kWilyPillarShootTimer
                                            : kWilyPillarBlockInterval;
                if (object.spawnedCount < 3 && object.phaseTime >= nextDelay) {
                    static constexpr float offsets[3][2] = {
                        {-10.0f, -10.0f}, {-10.0f, -26.0f}, {-10.0f, -42.0f},
                    };
                    SpawnedObjectEvent block;
                    block.kind = SpawnedObjectKind::WilyMachineBlock;
                    block.x = object.x + offsets[object.spawnedCount][0];
                    block.y = object.y + offsets[object.spawnedCount][1];
                    block.targetX = object.targetX;
                    block.targetY = object.targetY;
                    block.damage = kWilyBlockDamage;
                    block.sequence = object.spawnedCount;
                    block.ownerObjectIndex = object.ownerIndex;
                    pendingBlocks.push_back(block);
                    ++object.spawnedCount;
                    object.phaseTime = 0.0f;
                } else if (object.spawnedCount >= 3 &&
                           object.phaseTime >= kWilyPillarBlockInterval) {
                    object.projectilePhase = 3;
                    object.phaseTime = 0.0f;
                }
                break;
            }
            case 3: // Lift
                object.vy = -kWilyPillarLiftSpeed;
                if (object.phaseTime >= kWilyPillarLiftDuration) {
                    object.projectilePhase = 0;
                    object.phaseTime = 0.0f;
                    object.spawnedCount = 0;
                }
                break;
            default:
                object.projectilePhase = 0;
                break;
            }
            break;
        }
        case SpawnedObjectKind::WilyMachineBlock: {
            // Fly -> Wait -> Return, using facingRight to remember the outward
            // direction. Gun object 0xec182c21 supplies speed 230/damage 4.
            object.phaseTime += dt;
            const float direction = object.facingRight ? 1.0f : -1.0f;
            if (object.projectilePhase == 0) {
                object.vx = direction * kWilyBlockSpeed;
                object.vy = 0.0f;
                if (object.phaseTime >= kWilyBlockFlyTime) {
                    object.projectilePhase = 1;
                    object.phaseTime = 0.0f;
                }
            } else if (object.projectilePhase == 1) {
                object.vx = 0.0f;
                object.vy = 0.0f;
                if (object.phaseTime >= kWilyBlockWaitTime) {
                    object.projectilePhase = 2;
                    object.phaseTime = 0.0f;
                }
            } else {
                object.vx = -direction * kWilyBlockSpeed;
                object.vy = 0.0f;
                if (object.phaseTime >= kWilyBlockReturnTime) object.active = false;
            }
            if (object.projectileAge >= kWilyBlockLifetime) object.active = false;
            break;
        }
        default:
            break;
        }

        if (object.lifetime > 0.0f && object.projectileAge >= object.lifetime)
            object.active = false;
    }

    for (const SpawnedObjectEvent& block : pendingBlocks)
        spawnProjectile(block);
}

void World::clear() {
    objects_.clear();
    enemies_.clear();
    collision_ = CollisionMap{};
    camera_ = Camera{};
    hasScene_ = false;
    sceneKey_ = 0;
    playerIndex_ = kNoObject;
    time_ = 0.0f;
    worldWidth_ = 256.0f;
    worldHeight_ = 224.0f;
    nextId_ = 1;
    controller_.reset(0.0f, 0.0f);
    controller_.collision = nullptr;
}

bool World::loadScene(const PackedScene& scene, const TypeImageMap& typeImages) {
    clear();
    typeImages_ = typeImages;
    hasScene_ = true;
    sceneKey_ = scene.key;
    objects_.reserve(scene.spawnCount);

    float maxX = 255.0f;
    float maxY = 223.0f;
    for (u32 i = 0; i < scene.spawnCount; ++i) {
        const PackedSpawn& spawn = scene.spawns[i];
        WorldObject object;
        object.id = nextId_++;
        object.type = spawn.type;
        object.x = spawn.x;
        object.y = spawn.y;
        object.autoRespawn = spawn.autoRespawn;
        object.facingRight = spawn.facingRight;
        object.respawnOnDeath = spawn.respawnOnDeath;
        object.respawnOnScreen = spawn.respawnOnScreen;
        object.deleteOffScreen = spawn.deleteOffScreen;
        object.collectible = spawn.collectible;
        object.attr1 = spawn.attr1;
        object.attr2 = spawn.attr2;
        const auto found = typeImages.find(toLowerAscii(spawn.type));
        if (found != typeImages.end()) object.imageIndex = found->second;
        objects_.push_back(object);

        maxX = std::max(maxX, spawn.x);
        maxY = std::max(maxY, spawn.y);
    }
    worldWidth_ = maxX + 128.0f;
    worldHeight_ = maxY + 112.0f;

    // Pick a player proxy until the real player state machine is reconstructed:
    // prefer an exact MegaMan/player type token, otherwise use the first object.
    // A lowercase substring would also match the distinct ShadowMegaMan token.
    // An index survives runtime projectile pushes that reallocate objects_.
    for (std::size_t i = 0; i < objects_.size(); ++i) {
        if (objects_[i].type == "MegaMan" || objects_[i].type == "player") {
            playerIndex_ = static_cast<u32>(i);
            break;
        }
    }
    if (playerIndex_ == kNoObject && !objects_.empty()) playerIndex_ = 0;

    // Attach lightweight state machines to enemy spawns. The player proxy is
    // excluded even when levels.mmu does not ship an explicit MegaMan spawn.
    enemies_.reserve(objects_.size());
    for (std::size_t i = 0; i < objects_.size(); ++i) {
        const WorldObject& object = objects_[i];
        if (i == playerIndex_) continue;
        const EnemyKind kind = inferEnemyKind(object.type);
        if (kind == EnemyKind::None) continue;
        EnemyController enemy;
        enemy.reset(kind, object.x, object.y, object.facingRight);
        enemy.objectIndex = static_cast<u32>(i);
        enemies_.push_back(enemy);
    }

    if (WorldObject* player = this->player())
        controller_.reset(player->x, player->y, player->facingRight);
    if (const WorldObject* player = this->player())
        camera_.follow(player->x, player->y, 1.0f);
    camera_.clamp(worldWidth_, worldHeight_);
    return true;
}

bool World::loadScene(const PackedArchive& pak, const PackedScene& scene,
                      const TypeImageMap& typeImages) {
    const bool loaded = loadScene(scene, typeImages);
    collision_ = resolveSceneCollision(pak, scene);
    controller_.collision = collision_.valid() ? &collision_ : nullptr;
    if (WorldObject* player = this->player(); player && collision_.valid()) {
        // A few editor spawns sit inside a wall; lift the player to open space.
        for (int i = 0; i < 64 && collision_.solidAt(controller_.x, controller_.y); ++i)
            controller_.y -= 1.0f;
        player->y = controller_.y;
        camera_.follow(player->x, player->y, 1.0f);
        camera_.clamp(worldWidth_, worldHeight_);
    }
    return loaded;
}

void World::step(float dt, float playerVX, float playerVY) {
    time_ += dt;
    if (WorldObject* player = this->player(); player && player->active) {
        player->vx = playerVX;
        player->vy = playerVY;
        if (playerVX > 0.0f) player->facingRight = true;
        else if (playerVX < 0.0f) player->facingRight = false;
    }
    stepProjectiles(dt);
    for (WorldObject& object : objects_) {
        if (!object.active) continue;
        object.x += object.vx * dt;
        object.y += object.vy * dt;
    }
    if (const WorldObject* player = this->player(); player && player->active)
        camera_.follow(player->x, player->y, std::min(1.0f, dt * 8.0f));
    camera_.clamp(worldWidth_, worldHeight_);
}

void World::step(float dt, const PlayerInput& input) {
    time_ += dt;
    controller_.collision = collision_.valid() ? &collision_ : nullptr;
    controller_.update(dt, input);
    if (WorldObject* player = this->player(); player && player->active) {
        player->x = controller_.x;
        player->y = controller_.y;
        player->vx = controller_.vx;
        player->vy = controller_.vy;
        player->facingRight = controller_.facingRight;
    }
    const float playerX = player() ? player()->x : controller_.x;
    const float playerY = player() ? player()->y : controller_.y;

    // Enemy controllers own the motion of their source WorldObject. Everything
    // else keeps the legacy constant-velocity integration.
    std::vector<char> driven(objects_.size(), 0);
    const CollisionMap* enemyCollision = collision_.valid() ? &collision_ : nullptr;
    for (EnemyController& enemy : enemies_) {
        enemy.update(dt, playerX, playerY, enemyCollision);
        if (enemy.objectIndex >= objects_.size()) continue;
        WorldObject& object = objects_[enemy.objectIndex];
        object.x = enemy.x;
        object.y = enemy.y;
        object.vx = enemy.vx;
        object.vy = enemy.vy;
        object.facingRight = enemy.facingRight;
        driven[enemy.objectIndex] = 1;
    }

    // Instantiate spawn requests after all controllers have stepped. Pushing
    // objects can reallocate objects_, so no raw WorldObject* survives here.
    for (const EnemyController& enemy : enemies_)
        for (const SpawnedObjectEvent& event : enemy.spawnEvents) {
            if (event.event != 0) routeProjectileCue(event);
            else spawnProjectile(event);
        }
    driven.resize(objects_.size(), 0);
    stepProjectiles(dt);

    for (std::size_t i = 0; i < objects_.size(); ++i) {
        WorldObject& object = objects_[i];
        if (!object.active || i == playerIndex_ || driven[i]) continue;
        object.x += object.vx * dt;
        object.y += object.vy * dt;
    }

    // Body-contact damage: enemy boxes are expressed relative to their spawn
    // handle; MegaMan uses the same ~7x12 box as the collision-map resolver.
    WorldObject* player = this->player();
    if (player && player->active && !controller_.dead && !controller_.invincible) {
        const float left = controller_.x - 3.0f;
        const float right = controller_.x + 3.0f;
        const float top = controller_.y - 6.0f;
        const float bottom = controller_.y + 6.0f;
        bool hit = false;
        for (const EnemyController& enemy : enemies_) {
            if (!enemy.contactDamage || enemy.objectIndex >= objects_.size()) continue;
            if (!objects_[enemy.objectIndex].active) continue;
            const float eLeft = enemy.x - enemy.halfWidth;
            const float eRight = enemy.x + enemy.halfWidth;
            const float eTop = enemy.y + enemy.yOffset - enemy.height;
            const float eBottom = enemy.y + enemy.yOffset;
            if (eLeft < right && eRight > left && eTop < bottom && eBottom > top) {
                hit = true;
                controller_.handleEvent(PlayerEvent::Damage, enemy.x);
                break;
            }
        }
        if (!hit) {
            for (const WorldObject& object : objects_) {
                if (!object.active || object.projectile == SpawnedObjectKind::None ||
                    object.delay > 0.0f || object.damage <= 0.0f)
                    continue;
                float halfWidth = 6.0f;
                float height = 12.0f;
                if (object.projectile == SpawnedObjectKind::WilyMachineLightning) {
                    halfWidth = 8.0f;
                    height = 48.0f;
                } else if (object.projectile == SpawnedObjectKind::WilyMachinePillar) {
                    halfWidth = 10.0f;
                    height = 20.0f;
                }
                if (object.x - halfWidth < right && object.x + halfWidth > left &&
                    object.y - height < bottom && object.y > top) {
                    controller_.handleEvent(PlayerEvent::Damage, object.x);
                    break;
                }
            }
        }
    }

    if (const WorldObject* trackedPlayer = this->player();
        trackedPlayer && trackedPlayer->active)
        camera_.follow(trackedPlayer->x, trackedPlayer->y, std::min(1.0f, dt * 8.0f));
    camera_.clamp(worldWidth_, worldHeight_);
}

} // namespace mmu
