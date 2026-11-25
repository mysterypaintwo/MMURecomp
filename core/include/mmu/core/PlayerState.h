#pragma once

#include "mmu/core/Collision.h"
#include "mmu/core/PackedArchive.h"
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

// Held state of the gameplay actions for one frame. The original input layer
// (0x441820) derives pressed/released edges itself from consecutive frames;
// PlayerController does the same, so callers only report what is held.
// Action ids (0x440880): right 0x15, left 0x16, up 0x17, down 0x18,
// jump 0x1e, slide 0x1f, fire 0x20.
struct PlayerInput {
    float moveX = 0.0f;   // <0 left held, >0 right held
    float moveY = 0.0f;   // <0 up held, >0 down held
    bool jump = false;    // jump held
    bool shoot = false;   // fire held
    bool dash = false;    // slide held
};

// MegaMan's guns.mmu GameParams (object 0xc31e776a,
// "Units/MegaMan/megaman.xgun" with backslashes). Defaults are the shipped values so the controller also
// runs without an archive; loadFrom() replaces them with the packed rows.
struct PlayerParams {
    float moveSpeed = 90.0f;        // 0x9f2d439d move_speed
    float slideSpeed = 150.0f;      // 0x16c2433f slide_speed
    float maxFallSpeed = 500.0f;    // 0x89c83879 max_fall_speed
    float flySpeed = 400.0f;        // 0xbc49de4e Fly_Speed
    float jumpVelocity = -325.0f;   // 0xccf2a219 (0x5a9de0 jump start)
    float jumpCutSpeed = -50.0f;    // 0xab528d4a (Fall: release caps |vy|)
    float airAccel = 8000.0f;       // 0xc5e0085d (Fall: input held)
    float airDecel = 2000.0f;       // 0xc5e50e5d (Fall: no input)
    float inputBufferMs = 50.0f;    // 0xba334ef9 (jump/slide pressed before landing)
    float slideDistance = 55.0f;    // 0x3944b87d (Slide.update)
    float ladderSpeed = 75.0f;      // 0x731a31b3 (Ladder.update)
    float ladderTopProbe = 20.0f;   // 0x863126a0 (Ladder/LadderOut)
    float ladderOutHeight = 14.0f;  // 0x6f213b03 (Ladder -> LadderOut)
    float ladderOutMs = 50.0f;      // 0x87d1161e (LadderOut.update)
    float ladderRegrabMs = 200.0f;  // 0xe1e903e4 (0x5a97d0 cooldown)

    void loadFrom(const PackedArchive& pak, const PackedGameObject& object);
};

constexpr u32 kMegaManObjectKey = 0xc31e776au;
// Global gravity written by every level setup (_DAT_006c5598 = 990.0) and
// the in-water value returned by 0x43c840.
constexpr float kDefaultGravity = 990.0f;
constexpr float kWaterGravity = 375.0f;

// What the controller needs from the world each frame: the room's collision
// types, the camera (tests run in screen space), and the archive for the
// per-animation collision surfaces.
struct PlayerWorldContext {
    const PackedArchive* pak = nullptr;
    const PackedGameObject* object = nullptr;
    const TypeMap* level = nullptr;
    CollisionMaskCache* masks = nullptr;
    float cameraX = 0.0f;
    float cameraY = 0.0f;
    float gravity = kDefaultGravity;
};

// MegaMan, reconstructed from vftable 0x68fccc. Each state has the original
// enter/update/exit callbacks (vslots 18+3n .. 20+3n); see
// docs/player_reconstruction.md for the per-function mapping. Z-Prototype
// (+0x3dc) branches, weapons and ride/carry objects are not reconstructed.
struct PlayerController {
    PlayerState state = PlayerState::Spawn;
    PlayerState previous = PlayerState::Stand;
    float stateTime = 0.0f;

    float x = 0.0f;           // +0x58 handle position (feet)
    float y = 0.0f;           // +0x5c
    float floorY = 208.0f;    // fallback floor when no collision map is attached
    float vx = 0.0f;          // +0x60
    float vy = 0.0f;          // +0x64
    bool facingRight = true;  // +0x68
    bool grounded = false;
    bool onLadder = false;
    bool glued = false;
    bool stunned = false;
    bool invincible = false;  // +0x1bd
    bool dead = false;

    float health = 100.0f;
    float weaponEnergy = 0.0f;
    int weapon = 0;
    float invincibleTime = 0.0f;
    float glueTime = 0.0f;

    // Contact flags gathered by the slot-14 pass (0x43cc00): the OR of every
    // level type under the current collision surface.
    u32 contactFlags = 0;
    // Animation (vslot 1 lookup by CRC-32 of the name, vslot 4 apply).
    u32 animKey = 0;
    const PackedSprite* sprite = nullptr;
    u32 frame = 0;
    float frameTime = 0.0f;
    bool animPaused = false;
    bool animFinished = false;

    PlayerParams params;

    // Legacy approximation used only without a typed collision map.
    const CollisionMap* collision = nullptr;

    void reset(float spawnX, float spawnY, bool faceRight = true);
    void setState(PlayerState next);
    void handleEvent(PlayerEvent event, float value = 0.0f);
    void update(float dt, const PlayerInput& input);
    void update(float dt, const PlayerInput& input, const PlayerWorldContext& world);

    bool canControl() const;

    // Animation by name, as vslot 1. Unknown names keep the current sprite.
    void setAnimation(const char* name);
    const PackedFrame* currentFrame() const;

private:
    struct Buttons {
        bool held = false, pressed = false, released = false;
        void sample(bool now) {
            pressed = now && !held;
            released = !now && held;
            held = now;
        }
    };
    Buttons left_, right_, up_, down_, jump_, slide_, fire_;
    const PlayerWorldContext* world_ = nullptr;
    float clockMs_ = 0.0f;       // stands in for SDL_GetTicks()
    float jumpPressMs_ = -1e9f;  // +0x16c
    float slidePressMs_ = -1e9f; // +0x170
    float ladderExitMs_ = -1e9f; // +0x188
    float ladderOutMs_ = 0.0f;   // +0x184
    float slideStartX_ = 0.0f;   // +0x164
    bool slideReversed_ = false; // +0x1a4
    bool rising_ = false;        // +0x224 started descending this jump
    bool landSoundMuted_ = false; // +0x1a3
    bool ladderOutFromTop_ = false; // +0x19c
    float dt_ = 0.0f;

    void enterState(PlayerState s);
    void updateState(PlayerState s);
    void exitState(PlayerState s);

    float gravity() const;
    float gravitySign() const { return gravity() < 0.0f ? -1.0f : 1.0f; }
    u32 moverMask() const { return collision::Solid | collision::SolidAlt; } // 0x5a7400
    bool climbHeld() const;   // 0x5a83f0: "up" relative to gravity
    bool descendHeld() const; // 0x5a8ae0(0): "down" relative to gravity
    CollisionShape shapeFor(const char* anim) const;
    CollisionShape shape() const;
    bool blockedAt(const CollisionShape& s, float wx, float wy, u32 mask,
                   std::vector<CollisionContact>* contacts = nullptr) const;
    SweepResult sweep(float& velX, float& velY, u32 mask);
    void gatherContacts();
    void advanceAnimation(float dt);

    bool trySlide();        // 0x5aa080
    bool tryJump();         // 0x5a9de0
    bool tryFallOff();      // 0x5a9c40(1)
    bool tryGrabLadder(bool fromGround); // 0x5a97d0
    bool trySlideFallOff(); // 0x5a9b20
    bool slideHasHeadroom() const; // 0x5aa3c0
    void landToGround();    // 0x5a9770

    bool bodyHits(float px, float py) const;
    void legacyUpdate(float dt, const PlayerInput& input);
    void moveWithCollision(float dt);
};

} // namespace mmu
