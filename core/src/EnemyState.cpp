#include "mmu/core/EnemyState.h"

#include "mmu/core/BossDamageParams.h"
#include "mmu/core/EnemyStateGraph.h"
#include "mmu/core/GunParams.h"
#include "mmu/core/LevelArt.h"
#include "mmu/core/WilyAttackSelector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <string>

namespace mmu {
namespace {

// Base Boss Pose 0x5a3690 fills +0x150 by 1 every 0.08s until it reaches 28.
// The depletion branch of FUN_00420380/0x420690 reads GameParam 0xa96bff8d and
// compares it with the 1.0 constant at 0x697ce8; the decoded guns.mmu value is
// 1.0, so equality selects raw return code 4 (docs/boss_damage.txt). The
// subtracted amount comes from FUN_005b4e80/0x5b5160, not from this key.
constexpr float kFullMeter = 28.0f;
static_assert(kBossDamageParamKey == 0xa96bff8du &&
              kBossDamageParamDefault == 1.0f &&
              kBossDamageDepletionCode == 4 &&
              bossDamageDepletionMatches(kBossDamageParamDefault) &&
              !bossDamageDepletionMatches(0.5f));

// WilyMachine projectile constants recovered from the gun objects identified
// by sprite/sound keys. The GameParam names are resolved in docs/key_names.txt.
constexpr float kWilyShotDamage = 4.0f;          // "Damage", object 0xb4cb8fba
constexpr float kWilyLightningLifetime = 1.6f;   // "LightningDuration", object 0x45b4d5d7
constexpr float kWilyLightningDamage = 6.0f;     // "Damage", object 0x45b4d5d7
constexpr float kWilyPillarDamage = 6.0f;        // "Damage", object 0xf47b10d7
constexpr float kWilyLightningFirstDelay = 0.01f;// 0xc0aebcf4
constexpr float kWilyLightningInterval = 2.0f;   // 0xc4cfff3e
constexpr float kWilyLightningExitDelay = 1.0f;  // 0xefdf17dd
constexpr float kWilyAttack3ExitDelay = 0.75f;   // 0x7ffa2ceb
constexpr float kWilyCrushDelay = 1.0f;          // 0xefdf17dd
// The machine stays in PillarDrop until the persistent pillar has dropped,
// fired its three blocks and lifted again; Crush then releases it with 0x20.
constexpr float kWilyPillarCycleDuration = 5.25f;

// Shared ballistic shot used by the newly recovered turret/Met/cannon cycles.
// The original cues (EnemyShot, CrossShooterShot, MetLaserShot) are preserved as
// spawn event kinds once their gun objects are matched to exact parameters.
constexpr float kEnemyShotSpeed = 180.0f;

std::string lower(std::string_view text) {
    std::string out(text);
    for (char& c : out)
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    return out;
}

bool containsAny(const std::string& text, std::initializer_list<const char*> needles) {
    for (const char* needle : needles)
        if (text.find(needle) != std::string::npos) return true;
    return false;
}

constexpr std::array<const char*, 60> kStateNames = {
    "Idle", "MoveH", "MoveV", "Wait", "Death",
    "Scan", "Explode", "Stand", "Turn", "Attack",
    "Intro", "Pose", "Hurt", "Patrol", "Hover",
    "PillarDrop", "Lightning", "Ram", "Attack3", "GotoAttack2",
    "Fly", "FlyDown", "BigExplosion", "MidBossExplosion",
    "Move", "Shoot", "Init", "RamReturn",
    "Spawn", "FlyUp", "GotoAttack1", "GotoAttack3", "GotoAttack4",
    "Crush", "EnergyFill", "WilyBossExplosion",
    "Up", "Down", "Flash", "Appear", "Hidden", "Pop", "Fire", "SwimUp", "FallDown",
    "Hiding", "Exposed", "MoveBack", "Charge", "ShootUp", "ShootDown",
    "TurnUp", "TurnDown",
    "Burst", "Jump", "Walk", "Glued",
    "Popping", "Popped", "Disappear",
};

} // namespace

const char* enemyKindName(EnemyKind kind) {
    switch (kind) {
    case EnemyKind::None: return "None";
    case EnemyKind::Telly: return "Telly";
    case EnemyKind::Patrol: return "Patrol";
    case EnemyKind::Platform: return "Platform";
    case EnemyKind::EZBlock: return "EZBlock";
    case EnemyKind::MineE: return "MineE";
    case EnemyKind::SawPole: return "SawPole";
    case EnemyKind::SuperCannon: return "SuperCannon";
    case EnemyKind::Flasher: return "Flasher";
    case EnemyKind::Electrode: return "Electrode";
    case EnemyKind::ExplosivePlatform: return "ExplosivePlatform";
    case EnemyKind::YokuBlock: return "YokuBlock";
    case EnemyKind::YokuSpike: return "YokuSpike";
    case EnemyKind::MetStand: return "MetStand";
    case EnemyKind::MetSwim: return "MetSwim";
    case EnemyKind::MetLaser: return "MetLaser";
    case EnemyKind::MetTank: return "MetTank";
    case EnemyKind::CrossShooter: return "CrossShooter";
    case EnemyKind::SpikePlatform: return "SpikePlatform";
    case EnemyKind::ShieldAttacker: return "ShieldAttacker";
    case EnemyKind::CannonBall: return "CannonBall";
    case EnemyKind::LuggageMan: return "LuggageMan";
    case EnemyKind::Skoripion: return "Skoripion";
    case EnemyKind::ChickenBomb: return "ChickenBomb";
    case EnemyKind::BulletTrap: return "BulletTrap";
    case EnemyKind::SniperJoe: return "SniperJoe";
    case EnemyKind::TankJoe: return "TankJoe";
    case EnemyKind::GrenadeSoldier: return "GrenadeSoldier";
    case EnemyKind::MineP: return "MineP";
    case EnemyKind::Leprechaun: return "Leprechaun";
    case EnemyKind::LaserSpreader: return "LaserSpreader";
    case EnemyKind::MetTrois: return "MetTrois";
    case EnemyKind::SpreadMine: return "SpreadMine";
    case EnemyKind::Boss: return "Boss";
    case EnemyKind::MidBoss: return "MidBoss";
    case EnemyKind::WilyBoss: return "WilyBoss";
    case EnemyKind::WilyMachine: return "WilyMachine";
    case EnemyKind::WilyCapsule: return "WilyCapsule";
    case EnemyKind::Generic: return "Generic";
    }
    return "?";
}

const char* enemyStateName(EnemyState state) {
    const std::size_t index = static_cast<std::size_t>(state);
    return index < kStateNames.size() ? kStateNames[index] : "?";
}

const char* spawnedObjectKindName(SpawnedObjectKind kind) {
    switch (kind) {
    case SpawnedObjectKind::None: return "None";
    case SpawnedObjectKind::WilyMachineShot: return "WilyMachineShot";
    case SpawnedObjectKind::WilyMachineLightning: return "WilyMachineLightning";
    case SpawnedObjectKind::WilyMachinePillar: return "WilyMachinePillar";
    case SpawnedObjectKind::WilyMachineBlock: return "WilyMachineBlock";
    case SpawnedObjectKind::EnemyShot: return "EnemyShot";
    case SpawnedObjectKind::EnemyExplosion: return "EnemyExplosion";
    }
    return "?";
}

bool parseEnemyKind(std::string_view name, EnemyKind& out) {
    for (int i = 0; i <= static_cast<int>(EnemyKind::Generic); ++i) {
        const EnemyKind kind = static_cast<EnemyKind>(i);
        if (name == enemyKindName(kind)) {
            out = kind;
            return true;
        }
    }
    return false;
}

bool parseEnemyState(std::string_view name, EnemyState& out) {
    for (std::size_t i = 0; i < kStateNames.size(); ++i) {
        if (name == kStateNames[i]) {
            out = static_cast<EnemyState>(i);
            return true;
        }
    }
    return false;
}

EnemyKind inferEnemyKind(std::string_view type) {
    const std::string name = lower(type);
    if (name.empty()) return EnemyKind::None;

    // Pickups and projectiles are represented by spawn records too, but they
    // have no gameplay controller in this reconstruction.
    if (containsAny(name, {
            "lifeenergy", "weaponenergy", "bolt", "etank", "wtank", "extralife",
            "shot", "bullet", "laser", "wave", "cutter", "arrow", "camera target",
            "cameratarget", "explosion", "debris", "spark"})) {
        // LaserChanger/LaserSpreader are level mechanisms, MetLaser is an
        // enemy, and BulletTrap's RTTI name contains the raw word "bullet".
        // None of them are projectiles in the reconstructed controller set.
        if (name.find("laserchanger") == std::string::npos &&
            name.find("laserspreader") == std::string::npos &&
            name.find("metlaser") == std::string::npos &&
            name.find("bullettrap") == std::string::npos)
            return EnemyKind::None;
    }

    if (name.find("telly") != std::string::npos) return EnemyKind::Telly;
    if (name.find("ezblock") != std::string::npos) return EnemyKind::EZBlock;
    if (name.find("minee") != std::string::npos) return EnemyKind::MineE;
    if (name.find("sawpole") != std::string::npos) return EnemyKind::SawPole;
    if (name.find("supercannon") != std::string::npos) return EnemyKind::SuperCannon;
    if (name.find("flasher") != std::string::npos) return EnemyKind::Flasher;
    if (name.find("electrode") != std::string::npos) return EnemyKind::Electrode;

    // Concrete archetypes must precede the broad block/platform/shooter falls
    // below; their type strings include those generic words.
    if (name.find("explosiveplatform") != std::string::npos)
        return EnemyKind::ExplosivePlatform;
    if (name.find("yokuspike") != std::string::npos) return EnemyKind::YokuSpike;
    if (name.find("yokublock") != std::string::npos) return EnemyKind::YokuBlock;
    if (name.find("metstand") != std::string::npos) return EnemyKind::MetStand;
    if (name.find("metswim") != std::string::npos) return EnemyKind::MetSwim;
    if (name.find("metlaser") != std::string::npos) return EnemyKind::MetLaser;
    if (name.find("mettank") != std::string::npos) return EnemyKind::MetTank;
    if (name.find("crossshooter") != std::string::npos) return EnemyKind::CrossShooter;
    if (name.find("spikeplatform") != std::string::npos) return EnemyKind::SpikePlatform;
    if (name.find("shieldattacker") != std::string::npos) return EnemyKind::ShieldAttacker;
    if (name.find("cannonball") != std::string::npos) return EnemyKind::CannonBall;

    // Exact RTTI-name clusters from tools/analyze_enemy_state_machines.py.
    if (name.find("luggageman") != std::string::npos) return EnemyKind::LuggageMan;
    if (name.find("skoripion") != std::string::npos) return EnemyKind::Skoripion;
    if (name.find("chickenbomb") != std::string::npos) return EnemyKind::ChickenBomb;
    if (name.find("bullettrap") != std::string::npos) return EnemyKind::BulletTrap;
    if (name.find("sniperjoe") != std::string::npos) return EnemyKind::SniperJoe;
    if (name.find("tankjoe") != std::string::npos) return EnemyKind::TankJoe;
    if (name.find("grenadesoldier") != std::string::npos)
        return EnemyKind::GrenadeSoldier;
    if (name.find("minep") != std::string::npos) return EnemyKind::MineP;
    if (name.find("leprechaun") != std::string::npos) return EnemyKind::Leprechaun;
    if (name.find("laserspreader") != std::string::npos)
        return EnemyKind::LaserSpreader;
    if (name.find("mettrois") != std::string::npos) return EnemyKind::MetTrois;
    if (name.find("spreadmine") != std::string::npos)
        return EnemyKind::SpreadMine;

    if (containsAny(name, {
            "block", "platform", "wall", "crusher", "forklift", "shutter"})) {
        return EnemyKind::Platform;
    }

    if (containsAny(name, {"toad", "skateboardman"})) {
        return EnemyKind::Patrol;
    }

    if (name.find("wilymachine") != std::string::npos) return EnemyKind::WilyMachine;
    if (name.find("wilycapsule") != std::string::npos) return EnemyKind::WilyCapsule;
    if (name.find("wilyboss") != std::string::npos) return EnemyKind::WilyBoss;
    if (name.find("midboss") != std::string::npos) return EnemyKind::MidBoss;
    if (name.find("boss") != std::string::npos) return EnemyKind::Boss;

    // Everything else in levels.mmu with an enemy-like name gets a generic
    // controller so the debug view can report it without inventing behavior.
    if (containsAny(name, {
            "attacker", "bomb", "cannon", "chicken", "copter", "electrode",
            "flasher", "guardian", "hologram", "jellyfish", "luggageman",
            "mine", "platy", "scorpion", "shooter", "sweeper", "yokocritic",
            "yoyo", "met", "robot", "fish", "marine"})) {
        return EnemyKind::Generic;
    }
    return EnemyKind::None;
}

void EnemyController::reset(EnemyKind kind_, float spawnX, float spawnY, bool faceRight) {
    kind = kind_;
    state = EnemyState::Idle;
    switch (kind) {
    case EnemyKind::Platform:
    case EnemyKind::EZBlock:
        state = EnemyState::Wait;
        break;
    case EnemyKind::MineE:
        state = EnemyState::Scan;
        break;
    case EnemyKind::SawPole:
        state = EnemyState::Patrol;
        break;
    case EnemyKind::SuperCannon:
        state = EnemyState::Stand;
        break;
    case EnemyKind::Flasher:
        state = EnemyState::Move;
        break;
    case EnemyKind::Electrode:
        state = EnemyState::Init;
        break;
    case EnemyKind::ExplosivePlatform:
        state = EnemyState::Idle;
        break;
    case EnemyKind::YokuBlock:
    case EnemyKind::YokuSpike:
    case EnemyKind::CrossShooter:
    case EnemyKind::SpikePlatform:
        state = EnemyState::Wait;
        break;
    case EnemyKind::MetStand:
    case EnemyKind::MetSwim:
    case EnemyKind::MetTank:
        state = EnemyState::Stand;
        break;
    case EnemyKind::MetLaser:
        state = EnemyState::Hidden;
        break;
    case EnemyKind::ShieldAttacker:
        state = EnemyState::Init;
        break;
    case EnemyKind::CannonBall:
        state = EnemyState::Up;
        break;
    case EnemyKind::Boss:
        // Boss::Boss 0x5a42ca sets "Spawn"; there is no Intro state.
        state = EnemyState::Spawn;
        break;
    case EnemyKind::MidBoss:
        // The base MidBoss vtable only registers MidBossExplosion; the generic
        // cycle below stands in for its concrete robot-master subclasses.
        state = EnemyState::Intro;
        break;
    case EnemyKind::WilyBoss:
        // 0x5767a5 sets "Idle" (state_graph.txt previously said "?").
        state = EnemyState::Idle;
        break;
    case EnemyKind::WilyMachine:
        // 0x56dd70 ends with setState at 0x56eb95: the machine starts in Wait.
        state = EnemyState::Wait;
        break;
    case EnemyKind::WilyCapsule:
        // 0x56bbb1 sets "FlyUp"; transitions are position-conditional.
        state = EnemyState::FlyUp;
        break;
    default:
        break;
    }
    // The generated graph is the direct constructor evidence for these RTTI
    // owners; use its initial state instead of any generic fallback above.
    if (const EnemyStateGraphMachine* machine =
            enemyStateGraphMachine(enemyKindName(kind))) {
        EnemyState graphState = state;
        if (parseEnemyState(machine->initialState, graphState))
            state = graphState;
    }
    previous = state;
    stateTime = 0.0f;
    objectIndex = kNoObject;
    spawnEvents.clear();
    spawnedPillar = false;
    x = homeX = spawnX;
    y = homeY = spawnY;
    vx = vy = 0.0f;
    facingRight = faceRight;
    moveSign = faceRight ? 1.0f : -1.0f;
    speed = 30.0f;
    range = 60.0f;
    fireInterval = 1.2f;
    halfWidth = 6.0f;
    height = 14.0f;
    yOffset = 0.0f;
    contactDamage = true;
    hp = 100.0f;
    shotsFired = 0;
    phase = 0;
    activated = false;

    switch (kind) {
    case EnemyKind::Telly: {
        // Identified from Telly::Telly: "MoveSpeed" -> 30, "MoveDistance" -> 60,
        // and "TargetHotSpotX/Y" -> 26/-53. The owning .xgun object
        // (0xaabc064d) is recorded in GunParams.h.
        speed = gunParam(kTelly, kMoveSpeedKey, 30.0f);
        range = gunParam(kTelly, kMoveDistanceKey, 60.0f);
        const float contactWidth = gunParam(kTelly, kTargetHotSpotXKey, 26.0f);
        const float contactHeight = gunParam(kTelly, kTargetHotSpotYKey, -53.0f);
        halfWidth = std::max(0.0f, contactWidth * 0.5f);
        height = std::abs(contactHeight);
        break;
    }
    case EnemyKind::Patrol:
        speed = 30.0f;
        range = 48.0f;
        halfWidth = 8.0f;
        height = 16.0f;
        break;
    case EnemyKind::Platform:
        speed = 0.0f;
        range = 0.0f;
        halfWidth = 0.0f;
        height = 0.0f;
        contactDamage = false;
        break;
    case EnemyKind::EZBlock:
        // Constructor 0x614a20 registers "Wait" then "Idle" and writes 3/7 to
        // the block-style field +0x9c from its state callbacks.
        speed = 0.0f;
        range = 0.0f;
        halfWidth = 10.0f;
        height = 10.0f;
        contactDamage = false;
        break;
    case EnemyKind::MineE:
        // Constructor 0x58f540 registers "Scan" and "Explode" in that order.
        speed = 0.0f;
        range = 64.0f;
        halfWidth = 9.0f;
        height = 9.0f;
        break;
    case EnemyKind::SawPole:
        // Path-derived gun object 0x650b698b stores MoveSpeed 70 and a 28px
        // TargetHotSpotY box.
        speed = gunParam(kSawPole, kMoveSpeedKey, 70.0f);
        range = std::abs(gunParam(kSawPole, kTargetHotSpotYKey, -28.0f));
        halfWidth = 7.0f;
        height = range;
        break;
    case EnemyKind::SuperCannon:
        // Constructor 0x5a45d0 registers "Stand", "Turn", "Attack". Its
        // "ShootDelay" GameParam is 1.8 seconds.
        speed = 0.0f;
        range = 0.0f;
        fireInterval = gunParam(kSuperCannon, kShootDelayKey, 1.8f);
        halfWidth = 16.0f;
        height = 32.0f;
        break;
    case EnemyKind::Flasher:
        // 0x57c5d0 registers Move then Attack; the callback at 0x57c380 plays
        // FlasherAttack and returns to Move. Timing/contact are approximations.
        speed = 40.0f;
        range = 48.0f;
        fireInterval = 1.5f;
        halfWidth = 8.0f;
        height = 16.0f;
        break;
    case EnemyKind::Electrode:
        // 0x586c40 registers Init, Move, Shoot; slot 27 plays Electrode and
        // slot 30 returns to Move. Timing/contact are approximations.
        speed = 55.0f;
        range = 60.0f;
        fireInterval = 1.6f;
        halfWidth = 9.0f;
        height = 18.0f;
        break;
    case EnemyKind::ExplosivePlatform:
        // 0x54f710 accumulates an "ExplosionDelay" fuse (0.8s normal, 0.5s
        // alternate) before calling the explosion helper at 0x54f8a0.
        speed = 0.0f;
        range = 0.0f;
        fireInterval = 0.8f; // "ExplosionDelay" on the identified gun object
        halfWidth = 14.0f;
        height = 8.0f;
        contactDamage = false;
        break;
    case EnemyKind::YokuBlock:
    case EnemyKind::YokuSpike:
        // 0x54ea00 registers Wait / Appear / Idle; "IdleDelay" is read in the
        // base constructor, so the wait period comes from the decoded object.
        speed = 0.0f;
        range = 0.0f;
        fireInterval = 1.2f; // "IdleDelay" on the identified gun object
        halfWidth = 12.0f;
        height = 12.0f;
        contactDamage = false;
        break;
    case EnemyKind::MetStand:
    case EnemyKind::MetSwim:
    case EnemyKind::MetTank:
        // Shared Met base 0x58da40: Stand / Hidden / Charge. Contact damage is
        // toggled by setState while the concrete subclass is Hidden.
        speed = 40.0f;
        range = 64.0f;
        fireInterval = 1.0f;
        halfWidth = 8.0f;
        height = 16.0f;
        break;
    case EnemyKind::MetLaser:
        // MetLaser::MetLaser 0x58cc00 starts in Hidden, so its hitbox is off
        // from the first frame until Pop clears the Met base's Hidden state.
        speed = 40.0f;
        range = 64.0f;
        fireInterval = 1.0f;
        halfWidth = 8.0f;
        height = 16.0f;
        contactDamage = false;
        break;
    case EnemyKind::CrossShooter:
        // 0x57eb60 reads "IdleDelay" (0xb98e43bd) in its constructor.
        speed = 0.0f;
        range = 96.0f;
        fireInterval = 1.2f; // "IdleDelay" at 0x57edce
        halfWidth = 10.0f;
        height = 16.0f;
        break;
    case EnemyKind::SpikePlatform:
        // Same Wait / Shoot machine as CrossShooter with platform contact off.
        speed = 0.0f;
        range = 96.0f;
        fireInterval = 1.2f; // "IdleDelay" at 0x54d653
        halfWidth = 12.0f;
        height = 8.0f;
        contactDamage = false;
        break;
    case EnemyKind::ShieldAttacker:
        // 0x58ff20: Init / Move / Turn; Move's enter emits two EnemyShot cues.
        speed = 36.0f;
        range = 80.0f;
        fireInterval = 1.2f;
        halfWidth = 10.0f;
        height = 20.0f;
        break;
    case EnemyKind::CannonBall:
        // 0x587750 reads "ShotAngleOffset" and "IdleDelay"; its six callbacks
        // form the vertical Up/Down cycle with shots at both firing states.
        speed = 30.0f;
        range = 120.0f;
        fireInterval = 1.25f; // constructor values include 30 and 1.25
        halfWidth = 10.0f;
        height = 18.0f;
        break;
    case EnemyKind::Boss:
        // Base Boss vtable 0x6900cc: Spawn -> Pose, integer HP at +0x150.
        speed = 45.0f;
        range = 120.0f;
        halfWidth = 14.0f;
        height = 28.0f;
        hp = kFullMeter;
        break;
    case EnemyKind::MidBoss:
        // 0x691b94 overrides slots 23..27; 0x5804a0 enters MidBossExplosion.
        speed = 55.0f;
        range = 140.0f;
        halfWidth = 18.0f;
        height = 36.0f;
        hp = kFullMeter;
        break;
    case EnemyKind::WilyBoss:
        // 0x692794 keeps the energy-meter callbacks at 0x575490/0x5752b0.
        speed = 60.0f;
        range = 160.0f;
        halfWidth = 16.0f;
        height = 32.0f;
        hp = kFullMeter;
        break;
    case EnemyKind::WilyMachine:
        // 0x692cc4: 12 states, four setup states and four attacks.
        speed = 30.0f;
        range = 200.0f;
        halfWidth = 24.0f;
        height = 48.0f;
        hp = kFullMeter;
        break;
    case EnemyKind::WilyCapsule:
        // 0x692e94: FlyUp / Wait / FlyDown, position-conditional.
        speed = 90.0f;
        range = 200.0f;
        halfWidth = 14.0f;
        height = 24.0f;
        hp = kFullMeter;
        break;
    case EnemyKind::LuggageMan:
    case EnemyKind::Skoripion:
    case EnemyKind::ChickenBomb:
    case EnemyKind::BulletTrap:
    case EnemyKind::SniperJoe:
    case EnemyKind::TankJoe:
    case EnemyKind::GrenadeSoldier:
    case EnemyKind::MineP:
    case EnemyKind::Leprechaun:
    case EnemyKind::LaserSpreader:
    case EnemyKind::MetTrois:
    case EnemyKind::SpreadMine:
    case EnemyKind::Generic:
    case EnemyKind::None:
        // No timing, movement or firing constants are recovered for these
        // state graphs; the generated initial state is the only behavior used.
        speed = 0.0f;
        range = 0.0f;
        break;
    }
}

void EnemyController::setState(EnemyState next) {
    if (state == next) return;
    previous = state;
    state = next;
    stateTime = 0.0f;
    if (next == EnemyState::MoveH) moveSign = targetX >= x ? 1.0f : -1.0f;
    else if (next == EnemyState::MoveV) moveSign = targetY >= y ? 1.0f : -1.0f;
    else if (next == EnemyState::Patrol) moveSign = facingRight ? 1.0f : -1.0f;

    // SuperCannon's Attack enter callback (0x5a43c0) spawns "SuperCannonShoot"
    // and "SuperCannonBall". Count one volley per entry so it is observable.
    if (next == EnemyState::Attack &&
        (kind == EnemyKind::SuperCannon || kind == EnemyKind::Flasher))
        ++shotsFired;
    if (next == EnemyState::Shoot && kind == EnemyKind::Electrode)
        ++shotsFired;

    const auto emit = [&](SpawnedObjectKind spawnKind, float ox, float oy,
                          float delay, float lifetime, float damage, u32 sequence,
                          u32 cue = 0) {
        SpawnedObjectEvent event;
        event.kind = spawnKind;
        event.x = x + ox;
        event.y = y + oy;
        event.targetX = targetX;
        event.targetY = targetY;
        event.delay = delay;
        event.lifetime = lifetime;
        event.damage = damage;
        event.sequence = sequence;
        event.ownerObjectIndex = objectIndex;
        event.event = cue;
        if (spawnKind == SpawnedObjectKind::EnemyShot) {
            const float dx = targetX - event.x;
            const float dy = targetY - event.y;
            const float length = std::sqrt(dx * dx + dy * dy);
            if (length > 0.001f) {
                event.vx = dx / length * kEnemyShotSpeed;
                event.vy = dy / length * kEnemyShotSpeed;
            }
        }
        spawnEvents.push_back(event);
    };

    // Enter callbacks recovered for the concrete archetypes. Each shooting
    // state emits one aimed EnemyShot; ShieldAttacker's Move enter emits two.
    const bool metShot = (kind == EnemyKind::MetStand ||
                          kind == EnemyKind::MetTank) &&
                         next == EnemyState::Fire;
    const bool metSwimShot = kind == EnemyKind::MetSwim &&
                             next == EnemyState::FallDown;
    const bool metLaserShot = kind == EnemyKind::MetLaser &&
                              next == EnemyState::Pop;
    const bool turretShot = (kind == EnemyKind::CrossShooter ||
                             kind == EnemyKind::SpikePlatform) &&
                            next == EnemyState::Shoot;
    const bool cannonShot = kind == EnemyKind::CannonBall &&
                            (next == EnemyState::ShootUp ||
                             next == EnemyState::ShootDown);
    const bool shieldShots = kind == EnemyKind::ShieldAttacker &&
                             next == EnemyState::Move;
    if (metShot || metSwimShot || metLaserShot || turretShot || cannonShot) {
        emit(SpawnedObjectKind::EnemyShot, 0.0f, -8.0f, 0.0f, 2.0f, 3.0f, 0u);
        ++shotsFired;
    } else if (shieldShots) {
        emit(SpawnedObjectKind::EnemyShot, 0.0f, -6.0f, 0.0f, 2.0f, 3.0f, 0u);
        emit(SpawnedObjectKind::EnemyShot, 0.0f, -12.0f, 0.0f, 2.0f, 3.0f, 1u);
        shotsFired += 2;
    }

    // The shared Met base hides its hitbox while its concrete subclass is in
    // Hidden; 0x58d840 is the enter callback that clears contact damage.
    if (kind == EnemyKind::MetStand || kind == EnemyKind::MetSwim ||
        kind == EnemyKind::MetLaser || kind == EnemyKind::MetTank)
        contactDamage = next != EnemyState::Hidden;

    // PillarDrop's enter callback 0x56ccb0 does not spawn a new object. It
    // looks up the persistent pillar at WilyMachine+0x36c and sends event 0xf
    // (Drop). World routes that cue to the pillar with the same owner index.
    if (kind == EnemyKind::WilyMachine && next == EnemyState::PillarDrop)
        emit(SpawnedObjectKind::WilyMachinePillar, 0.0f, 0.0f, 0.0f, 0.0f,
             kWilyPillarDamage, 0u, 0x0fu);

    // GotoAttack3 enter 0x56cb20 calls 0x56cf70 once, creating three homing
    // WilyMachineShot objects at these offsets with 1/2/3s warning delays.
    // Gun object 0xb4cb8fba supplies speed 250 and damage 4.
    if (kind == EnemyKind::WilyMachine && next == EnemyState::GotoAttack3) {
        constexpr float offsets[3][2] = {{-64.0f, 26.0f}, {16.0f, -64.0f},
                                         {90.0f, 26.0f}};
        for (u32 i = 0; i < 3; ++i)
            emit(SpawnedObjectKind::WilyMachineShot, offsets[i][0], offsets[i][1],
                 static_cast<float>(i + 1), 0.0f, kWilyShotDamage, i);
        shotsFired = 3;
    }

    // The +0x374 counter is reloaded by GotoAttack1/3 and by the Hover attack
    // selector, not by entering Hover itself.
    if (next == EnemyState::Explode) activated = true;
}

void EnemyController::hurt(float amount) {
    if (kind == EnemyKind::None || kind == EnemyKind::Platform ||
        kind == EnemyKind::EZBlock || kind == EnemyKind::ExplosivePlatform ||
        kind == EnemyKind::YokuBlock || kind == EnemyKind::YokuSpike ||
        kind == EnemyKind::SpikePlatform || state == EnemyState::Death)
        return;
    hp = std::max(0.0f, hp - amount);
    if (hp <= 0.0f) setState(EnemyState::Death);
    else if (kind == EnemyKind::Boss || kind == EnemyKind::MidBoss ||
             kind == EnemyKind::WilyBoss || kind == EnemyKind::WilyMachine ||
             kind == EnemyKind::WilyCapsule)
        setState(EnemyState::Hurt);
}

void EnemyController::update(float dt, float playerX, float playerY,
                             const CollisionMap* collision) {
    targetX = playerX;
    targetY = playerY;
    stateTime += dt;
    vx = vy = 0.0f;
    // World consumes these immediately after stepping every enemy. Clearing
    // here keeps direct state forcing and saved snapshots deterministic.
    spawnEvents.clear();

    switch (kind) {
    case EnemyKind::Telly: {
        // Decompiled callbacks 0x586090/0x586310/0x586020/0x5861d0/0x586100:
        // Idle picks an axis toward the player. Once a move has travelled
        // `range` from its spawn point, the update callback re-reads the player
        // and can switch axes or return to Idle. The enter direction is kept
        // while the same state remains active.
        const float dx = std::abs(x - playerX);
        const float dy = std::abs(y - playerY);
        if (state == EnemyState::Idle) {
            if (dx > range) setState(EnemyState::MoveH);
            else if (dy > range) setState(EnemyState::MoveV);
        } else if (state == EnemyState::MoveH) {
            vx = moveSign * speed;
            x += vx * dt;
            facingRight = moveSign > 0.0f;
            if (std::abs(x - homeX) > range) {
                const float newDx = std::abs(x - playerX);
                const float newDy = std::abs(y - playerY);
                if (newDy > range) setState(EnemyState::MoveV);
                else if (newDx > range) setState(EnemyState::MoveH);
                else setState(EnemyState::Idle);
            }
        } else if (state == EnemyState::MoveV) {
            vy = moveSign * speed;
            y += vy * dt;
            if (std::abs(y - homeY) > range) {
                const float newDx = std::abs(x - playerX);
                const float newDy = std::abs(y - playerY);
                if (newDx > range) setState(EnemyState::MoveH);
                else if (newDy > range) setState(EnemyState::MoveV);
                else setState(EnemyState::Idle);
            }
        }
        break;
    }

    case EnemyKind::Patrol:
    case EnemyKind::SawPole: {
        if (state != EnemyState::MoveH && state != EnemyState::Patrol)
            setState(EnemyState::Patrol);
        float nextX = x + moveSign * speed * dt;
        if (std::abs(nextX - homeX) > range) {
            moveSign = -moveSign;
            nextX = homeX + moveSign * range;
        }
        if (collision && collision->valid()) {
            const float probe = nextX + moveSign * halfWidth;
            if (collision->solidAt(probe, y - 1.0f)) {
                moveSign = -moveSign;
                nextX = x + moveSign * speed * dt;
            }
        }
        vx = (nextX - x) / (dt > 0.0f ? dt : 1.0f);
        x = nextX;
        facingRight = moveSign > 0.0f;
        break;
    }

    case EnemyKind::Platform:
    case EnemyKind::EZBlock:
        if (stateTime > 1.0f)
            setState(state == EnemyState::Wait ? EnemyState::Idle : EnemyState::Wait);
        break;

    case EnemyKind::MineE:
        if (state == EnemyState::Scan) {
            const float dx = x - playerX;
            const float dy = y - playerY;
            if (std::sqrt(dx * dx + dy * dy) < range)
                setState(EnemyState::Explode);
        } else if (state == EnemyState::Explode && stateTime > 0.4f) {
            setState(EnemyState::Death);
        }
        break;

    case EnemyKind::SuperCannon:
        if (state == EnemyState::Stand) {
            if (stateTime > fireInterval) {
                const bool wantsRight = playerX >= x;
                if (wantsRight != facingRight) {
                    facingRight = wantsRight;
                    setState(EnemyState::Turn);
                } else {
                    setState(EnemyState::Attack);
                }
            }
        } else if (state == EnemyState::Turn) {
            if (stateTime > 0.3f) setState(EnemyState::Attack);
        } else if (state == EnemyState::Attack && stateTime > 0.8f) {
            setState(EnemyState::Stand);
        }
        break;

    case EnemyKind::Flasher:
        // 0x57c4a0 sets Attack; 0x57c380 plays FlasherAttack and sets Move.
        if (state == EnemyState::Move) {
            vx = (playerX >= x ? 1.0f : -1.0f) * speed;
            x += vx * dt;
            facingRight = vx >= 0.0f;
            if (stateTime > fireInterval) setState(EnemyState::Attack);
        } else if (state == EnemyState::Attack && stateTime > 0.45f) {
            setState(EnemyState::Move);
        }
        break;

    case EnemyKind::Electrode:
        // 0x586b80 sets Move from Init, 0x586a60 sets Shoot, 0x586a20 returns
        // to Move. The Electrode cue is played from slot 29.
        if (state == EnemyState::Init) {
            if (stateTime > 0.5f) setState(EnemyState::Move);
        } else if (state == EnemyState::Move) {
            vx = (playerX >= x ? 1.0f : -1.0f) * speed;
            x += vx * dt;
            facingRight = vx >= 0.0f;
            if (stateTime > fireInterval) setState(EnemyState::Shoot);
        } else if (state == EnemyState::Shoot && stateTime > 0.4f) {
            setState(EnemyState::Move);
        }
        break;

    case EnemyKind::ExplosivePlatform:
        // 0x54f7d0 and timer callback 0x54f710 both select Flash. Flash's
        // update 0x54f8f0 reaches the ExplosionDelay fuse and calls 0x54f8a0.
        if (state == EnemyState::Idle && stateTime >= 0.3f) {
            setState(EnemyState::Flash);
        } else if (state == EnemyState::Flash) {
            const float flashDuration = std::max(0.1f, fireInterval - 0.3f);
            if (stateTime >= flashDuration) {
                SpawnedObjectEvent explosion;
                explosion.kind = SpawnedObjectKind::EnemyExplosion;
                explosion.x = x;
                explosion.y = y;
                explosion.targetX = targetX;
                explosion.targetY = targetY;
                explosion.lifetime = 0.35f;
                explosion.sequence = shotsFired;
                explosion.ownerObjectIndex = objectIndex;
                spawnEvents.push_back(explosion);
                activated = true;
                setState(EnemyState::Death);
            }
        }
        break;

    case EnemyKind::YokuBlock:
    case EnemyKind::YokuSpike:
        // YokuSpike reuses the YokuBlock vftable slots. Message handler
        // 0x54e780 selects Appear/Wait; 0x54e860 cues YokuBlock then Idle.
        if (state == EnemyState::Wait && stateTime >= fireInterval) {
            setState(EnemyState::Appear);
        } else if (state == EnemyState::Appear && stateTime >= 0.35f) {
            setState(EnemyState::Idle);
        } else if (state == EnemyState::Idle && stateTime >= 0.6f) {
            setState(EnemyState::Wait);
        }
        break;

    case EnemyKind::MetStand:
        // 0x5e1750/0x5e1710/0x5e16b0: Pop -> Fire -> Hidden -> Stand.
        if (state == EnemyState::Stand && stateTime >= 1.0f) {
            setState(EnemyState::Pop);
        } else if (state == EnemyState::Pop && stateTime >= 0.3f) {
            setState(EnemyState::Fire);
        } else if (state == EnemyState::Fire && stateTime >= 0.4f) {
            setState(EnemyState::Hidden);
        } else if (state == EnemyState::Hidden && stateTime >= 1.0f) {
            setState(EnemyState::Stand);
        }
        break;

    case EnemyKind::MetSwim:
        // 0x5e1120/0x5e1090/0x5e1000: SwimUp -> FallDown -> Hidden -> Stand.
        if (state == EnemyState::Stand && stateTime >= 0.8f) {
            setState(EnemyState::SwimUp);
        } else if (state == EnemyState::SwimUp && stateTime >= 0.35f) {
            setState(EnemyState::FallDown);
        } else if (state == EnemyState::FallDown && stateTime >= 0.4f) {
            setState(EnemyState::Hidden);
        } else if (state == EnemyState::Hidden && stateTime >= 1.0f) {
            setState(EnemyState::Stand);
        }
        break;

    case EnemyKind::MetLaser:
        // 0x58cb8f/0x58ca7d/0x58cbe5/0x58c965: Pop cues MetLaserShot and
        // Exposed, then Hiding returns to Hidden.
        if (state == EnemyState::Hidden && stateTime >= 1.0f) {
            setState(EnemyState::Pop);
        } else if (state == EnemyState::Pop && stateTime >= 0.3f) {
            setState(EnemyState::Exposed);
        } else if (state == EnemyState::Exposed && stateTime >= 0.45f) {
            setState(EnemyState::Hiding);
        } else if (state == EnemyState::Hiding && stateTime >= 0.6f) {
            setState(EnemyState::Hidden);
        }
        break;

    case EnemyKind::MetTank:
        // 0x58c443/0x58c3c2/0x58c2fd/0x58c2b8/0x58c270: MoveBack -> Pop ->
        // Fire -> Charge -> Hidden -> Stand.
        if (state == EnemyState::Stand && stateTime >= 1.0f) {
            setState(EnemyState::MoveBack);
        } else if (state == EnemyState::MoveBack) {
            vx = -moveSign * speed;
            x += vx * dt;
            if (stateTime >= 0.7f) setState(EnemyState::Pop);
        } else if (state == EnemyState::Pop && stateTime >= 0.3f) {
            setState(EnemyState::Fire);
        } else if (state == EnemyState::Fire && stateTime >= 0.4f) {
            setState(EnemyState::Charge);
        } else if (state == EnemyState::Charge) {
            vx = moveSign * speed;
            x += vx * dt;
            facingRight = moveSign > 0.0f;
            if (stateTime >= 0.5f) setState(EnemyState::Hidden);
        } else if (state == EnemyState::Hidden && stateTime >= 0.8f) {
            setState(EnemyState::Stand);
        }
        break;

    case EnemyKind::CrossShooter:
    case EnemyKind::SpikePlatform:
        // 0x57ead0/0x54d350 select Shoot; the Shoot update cues
        // CrossShooterShot and returns to Wait (0x57eac2/0x54d33c).
        if (state == EnemyState::Wait && stateTime >= fireInterval) {
            setState(EnemyState::Shoot);
        } else if (state == EnemyState::Shoot && stateTime >= 0.35f) {
            setState(EnemyState::Wait);
        }
        break;

    case EnemyKind::ShieldAttacker:
        // 0x58ff0c enters Move, whose enter emits two shots. 0x58fdbc/0x58fe10
        // enter Turn and 0x58fd65 returns to Move after reversing direction.
        if (state == EnemyState::Init && stateTime >= 0.3f) {
            setState(EnemyState::Move);
        } else if (state == EnemyState::Move) {
            vx = moveSign * speed;
            x += vx * dt;
            facingRight = moveSign > 0.0f;
            if (stateTime >= fireInterval) setState(EnemyState::Turn);
        } else if (state == EnemyState::Turn && stateTime >= 0.4f) {
            moveSign = playerX >= x ? 1.0f : -1.0f;
            facingRight = moveSign > 0.0f;
            setState(EnemyState::Move);
        }
        break;

    case EnemyKind::CannonBall: {
        // Six grounded callbacks at 0x587710/0x587600/0x5876d0/0x5875c0/
        // 0x5874b0/0x587580. Enter callbacks 0x587300/0x587150 fire upward
        // and downward; Turn states choose the next vertical direction.
        const bool groundBelow = collision && collision->valid() &&
                                 collision->solidAt(x, y + 2.0f);
        if (state == EnemyState::Up) {
            vy = -speed;
            y += vy * dt;
            if (stateTime >= 0.8f || groundBelow) setState(EnemyState::ShootUp);
        } else if (state == EnemyState::ShootUp && stateTime >= 0.3f) {
            setState(playerY < y ? EnemyState::TurnUp : EnemyState::Down);
        } else if (state == EnemyState::TurnUp && stateTime >= 0.5f) {
            setState(EnemyState::Down);
        } else if (state == EnemyState::Down) {
            vy = speed;
            y += vy * dt;
            if (stateTime >= 0.8f || groundBelow) setState(EnemyState::ShootDown);
        } else if (state == EnemyState::ShootDown && stateTime >= 0.3f) {
            setState(playerY > y ? EnemyState::TurnDown : EnemyState::Up);
        } else if (state == EnemyState::TurnDown && stateTime >= 0.5f) {
            setState(EnemyState::Up);
        }
        break;
    }

    case EnemyKind::Boss:
        if (state == EnemyState::Spawn) {
            // Enter 0x5a3790 writes +0x64 = 400 and an initial handle of
            // player.x + 216 / player.y - 300. Update 0x5a3ad0 descends to
            // player.y + 112 before entering Pose.
            vy = 400.0f;
            y += vy * dt;
            if (y >= playerY + 112.0f) setState(EnemyState::Pose);
        } else if (state == EnemyState::Pose) {
            // 0x5a3690 fills +0x150 from 0 to 28 in 0.08s steps (2.24s).
            if (stateTime >= 2.24f) setState(EnemyState::Attack);
        } else if (state == EnemyState::Attack) {
            // Slot 29 is a pure virtual attack hook overridden by subclasses.
            vx = (playerX >= x ? 1.0f : -1.0f) * speed;
            x += vx * dt;
            facingRight = vx >= 0.0f;
            if (stateTime > 1.5f) setState(EnemyState::Pose);
        } else if (state == EnemyState::Hurt && stateTime > 0.5f) {
            setState(EnemyState::Pose);
        }
        break;

    case EnemyKind::MidBoss:
        if (state == EnemyState::Intro) {
            if (stateTime > 2.0f) setState(EnemyState::Pose);
        } else if (state == EnemyState::Pose) {
            if (stateTime > 1.0f) setState(EnemyState::Attack);
        } else if (state == EnemyState::Attack) {
            vx = (playerX >= x ? 1.0f : -1.0f) * speed;
            x += vx * dt;
            facingRight = vx >= 0.0f;
            if (stateTime > 1.5f) setState(EnemyState::Pose);
        } else if (state == EnemyState::Hurt && stateTime > 0.5f) {
            setState(EnemyState::Pose);
        } else if (state == EnemyState::Death && stateTime > 0.8f) {
            setState(EnemyState::BigExplosion);
        } else if (state == EnemyState::BigExplosion && stateTime > 1.2f) {
            // 0x5804a0 is the MidBossExplosion enter callback; 0x5804d0 plays
            // BigExplosion first, so the order here mirrors the original.
            setState(EnemyState::MidBossExplosion);
        }
        break;

    case EnemyKind::WilyBoss:
        // 0x5767a5 starts WilyBoss in Idle; EnergyFill is the 0x575490 triple.
        if (state == EnemyState::Idle) {
            if (stateTime > 2.0f) setState(EnemyState::EnergyFill);
        } else if (state == EnemyState::EnergyFill) {
            if (stateTime > 2.24f) setState(EnemyState::Attack);
        } else if (state == EnemyState::Pose) {
            if (stateTime > 1.0f) setState(EnemyState::Attack);
        } else if (state == EnemyState::Attack) {
            vx = (playerX >= x ? 1.0f : -1.0f) * speed;
            x += vx * dt;
            facingRight = vx >= 0.0f;
            if (stateTime > 1.5f) setState(EnemyState::Pose);
        } else if (state == EnemyState::Hurt && stateTime > 0.5f) {
            setState(EnemyState::Pose);
        } else if (state == EnemyState::Death && stateTime > 1.0f) {
            setState(EnemyState::WilyBossExplosion);
        }
        break;

    case EnemyKind::WilyMachine: {
        // Selector 0x56d580 is a four-entry jump table over raw this+0x368.
        // tools/analyze_wily_attack_selector.py recovers the literal state
        // names and the raw counter clears on phases 1 and 2; the Hover update
        // calls it when this+0x364 expires. Timings are the resolved GameParam
        // hashes: 2.0s setup, 1.0s post-attack, 0.75s post-volley, 186px ram
        // travel. The portable phase counter stays monotonic for debug output.
        constexpr float kHoverDelay = 2.0f;
        constexpr float kSetupDelay = 2.0f;
        constexpr float kRamSpeed = 186.0f;
        constexpr float kRamTime = 1.0f;
        constexpr float kRamReturnTime = 3.0f; // 186px / 62px/s

        const auto steerToward = [&](float tx, float ty) {
            const float dx = tx - x;
            const float dy = ty - y;
            const float length = std::sqrt(dx * dx + dy * dy);
            if (length <= 0.001f) return;
            vx = dx / length * speed;
            vy = dy / length * speed;
            x += vx * dt;
            y += vy * dt;
            facingRight = vx >= 0.0f;
        };

        if (state == EnemyState::Wait) {
            // Wait's update callback 0x56cec0 is the one-shot pillar spawn. It
            // stores the object at WilyMachine+0x36c for later 0xf/0x20 events.
            if (!spawnedPillar) {
                SpawnedObjectEvent event;
                event.kind = SpawnedObjectKind::WilyMachinePillar;
                event.x = x;
                event.y = y;
                event.targetX = targetX;
                event.targetY = targetY;
                event.damage = kWilyPillarDamage;
                event.ownerObjectIndex = objectIndex;
                spawnEvents.push_back(event);
                spawnedPillar = true;
            }
            if (stateTime >= kHoverDelay) setState(EnemyState::Hover);
        } else if (state == EnemyState::Hover) {
            steerToward(playerX + 128.0f, playerY + 63.0f);
            if (stateTime >= kHoverDelay) {
                const int nextPhase = wilySelectorPhase(phase);
                // Raw phases 1 and 2 clear this+0x374 in their jump-table
                // targets; phases 0 and 3 do not.
                if (wilySelectorClearsShotCounter(nextPhase)) shotsFired = 0;
                switch (nextPhase) {
                case 0: setState(EnemyState::GotoAttack1); break;
                case 1: setState(EnemyState::GotoAttack2); break;
                case 2: setState(EnemyState::GotoAttack3); break;
                default: setState(EnemyState::GotoAttack4); break;
                }
                ++phase;
            }
        } else if (state == EnemyState::GotoAttack1) {
            steerToward(playerX + 90.0f, playerY + 80.0f);
            if (stateTime >= kSetupDelay) setState(EnemyState::PillarDrop);
        } else if (state == EnemyState::GotoAttack2) {
            if (stateTime >= kSetupDelay) setState(EnemyState::Lightning);
        } else if (state == EnemyState::GotoAttack3) {
            if (stateTime >= kSetupDelay) setState(EnemyState::Attack3);
        } else if (state == EnemyState::GotoAttack4) {
            if (stateTime >= kSetupDelay) setState(EnemyState::Ram);
        } else if (state == EnemyState::PillarDrop) {
            // The enter callback 0x56ccb0 sent the Drop cue; this state's own
            // update is empty and the pillar drives the transition into Crush.
            if (stateTime >= kWilyPillarCycleDuration) setState(EnemyState::Crush);
        } else if (state == EnemyState::Crush) {
            // Crush's update 0x56d940 releases the pillar with event 0x20 when
            // the crush timer expires, then returns to Hover via 0x56d660.
            if (stateTime >= kWilyCrushDelay) {
                SpawnedObjectEvent cue;
                cue.kind = SpawnedObjectKind::WilyMachinePillar;
                cue.x = x;
                cue.y = y;
                cue.targetX = targetX;
                cue.targetY = targetY;
                cue.ownerObjectIndex = objectIndex;
                cue.event = 0x20u;
                spawnEvents.push_back(cue);
                setState(EnemyState::Hover);
            }
        } else if (state == EnemyState::Lightning) {
            // Lightning enter 0x56c800 sets 0.01s and clears +0x370; update
            // 0x56d7a0 spawns two bolts 2.0s apart at boss + (56,94), then
            // either revisits GotoAttack2 or calls 0x56d660(1.0s). Bolt object
            // 0x45b4d5d7 lives 1.6s and deals 6 damage.
            if (shotsFired < 2 &&
                stateTime >= (shotsFired == 0 ? kWilyLightningFirstDelay
                                              : kWilyLightningInterval * shotsFired)) {
                ++shotsFired;
                SpawnedObjectEvent event;
                event.kind = SpawnedObjectKind::WilyMachineLightning;
                event.x = x + 56.0f;
                event.y = y + 94.0f;
                event.targetX = targetX;
                event.targetY = targetY;
                event.delay = 0.0f;
                event.lifetime = kWilyLightningLifetime;
                event.damage = kWilyLightningDamage;
                event.sequence = shotsFired;
                event.ownerObjectIndex = objectIndex;
                spawnEvents.push_back(event);
            }
            if (shotsFired >= 2 &&
                stateTime >= kWilyLightningInterval * shotsFired + kWilyLightningExitDelay)
                setState(EnemyState::Hover);
        } else if (state == EnemyState::Attack3) {
            // The volley is emitted on GotoAttack3 enter; 0x7ffa2ceb holds this
            // state for 0.75s while the three shots activate on their delays.
            if (stateTime >= kWilyAttack3ExitDelay) setState(EnemyState::Hover);
        } else if (state == EnemyState::Ram) {
            vx = -kRamSpeed;
            x += vx * dt;
            facingRight = false;
            if (stateTime >= kRamTime) setState(EnemyState::RamReturn);
        } else if (state == EnemyState::RamReturn) {
            vx = kRamSpeed / 3.0f;
            x += vx * dt;
            facingRight = true;
            if (stateTime >= kRamReturnTime) setState(EnemyState::Hover);
        } else if (state == EnemyState::Hurt && stateTime > 0.5f) {
            setState(EnemyState::Hover);
        }
        break;
    }

    case EnemyKind::WilyCapsule:
        // 0x692e94 transitions are position-conditional, not fixed timers:
        // FlyUp until y - player.y < -17, Wait for a 5.0s countdown, FlyDown
        // until y - player.y >= 170, then virtual +0xc(0xf, 0).
        if (state == EnemyState::FlyUp) {
            vy = -30.0f;
            y += vy * dt;
            if (y - playerY < -17.0f) setState(EnemyState::Wait);
        } else if (state == EnemyState::Wait) {
            if (stateTime >= 5.0f) setState(EnemyState::FlyDown);
        } else if (state == EnemyState::FlyDown) {
            vy = 60.0f;
            y += vy * dt;
            if (y - playerY >= 170.0f) setState(EnemyState::FlyUp);
        } else if (state == EnemyState::Hurt && stateTime > 0.5f) {
            setState(EnemyState::FlyUp);
        }
        break;

    case EnemyKind::LuggageMan:
    case EnemyKind::Skoripion:
    case EnemyKind::ChickenBomb:
    case EnemyKind::BulletTrap:
    case EnemyKind::SniperJoe:
    case EnemyKind::TankJoe:
    case EnemyKind::GrenadeSoldier:
    case EnemyKind::MineP:
    case EnemyKind::Leprechaun:
    case EnemyKind::LaserSpreader:
    case EnemyKind::MetTrois:
    case EnemyKind::SpreadMine:
    case EnemyKind::Generic:
    case EnemyKind::None:
        // No update callbacks are decoded for these classes beyond the raw
        // slot edges in EnemyStateGraph.h; do not invent movement or timing.
        break;
    }
}

} // namespace mmu
