#include "mmu/core/PlayerState.h"

#include "mmu/core/LevelArt.h"

#include <array>
#include <cmath>
#include <cstring>
#include <vector>

namespace mmu {
namespace {

constexpr std::array<const char*, 29> kStateNames = {
    "Stand", "Step", "Run", "Fall", "Hurt", "HurtKnockback", "HurtSlide",
    "Ride", "Slide", "Ladder", "LadderOut", "Transition", "Death", "Fly",
    "Wait", "Spawn", "Glued", "Stun", "CometDash", "WeaponAbsorb",
    "Carried", "Teleport", "ZeroShoryuken", "ZeroMessenko",
    "ZeroMessenkoDrop", "ZeroSaberSlash", "ZeroPunch", "Land",
};

// Legacy harness approximation, used only when no typed collision map is
// attached (e.g. a scene without a resolvable collision image).
constexpr float kRunSpeed = 120.0f;
constexpr float kStepSpeed = 55.0f;
constexpr float kLadderSpeed = 70.0f;
constexpr float kGravity = 900.0f;
constexpr float kJumpVelocity = -310.0f;
constexpr float kDashSpeed = 260.0f;
constexpr float kKnockback = 95.0f;

} // namespace

const char* playerStateName(PlayerState state) {
    const std::size_t index = static_cast<std::size_t>(state);
    return index < kStateNames.size() ? kStateNames[index] : "?";
}

bool parsePlayerState(std::string_view name, PlayerState& out) {
    for (std::size_t i = 0; i < kStateNames.size(); ++i) {
        if (name == kStateNames[i]) {
            out = static_cast<PlayerState>(i);
            return true;
        }
    }
    return false;
}

bool PlayerController::canControl() const {
    if (dead || stunned || glued || (invincible && state == PlayerState::Hurt))
        return false;
    switch (state) {
    case PlayerState::Wait:
    case PlayerState::Death:
    case PlayerState::Transition:
    case PlayerState::Spawn:
    case PlayerState::Hurt:
    case PlayerState::HurtKnockback:
    case PlayerState::HurtSlide:
    case PlayerState::Glued:
    case PlayerState::Stun:
    case PlayerState::Carried:
    case PlayerState::Teleport:
    case PlayerState::WeaponAbsorb:
    case PlayerState::ZeroShoryuken:
    case PlayerState::ZeroMessenko:
    case PlayerState::ZeroMessenkoDrop:
    case PlayerState::ZeroSaberSlash:
    case PlayerState::ZeroPunch:
        return false;
    default:
        return true;
    }
}

void PlayerController::reset(float spawnX, float spawnY, bool faceRight) {
    state = PlayerState::Spawn;
    previous = PlayerState::Stand;
    stateTime = 0.0f;
    x = spawnX;
    y = spawnY;
    floorY = spawnY;
    vx = vy = 0.0f;
    facingRight = faceRight;
    grounded = false;
    onLadder = false;
    glued = false;
    stunned = false;
    invincible = false;
    dead = false;
    health = 100.0f;
    weaponEnergy = 0.0f;
    weapon = 0;
    invincibleTime = 0.0f;
    glueTime = 0.0f;
    contactFlags = 0;
    animKey = 0;
    sprite = nullptr;
    frame = 0;
    frameTime = 0.0f;
    animPaused = false;
    animFinished = false;
    left_ = right_ = up_ = down_ = jump_ = slide_ = fire_ = Buttons{};
    clockMs_ = 0.0f;
    jumpPressMs_ = slidePressMs_ = ladderExitMs_ = -1e9f;
    ladderOutMs_ = 0.0f;
    slideReversed_ = rising_ = landSoundMuted_ = ladderOutFromTop_ = false;
}

void PlayerController::setState(PlayerState next) {
    // The reconstructed path runs the original enter/exit callbacks; the
    // legacy path (no world context) only swaps the state value.
    if (world_ && world_->level && world_->level->valid()) {
        exitState(state);
        previous = state;
        state = next;
        stateTime = 0.0f;
        enterState(next);
        return;
    }
    if (state == next) return;
    previous = state;
    state = next;
    stateTime = 0.0f;
}

void PlayerController::handleEvent(PlayerEvent event, float value) {
    switch (event) {
    case PlayerEvent::Move:
    case PlayerEvent::Input:
        // Handled by update() from a PlayerInput; kept as an explicit event so
        // scripted tests can inject input through the same message path.
        break;

    case PlayerEvent::Death:
        dead = true;
        setState(PlayerState::Death);
        break;

    case PlayerEvent::Transition:
        setState(PlayerState::Transition);
        break;

    case PlayerEvent::MoveBy:
        x += value;
        break;

    case PlayerEvent::Damage: {
        // VA 0x5b02f0 case 8: ignore Death/Transition, set invincibility,
        // record attacker x, then push away and enter Hurt/HurtKnockback.
        if (state == PlayerState::Death || state == PlayerState::Transition || invincible)
            break;
        health -= 10.0f;
        invincible = true;
        invincibleTime = 1.2f;
        vx = value >= x ? -kKnockback : kKnockback;
        vy = -70.0f;
        grounded = false;
        setState(PlayerState::Hurt);
        break;
    }

    case PlayerEvent::Reset:
        // VA 0x5b02f0 case 9: clear temporary flags unless in a locked state.
        invincible = false;
        invincibleTime = 0.0f;
        glued = false;
        glueTime = 0.0f;
        stunned = false;
        dead = false;
        if (state != PlayerState::Death && state != PlayerState::Transition &&
            state != PlayerState::Ladder && state != PlayerState::LadderOut &&
            state != PlayerState::HurtKnockback && state != PlayerState::Glued &&
            state != PlayerState::Teleport && state != PlayerState::ZeroShoryuken &&
            state != PlayerState::Wait)
            setState(PlayerState::Fall);
        break;

    case PlayerEvent::Heal:
        health += value;
        if (health > 100.0f) health = 100.0f;
        break;

    case PlayerEvent::AddWeaponEnergy:
        weaponEnergy += value;
        if (weaponEnergy > 100.0f) weaponEnergy = 100.0f;
        break;

    case PlayerEvent::Fly:
        if (state != PlayerState::Death) setState(PlayerState::Fly);
        break;

    case PlayerEvent::Glued:
        glued = true;
        glueTime = value > 0.0f ? value : 2.0f;
        setState(PlayerState::Glued);
        break;

    case PlayerEvent::Unglued:
        glued = false;
        glueTime = 0.0f;
        if (state == PlayerState::Glued) setState(PlayerState::Stand);
        break;

    case PlayerEvent::Stun:
        if (state != PlayerState::Death && state != PlayerState::Fall &&
            state != PlayerState::ZeroShoryuken && state != PlayerState::Hurt) {
            stunned = true;
            setState(PlayerState::Stun);
        }
        break;

    case PlayerEvent::Dash:
        if (canControl()) {
            vx = facingRight ? kDashSpeed : -kDashSpeed;
            setState(PlayerState::CometDash);
        }
        break;

    case PlayerEvent::Unpause:
        if (state == PlayerState::Wait) setState(PlayerState::Stand);
        break;

    case PlayerEvent::CarriedEnd:
        if (state == PlayerState::Carried) setState(PlayerState::Stand);
        break;

    case PlayerEvent::Teleport:
        setState(PlayerState::Teleport);
        break;

    case PlayerEvent::GravityChange:
        vy *= -0.5f;
        break;
    }
}

bool PlayerController::bodyHits(float px, float py) const {
    if (!collision || !collision->valid()) return false;
    // The player box is roughly 7x12 pixels in the original 256x224 space.
    return collision->solidAt(px - 3.0f, py - 6.0f) ||
           collision->solidAt(px + 3.0f, py - 6.0f) ||
           collision->solidAt(px - 3.0f, py + 6.0f) ||
           collision->solidAt(px + 3.0f, py + 6.0f);
}

void PlayerController::moveWithCollision(float dt) {
    if (!collision || !collision->valid()) {
        x += vx * dt;
        y += vy * dt;
        // Without a packed collision image the spawn Y is the platform.
        if (y >= floorY) {
            y = floorY;
            vy = 0.0f;
            grounded = true;
            if (state == PlayerState::Fall) setState(PlayerState::Land);
        } else {
            grounded = false;
        }
        return;
    }

    const float targetX = x + vx * dt;
    if (!bodyHits(targetX, y)) {
        x = targetX;
    } else {
        vx = 0.0f;
    }

    const float targetY = y + vy * dt;
    if (!bodyHits(x, targetY)) {
        y = targetY;
    } else {
        if (vy > 0.0f) grounded = true;
        vy = 0.0f;
    }
    grounded = bodyHits(x, y + 1.0f);
    if (grounded && state == PlayerState::Fall) setState(PlayerState::Land);
}

void PlayerController::legacyUpdate(float dt, const PlayerInput& input) {
    stateTime += dt;
    if (invincibleTime > 0.0f) {
        invincibleTime -= dt;
        if (invincibleTime <= 0.0f) invincible = false;
    }
    if (glueTime > 0.0f) {
        glueTime -= dt;
        if (glueTime <= 0.0f) {
            glued = false;
            if (state == PlayerState::Glued) setState(PlayerState::Stand);
        }
    }
    if (stunned && stateTime > 0.5f) {
        stunned = false;
        setState(grounded ? PlayerState::Stand : PlayerState::Fall);
    }

    // Locked-state timers are explicitly unverified placeholders. The hash
    // function and many parameter names are resolved, but these durations are
    // not tied to exact MegaMan values.
    switch (state) {
    case PlayerState::Spawn:
        if (stateTime > 0.4f) setState(grounded ? PlayerState::Stand : PlayerState::Fall);
        return;
    case PlayerState::Death:
    case PlayerState::Wait:
    case PlayerState::Transition:
    case PlayerState::Teleport:
    case PlayerState::WeaponAbsorb:
    case PlayerState::ZeroShoryuken:
    case PlayerState::ZeroMessenko:
    case PlayerState::ZeroMessenkoDrop:
    case PlayerState::ZeroSaberSlash:
    case PlayerState::ZeroPunch:
        vx = vy = 0.0f;
        return;
    case PlayerState::Hurt:
    case PlayerState::HurtKnockback:
    case PlayerState::HurtSlide:
        vy += kGravity * dt;
        moveWithCollision(dt);
        if (stateTime > (state == PlayerState::HurtSlide ? 0.6f : 0.35f))
            setState(grounded ? PlayerState::Stand : PlayerState::Fall);
        return;
    case PlayerState::Glued:
    case PlayerState::Stun:
    case PlayerState::Carried:
        vx = vy = 0.0f;
        return;
    case PlayerState::Fly:
        break;
    default:
        break;
    }

    if (!canControl()) return;

    if (input.dash && state != PlayerState::CometDash) {
        vx = facingRight ? kDashSpeed : -kDashSpeed;
        setState(PlayerState::CometDash);
    }
    if (state == PlayerState::CometDash) {
        if (stateTime > 0.25f) setState(grounded ? PlayerState::Stand : PlayerState::Fall);
        moveWithCollision(dt);
        return;
    }

    if (input.moveX > 0.05f) facingRight = true;
    else if (input.moveX < -0.05f) facingRight = false;

    if (onLadder || std::abs(input.moveY) > 0.1f) {
        onLadder = true;
        vy = input.moveY * kLadderSpeed;
        vx = input.moveX * kStepSpeed;
        if (std::abs(vy) > 0.1f) setState(PlayerState::Ladder);
        else setState(PlayerState::Stand);
    } else {
        onLadder = false;
        vx = input.moveX * (std::abs(input.moveX) > 0.6f ? kRunSpeed : kStepSpeed);
        if (grounded && input.jump) {
            vy = kJumpVelocity;
            grounded = false;
            setState(PlayerState::Fall);
        } else {
            vy += kGravity * dt;
            if (!grounded || vy < -0.1f) {
                setState(PlayerState::Fall);
            } else if (std::abs(vx) > kRunSpeed * 0.8f) {
                setState(PlayerState::Run);
            } else if (std::abs(vx) > 1.0f) {
                setState(PlayerState::Step);
            } else {
                setState(PlayerState::Stand);
            }
        }
    }

    moveWithCollision(dt);
    if (state == PlayerState::Land && stateTime > 0.1f)
        setState(std::abs(vx) > 1.0f ? PlayerState::Run : PlayerState::Stand);

    // Raw evidence records "Slide" (hash 0x19bde9bb) and a MegaMan slot-37
    // setter at 0x5a8e30, but no exact trigger or timing is decoded here. The
    // CometDash branch above returns before this hook, so this line remains an
    // unverified placeholder rather than a claimed reconstruction of Slide.
    if (grounded && input.dash && state == PlayerState::Stand)
        setState(PlayerState::Slide);
}


// ---------------------------------------------------------------------------
// Reconstructed MegaMan (vftable 0x68fccc). Function VAs are noted per block.
// ---------------------------------------------------------------------------

namespace {

u32 crc(const char* text) {
    // MMU 0x43c010: non-reflected CRC-32, seed 0, no final xor.
    u32 h = 0;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        for (int i = 7; i >= 0; --i) {
            const bool top = (h & 0x80000000u) != 0;
            h = (h << 1) | ((*p >> i) & 1u);
            if (top) h ^= 0x04c11db7u;
        }
    }
    return h;
}

float absf(float v) { return v < 0.0f ? -v : v; }             // 0x5a7590
float signOf(float v) { return v < 0.0f ? -1.0f : 1.0f; }      // 0x5a75c0
float truncf32(float v) { return static_cast<float>(static_cast<i32>(v)); } // 0x5a7550
float clampf(float v, float lo, float hi) {                    // 0x5a7490
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

const char* animationFor(PlayerState state) {
    switch (state) {
    case PlayerState::Stand: return "stand";
    case PlayerState::Step: return "Step";
    case PlayerState::Run: return "run";
    case PlayerState::Fall: return "jump";
    case PlayerState::Hurt:
    case PlayerState::HurtKnockback: return "hurt";
    case PlayerState::HurtSlide: return "HurtSlide";
    case PlayerState::Slide: return "slide";
    case PlayerState::Ladder: return "ladder";
    case PlayerState::LadderOut: return "ladderout";
    case PlayerState::Death: return "Dead";
    case PlayerState::Fly: return "Fly";
    case PlayerState::Spawn: return "Spawn";
    case PlayerState::Glued: return "GlueStuck";
    case PlayerState::Stun: return "stunned";
    case PlayerState::CometDash: return "CometDash";
    case PlayerState::Carried: return "Carried";
    case PlayerState::Teleport: return "TeleporterIn";
    default: return "stand";
    }
}

} // namespace

void PlayerParams::loadFrom(const PackedArchive& pak, const PackedGameObject& object) {
    const auto read = [&](u32 key, float& out) {
        for (u32 i = 0; i < object.paramCount; ++i) {
            const PackedGameParam& p = pak.gameParams()[object.firstParam + i];
            if (p.key == key) { out = p.value; return; }
        }
    };
    read(0x9f2d439du, moveSpeed);
    read(0x16c2433fu, slideSpeed);
    read(0x89c83879u, maxFallSpeed);
    read(0xbc49de4eu, flySpeed);
    read(0xccf2a219u, jumpVelocity);
    read(0xab528d4au, jumpCutSpeed);
    read(0xc5e0085du, airAccel);
    read(0xc5e50e5du, airDecel);
    read(0xba334ef9u, inputBufferMs);
    read(0x3944b87du, slideDistance);
    read(0x731a31b3u, ladderSpeed);
    read(0x863126a0u, ladderTopProbe);
    read(0x6f213b03u, ladderOutHeight);
    read(0x87d1161eu, ladderOutMs);
    read(0xe1e903e4u, ladderRegrabMs);
}

void PlayerController::setAnimation(const char* name) {
    animPaused = false;
    if (!world_ || !world_->pak || !world_->object) return;
    const u32 key = crc(name);
    const PackedSprite* next = world_->pak->animationOf(*world_->object, key);
    if (!next) return;
    if (next == sprite && key == animKey) return;
    animKey = key;
    sprite = next;
    frame = 0;
    frameTime = 0.0f;
    animFinished = false;
}

const PackedFrame* PlayerController::currentFrame() const {
    if (!sprite || sprite->frameCount == 0) return nullptr;
    return &sprite->frames[frame < sprite->frameCount ? frame : 0];
}

void PlayerController::advanceAnimation(float dt) {
    if (!sprite || sprite->frameCount == 0 || animPaused) return;
    frameTime += dt;
    for (int guard = 0; guard < 16; ++guard) {
        const float duration = sprite->frames[frame].duration > 0.0f
                                   ? sprite->frames[frame].duration : 1.0f / 12.0f;
        if (frameTime < duration) break;
        frameTime -= duration;
        if (frame + 1 >= sprite->frameCount) {
            frame = 0;
            animFinished = true; // 0x405360 "animation done"
        } else {
            ++frame;
        }
    }
}

float PlayerController::gravity() const {
    // 0x43c840: water overrides the level's global gravity.
    if (contactFlags & collision::Water) return kWaterGravity;
    return world_ ? world_->gravity : kDefaultGravity;
}

bool PlayerController::climbHeld() const {
    // 0x5a83f0: up when gravity points down (and down when flipped).
    const float g = world_ ? world_->gravity : kDefaultGravity;
    return g <= 0.0f ? down_.held : up_.held;
}

bool PlayerController::descendHeld() const {
    // 0x5a8ae0(1): false while climbing is held, else the "down" button.
    if (climbHeld()) return false;
    const float g = world_ ? world_->gravity : kDefaultGravity;
    return g <= 0.0f ? up_.held : down_.held;
}

CollisionShape PlayerController::shapeFor(const char* anim) const {
    CollisionShape result;
    if (!world_ || !world_->pak || !world_->object || !world_->masks) return result;
    const PackedSprite* s = world_->pak->animationOf(*world_->object, crc(anim));
    if (!s || s->frameCount == 0) return result;
    const PackedFrame& f = s->frames[0];
    if (f.collisionIndex == 0xFFFFFFFFu) return result;
    result.mask = world_->masks->get(*world_->pak, f.collisionIndex, !facingRight);
    if (!result.mask) return result;
    result.handleX = facingRight ? s->xHandle : static_cast<i32>(result.mask->width) - s->xHandle;
    result.handleY = s->yHandle;
    return result;
}

CollisionShape PlayerController::shape() const {
    CollisionShape result;
    const PackedFrame* f = currentFrame();
    if (f && f->collisionIndex != 0xFFFFFFFFu && world_ && world_->masks && world_->pak) {
        result.mask = world_->masks->get(*world_->pak, f->collisionIndex, !facingRight);
        if (result.mask) {
            result.handleX = facingRight ? sprite->xHandle
                                         : static_cast<i32>(result.mask->width) - sprite->xHandle;
            result.handleY = sprite->yHandle;
            return result;
        }
    }
    // Frames without a collision surface reuse the standing body.
    return shapeFor("stand");
}

bool PlayerController::blockedAt(const CollisionShape& s, float wx, float wy, u32 mask,
                                 std::vector<CollisionContact>* contacts) const {
    if (!world_ || !world_->level) return false;
    const CollisionScene scene{world_->level, world_->cameraX, world_->cameraY};
    return testShapeAt(scene, s, wx, wy, mask, contacts) != 0;
}

SweepResult PlayerController::sweep(float& velX, float& velY, u32 mask) {
    const CollisionScene scene{world_->level, world_->cameraX, world_->cameraY};
    SweepResult r = sweepMove(scene, shape(), x, y, velX, velY, dt_, gravity(), mask);
    x = r.x;
    y = r.y;
    return r;
}

void PlayerController::gatherContacts() {
    // 0x43cc00: OR of every level type under the current surface.
    contactFlags = 0;
    if (!world_ || !world_->level) return;
    const CollisionScene scene{world_->level, world_->cameraX, world_->cameraY};
    contactFlags = testShapeAt(scene, shape(), x, y, 0xFFFFFFFFu);
}

// 0x5aa080: slide entry from Stand/Step/Run.
bool PlayerController::trySlide() {
    if (contactFlags & collision::NoWalk) return false;
    // Slide button, or down+jump ("can-use-down-jump-to-slide", default on).
    if (!slide_.pressed && !(jump_.pressed && descendHeld())) return false;
    // The slide surface must fit one frame of slide motion ahead.
    const float step = truncf32(absf(params.slideSpeed) * dt_);
    const CollisionShape s = shapeFor("slide");
    if (blockedAt(s, x + (facingRight ? step : -step), y, moverMask())) return false;
    setState(PlayerState::Slide);
    return true;
}

// 0x5a9de0: jump start.
bool PlayerController::tryJump() {
    if (state == PlayerState::Slide && descendHeld()) return false; // down+jump re-slides
    if (!jump_.pressed) {
        if (contactFlags & collision::Bounce) {
            vy = -60.0f;
            setState(PlayerState::Fall);
        }
        return false;
    }
    vy = params.jumpVelocity * signOf(gravity());
    rising_ = false;
    setState(PlayerState::Fall);
    return true;
}

// 0x5a9c40(1): walked off a ledge.
bool PlayerController::tryFallOff() {
    const CollisionShape s = shape();
    u32 mask = moverMask() | collision::OneWay | collision::Ladder;
    if (blockedAt(s, x, y, collision::OneWayA)) mask &= ~collision::OneWayA;
    if (blockedAt(s, x, y, collision::Ladder)) mask &= ~collision::Ladder;
    if (blockedAt(s, x, y, collision::OneWayB)) mask &= ~collision::OneWayB;
    const float probe = signOf(gravity());
    if (blockedAt(s, x, y + probe, mask)) return false;
    vy = 0.0f;
    if (vx > 0.0f) x = truncf32(x);
    y += probe;
    setState(PlayerState::Fall);
    return true;
}

// 0x5a97d0: grab a ladder (from the ground the "down" direction also works).
bool PlayerController::tryGrabLadder(bool fromGround) {
    if (clockMs_ - ladderExitMs_ < params.ladderRegrabMs) return false;
    const bool wants = fromGround ? (climbHeld() || descendHeld()) : climbHeld();
    if (!wants) return false;
    const float g = world_ ? world_->gravity : kDefaultGravity;
    const float up = g <= 0.0f ? 1.0f : -1.0f;  // 0x5a7460
    const float probeY = climbHeld() ? up : -up; // 0x5a8ab0
    const CollisionShape s = shape();
    std::vector<CollisionContact> contacts;
    if (!blockedAt(s, x, y + probeY, collision::Ladder, &contacts) || contacts.empty())
        return false;
    const float snapX = x + contacts.front().dx;
    if (blockedAt(s, snapX, y + probeY, moverMask())) return false;
    x = snapX;
    ladderOutFromTop_ = true;
    rising_ = true;
    setState(descendHeld() ? PlayerState::LadderOut : PlayerState::Ladder);
    return true;
}

// 0x5a9b20: slid off a ledge (switches to the jump body 12 px lower).
bool PlayerController::trySlideFallOff() {
    const float probe = 12.0f * signOf(gravity());
    const CollisionShape s = shapeFor("jump");
    if (blockedAt(s, x, y + probe, moverMask() | collision::OneWay | collision::Ladder))
        return false;
    vy = 0.0f;
    if (vx > 0.0f) x = truncf32(x);
    y += probe;
    setState(PlayerState::Fall);
    return true;
}

// 0x5aa3c0: may MegaMan stand up here?
bool PlayerController::slideHasHeadroom() const {
    return !blockedAt(shapeFor("stand"), x, y, moverMask());
}

// 0x5a9770
void PlayerController::landToGround() {
    if (!left_.held && !right_.held) setState(PlayerState::Stand);
    else setState(PlayerState::Run);
}

void PlayerController::enterState(PlayerState s) {
    setAnimation(animationFor(s));
    switch (s) {
    case PlayerState::Stand: // 0x5a7a50
        if (contactFlags & collision::Ice) vx = clampf(vx, -params.moveSpeed, params.moveSpeed);
        else vx = 0.0f;
        vy = 0.0f;
        grounded = true;
        onLadder = false;
        break;
    case PlayerState::Step: { // 0x5a8590
        const bool l = left_.held;
        const bool r = right_.held;
        facingRight = r;
        grounded = true;
        if (contactFlags & collision::Ice) break;
        const CollisionShape body = shape();
        if (!l) {
            vx += dt_ * 80.0f;
            if (!blockedAt(body, x + 1.0f, y, collision::Solid)) x += 1.0f;
        } else if (!blockedAt(body, x - 1.0f, y, moverMask())) {
            x -= 1.0f;
        }
        break;
    }
    case PlayerState::Run: // 0x5a7310 (calls Run.update immediately)
        vy = 0.0f;
        grounded = true;
        updateState(PlayerState::Run);
        break;
    case PlayerState::Fall: // 0x5a7eb0
        if (!left_.held && !right_.held) vx = 0.0f;
        landSoundMuted_ = true;
        grounded = false;
        onLadder = false;
        break;
    case PlayerState::Slide: // 0x5ab980
        vx = facingRight ? params.slideSpeed : -params.slideSpeed;
        vy = 0.0f;
        slideStartX_ = x;
        slideReversed_ = false;
        grounded = true;
        break;
    case PlayerState::Ladder: // 0x5a77c0
        vx = 0.0f;
        onLadder = true;
        grounded = false;
        animPaused = true;
        break;
    case PlayerState::LadderOut: // 0x5a7710
        ladderOutMs_ = clockMs_;
        onLadder = true;
        if (ladderOutFromTop_) {
            const float g = world_ ? world_->gravity : kDefaultGravity;
            y += (g <= 0.0f ? -1.0f : 1.0f) * (params.ladderTopProbe / 2.0f);
        }
        break;
    case PlayerState::Hurt: // 0x5acdc0
        invincible = true;
        invincibleTime = 1.5f;
        vx = (facingRight ? 1.0f : -1.0f) * -32.0f;
        break;
    default:
        break;
    }
}

void PlayerController::exitState(PlayerState s) {
    switch (s) {
    case PlayerState::Ladder: // 0x5a7280
        vy = 0.0f;
        ladderExitMs_ = clockMs_;
        onLadder = false;
        break;
    case PlayerState::LadderOut:
        onLadder = false;
        break;
    default:
        break;
    }
}

void PlayerController::updateState(PlayerState s) {
    switch (s) {
    case PlayerState::Stand: { // 0x5afed0
        if (vx != 0.0f && (contactFlags & collision::Ice)) {
            float sy = 0.0f;
            const SweepResult r = sweep(vx, sy, moverMask());
            if (r.completed) {
                const float before = vx;
                vx += dt_ * 80.0f * (vx <= 0.0f ? 1.0f : -1.0f);
                if (signOf(before) != signOf(vx)) vx = 0.0f;
            } else {
                vx = 0.0f;
            }
        } else if (!(contactFlags & collision::Ice)) {
            vx = 0.0f;
        }
        if (left_.held || right_.held) {
            if (!(contactFlags & collision::NoWalk)) {
                facingRight = !left_.held;
                setState(PlayerState::Step);
            } else {
                facingRight = !left_.held;
            }
        }
        if (!trySlide()) {
            if (!tryFallOff() && !tryJump()) tryGrabLadder(true);
        }
        break;
    }
    case PlayerState::Step: // 0x5afce0
        if (contactFlags & collision::NoWalk) { setState(PlayerState::Stand); break; }
        if (animFinished && (left_.held || right_.held)) setState(PlayerState::Run);
        else if (!left_.held && !right_.held) setState(PlayerState::Stand);
        if (state == PlayerState::Step && (contactFlags & collision::Ice))
            setState(PlayerState::Run);
        if (!trySlide()) {
            if (!tryJump() && !tryFallOff()) tryGrabLadder(true);
        }
        break;
    case PlayerState::Run: { // 0x5af930
        if (contactFlags & collision::NoWalk) { setState(PlayerState::Stand); break; }
        const bool l = left_.held;
        const bool r = right_.held;
        if (!l && !r) {
            setState(PlayerState::Stand);
        } else {
            const float dir = l ? -1.0f : 1.0f;
            facingRight = !l;
            if (contactFlags & collision::Ice) vx += dir * dt_ * 80.0f;
            else if (contactFlags & collision::StopRun) vx = 0.0f;
            else vx = dir * params.moveSpeed;
            vx = clampf(vx, -params.moveSpeed, params.moveSpeed);
            float sy = 0.0f;
            sweep(vx, sy, moverMask());
        }
        // 0x5af930 runs the slide/jump/ledge/ladder checks even after it has
        // just handed off to Stand this frame.
        if (!trySlide()) {
            if (!tryJump() && !tryFallOff()) tryGrabLadder(true);
        }
        break;
    }
    case PlayerState::Fall: { // 0x5aea50
        if ((contactFlags & (collision::WindLeft | collision::WindRight)) && vy > 0.0f) {
            vy = 0.0f;
            if (jump_.pressed) y += 1.0f;
            float wx = (contactFlags & collision::WindLeft) ? -50.0f : 50.0f;
            float wy = 0.0f;
            sweep(wx, wy, moverMask());
            if (left_.held) facingRight = false;
            else if (right_.held) facingRight = true;
            break;
        }
        if (jump_.pressed && !descendHeld()) jumpPressMs_ = clockMs_;
        if ((jump_.pressed && descendHeld()) || slide_.held) slidePressMs_ = clockMs_;

        const float g = gravity();
        bool falling = signOf(vy) == signOf(g);
        if (falling) rising_ = true;
        // Releasing jump while still rising caps the upward speed (0xab528d4a).
        if (rising_ && jump_.released && !falling)
            vy = signOf(vy) * std::min(absf(vy), -params.jumpCutSpeed);

        if (!left_.held && !right_.held) {
            if (absf(vx) > 0.0f) {
                float dv = dt_ * params.airDecel * -1.0f * signOf(vx);
                dv = signOf(dv) * std::min(absf(dv), absf(vx));
                vx += dv;
            }
        } else {
            const float dir = left_.held ? -1.0f : 1.0f;
            facingRight = !left_.held;
            float probeY = 0.0f;
            if (world_ && y < world_->cameraY) probeY = (world_->cameraY - y) + 5.0f;
            if (!blockedAt(shape(), x + dir, y + probeY, moverMask()))
                vx += dt_ * params.airAccel * dir;
            else
                vx = 0.0f;
        }
        vx = clampf(vx, -params.moveSpeed, params.moveSpeed);

        vy += g * dt_;
        if (g <= 0.0f) vy = std::max(vy, -params.maxFallSpeed);
        else vy = std::min(vy, params.maxFallSpeed);
        if (contactFlags & collision::Bounce) vy = -60.0f;

        falling = signOf(vy) == signOf(g);
        sweep(vx, vy, moverMask() | collision::OneWay);
        if (signOf(vy) == signOf(g)) rising_ = true;

        if (vy == 0.0f) {
            if (!falling) {
                if (!(contactFlags & collision::Bounce)) setState(PlayerState::Fall);
            } else if (clockMs_ - jumpPressMs_ < params.inputBufferMs) {
                vy = params.jumpVelocity * signOf(g);
                rising_ = false;
                setState(PlayerState::Fall);
            } else if (clockMs_ - slidePressMs_ < params.inputBufferMs) {
                setState(PlayerState::Slide);
            } else {
                landToGround();
            }
            grounded = state != PlayerState::Fall;
        } else {
            landSoundMuted_ = false;
            tryGrabLadder(false);
        }
        break;
    }
    case PlayerState::Slide: { // 0x5ae630
        if (contactFlags & collision::NoWalk) { setState(PlayerState::Stand); break; }
        const bool headroom = slideHasHeadroom();
        if (!headroom) {
            if ((vx > 0.0f && left_.held && !right_.held) ||
                (vx < 0.0f && right_.held && !left_.held)) {
                vx = -vx;
                facingRight = !facingRight;
                slideReversed_ = true;
            }
        } else {
            if (tryJump()) break;
            if (absf(x - slideStartX_) > params.slideDistance || slideReversed_) {
                landToGround();
                break;
            }
            if ((vx > 0.0f && left_.held) || (vx < 0.0f && right_.held)) {
                setState(PlayerState::Stand);
                break;
            }
        }
        if (trySlideFallOff()) break;
        if (vx == 0.0f) vx = facingRight ? params.slideSpeed : -params.slideSpeed;
        float sy = vy;
        const SweepResult r = sweep(vx, sy, moverMask());
        if (!r.completed && headroom) setState(PlayerState::Stand);
        break;
    }
    case PlayerState::Ladder: { // 0x5adfc0
        animPaused = true;
        if (jump_.pressed) {
            vx = left_.held ? -params.moveSpeed : right_.held ? params.moveSpeed : 0.0f;
            vy = 0.0f;
            setState(PlayerState::Fall);
            break;
        }
        const float g = world_ ? world_->gravity : kDefaultGravity;
        const float up = g <= 0.0f ? 1.0f : -1.0f;  // 0x5a7460
        const float down = -up;                     // 0x5a7430
        const CollisionShape body = shape();
        if (climbHeld()) {
            float probe = up * truncf32(params.ladderTopProbe);
            if (!blockedAt(body, x, y + probe, moverMask() | collision::Ladder)) {
                // Ladder ends within reach: find its top and climb out.
                while (!blockedAt(body, x, y + probe, moverMask() | collision::Ladder) &&
                       probe != 0.0f)
                    probe += down;
                y = truncf32(y) + probe + down * params.ladderOutHeight;
                ladderOutFromTop_ = false;
                setState(PlayerState::LadderOut);
                break;
            }
            const float step = up * truncf32(absf(params.ladderSpeed) * dt_);
            if (!blockedAt(body, x, y + step, moverMask())) {
                y += up * dt_ * params.ladderSpeed;
                animPaused = false;
            }
        }
        if (descendHeld()) {
            if (!blockedAt(body, x, y + down, moverMask() | collision::Ladder)) {
                ladderOutFromTop_ = false;
                setState(PlayerState::Fall);
                break;
            }
            float lvx = 0.0f;
            float lvy = down * params.ladderSpeed;
            const SweepResult r = sweep(lvx, lvy, moverMask());
            if (!r.completed) setState(PlayerState::Stand);
            else animPaused = false;
        }
        break;
    }
    case PlayerState::LadderOut: { // 0x5adde0
        if (clockMs_ - ladderOutMs_ > params.ladderOutMs) {
            if (ladderOutFromTop_ || descendHeld()) {
                ladderOutFromTop_ = false;
                setState(PlayerState::Ladder);
                break;
            }
            if (climbHeld()) {
                const float g = world_ ? world_->gravity : kDefaultGravity;
                const float up = g <= 0.0f ? 1.0f : -1.0f;
                y = std::round(y) + up * (params.ladderOutHeight + 1.0f);
                setState(PlayerState::Stand);
                break;
            }
        }
        if (jump_.pressed) {
            vx = left_.held ? -params.moveSpeed : right_.held ? params.moveSpeed : 0.0f;
            setState(PlayerState::Fall);
        }
        break;
    }
    case PlayerState::Hurt: { // 0x5acbf0 (knockback drift under gravity)
        vy += gravity() * dt_;
        vy = std::min(vy, params.maxFallSpeed);
        sweep(vx, vy, moverMask() | collision::OneWay);
        if (stateTime >= 0.5f) {
            if (vy == 0.0f) landToGround();
            else setState(PlayerState::Fall);
        }
        break;
    }
    case PlayerState::Spawn:
        if (stateTime > 0.4f) setState(PlayerState::Fall);
        break;
    default:
        // States whose callbacks are not reconstructed yet hold position.
        break;
    }
}

void PlayerController::update(float dt, const PlayerInput& input) {
    PlayerWorldContext none;
    update(dt, input, none);
}

void PlayerController::update(float dt, const PlayerInput& input,
                              const PlayerWorldContext& world) {
    left_.sample(input.moveX < -0.05f);
    right_.sample(input.moveX > 0.05f);
    up_.sample(input.moveY < -0.1f);
    down_.sample(input.moveY > 0.1f);
    jump_.sample(input.jump);
    slide_.sample(input.dash);
    fire_.sample(input.shoot);
    clockMs_ += dt * 1000.0f;

    if (!world.level || !world.level->valid()) {
        world_ = nullptr;
        PlayerInput legacy = input;
        legacy.jump = jump_.pressed;
        legacy.dash = slide_.pressed;
        legacyUpdate(dt, legacy);
        return;
    }
    world_ = &world;
    dt_ = dt;
    stateTime += dt;
    if (!sprite) setAnimation(animationFor(state));
    if (invincibleTime > 0.0f) {
        invincibleTime -= dt;
        if (invincibleTime <= 0.0f) invincible = false;
    }

    // Slot 11 (0x5aa470) -> state Update (0x43cf60) -> slot 14 (0x5b15d0).
    if (state != PlayerState::Wait) {
        const float startY = y;
        updateState(state);
        (void)startY;
    }
    advanceAnimation(dt);
    gatherContacts();

    if (state != PlayerState::Death) {
        // 0x5aa470: leaving the bottom of the screen (244 px below the camera,
        // or 20 px above it under inverted gravity) kills MegaMan.
        const float rel = y - world.cameraY;
        const bool outOfScreen = gravity() > 0.0f ? rel > 244.0f : rel < -20.0f;
        // 0x5b15d0: instant-death type 0x200000 and spikes (4 / 0x8000000).
        const bool spikes = (contactFlags & (collision::Hazard | collision::Type8000000)) != 0;
        if (outOfScreen || (contactFlags & collision::Type200000) || (spikes && !invincible)) {
            dead = true;
            vx = vy = 0.0f;
            setState(PlayerState::Death);
        }
    }
    world_ = nullptr;
}

} // namespace mmu
