#pragma once

#include "mmu/core/Types.h"

#include <string_view>

namespace mmu {

struct CollisionMap;

// The canonical state names are stored as string literals next to
// "Units\MegaMan\megaman.xgun" in MMU.exe (VA 0x67d544). They are also the
// names passed to the game's state setter FUN_004405c0 / animation selector.
enum class PlayerState : u8 {
    Stand,
    Step,
    Run,
    Fall,
    Hurt,
    HurtKnockback,
    HurtSlide,
    Ride,
    Slide,
    Ladder,
    LadderOut,
    Transition,
    Death,
    Fly,
    Wait,
    Spawn,
    Glued,
    Stun,
    CometDash,
    WeaponAbsorb,
    Carried,
    Teleport,
    ZeroShoryuken,
    ZeroMessenko,
    ZeroMessenkoDrop,
    ZeroSaberSlash,
    ZeroPunch,
    Land,
};

const char* playerStateName(PlayerState state);
bool parsePlayerState(std::string_view name, PlayerState& out);

// Message IDs from MegaMan's virtual message handler (vftable slot 3,
// VA 0x5b02f0). The names below are inferred from each switch arm's effects.
enum class PlayerEvent : u8 {
    Move = 0x00,          // direction/dash input
    Death = 0x01,         // kill player
    Transition = 0x05,    // room transition, value = direction
    MoveBy = 0x07,        // add a world-space offset
    Damage = 0x08,        // take damage, value = attacker x
    Reset = 0x09,         // cancel temporary states
    Heal = 0x0d,          // add health, value = amount
    AddWeaponEnergy = 0x0e,
    Fly = 0x0f,
    Glued = 0x11,         // value = glue timer/source
    Unglued = 0x13,
    Stun = 0x14,
    Dash = 0x1a,
    Input = 0x21,         // per-frame input state
    Unpause = 0x22,
    CarriedEnd = 0x23,
    Teleport = 0x24,
    GravityChange = 0x26,
};

struct PlayerInput {
    float moveX = 0.0f;
    float moveY = 0.0f;
    bool jump = false;
    // Retained for the portable harness. Raw "Shoot" rows exist for other
    // classes, but no exact MegaMan Shoot edge has been recovered here.
    bool shoot = false;
    bool dash = false;
};

// Platform-neutral player controller. State names, event IDs, and transition
// guards mirror the recovered MegaMan virtuals. Numeric physics constants are
// explicitly unverified placeholders: the 0x43c010 hash function and many key
// names are resolved, but no decoded MegaMan value has been tied to these
// exact constants, so they are retained only to keep the harness playable.
struct PlayerController {
    PlayerState state = PlayerState::Spawn;
    PlayerState previous = PlayerState::Stand;
    float stateTime = 0.0f;

    float x = 0.0f;
    float y = 0.0f;
    float floorY = 208.0f; // spawn Y is the platform the player starts on
    float vx = 0.0f;
    float vy = 0.0f;
    bool facingRight = true;
    bool grounded = false;
    bool onLadder = false;
    bool glued = false;
    bool stunned = false;
    bool invincible = false;
    bool dead = false;

    float health = 100.0f;
    float weaponEnergy = 0.0f;
    int weapon = 0;
    float invincibleTime = 0.0f;
    float glueTime = 0.0f;

    // Set by World after a scene load. When null the controller falls back to
    // the spawn-derived floorY approximation.
    const CollisionMap* collision = nullptr;

    void reset(float spawnX, float spawnY, bool faceRight = true);
    void setState(PlayerState next);
    void handleEvent(PlayerEvent event, float value = 0.0f);
    void update(float dt, const PlayerInput& input);

    // States that suppress normal movement input (from switch arms 0x11, 0x14,
    // 0x21 and update guards in VA 0x5b02f0 / 0x5aa470 / 0x5b15d0).
    bool canControl() const;

private:
    bool bodyHits(float px, float py) const;
    void moveWithCollision(float dt);
};

} // namespace mmu
