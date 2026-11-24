#include "mmu/core/PlayerState.h"

#include "mmu/core/LevelArt.h"

#include <array>
#include <cmath>

namespace mmu {
namespace {

constexpr std::array<const char*, 29> kStateNames = {
    "Stand", "Step", "Run", "Fall", "Hurt", "HurtKnockback", "HurtSlide",
    "Ride", "Slide", "Ladder", "LadderOut", "Transition", "Death", "Fly",
    "Wait", "Spawn", "Glued", "Stun", "CometDash", "WeaponAbsorb",
    "Carried", "Teleport", "ZeroShoryuken", "ZeroMessenko",
    "ZeroMessenkoDrop", "ZeroSaberSlash", "ZeroPunch", "Land",
};

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
}

void PlayerController::setState(PlayerState next) {
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

void PlayerController::update(float dt, const PlayerInput& input) {
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

} // namespace mmu
