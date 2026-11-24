#pragma once

#include "mmu/core/Types.h"

#include <string_view>
#include <vector>

namespace mmu {

struct CollisionMap;

// Runtime-spawned objects whose type names are exact strings from MMU.exe.
// They are emitted by enemy state callbacks and instantiated by World without
// putting SDL or rendering concerns in the core simulation.
enum class SpawnedObjectKind : u8 {
    None,
    WilyMachineShot,
    WilyMachineLightning,
    WilyMachinePillar,
    WilyMachineBlock,
    EnemyShot,        // generic enemy projectile, aimed at the player on spawn
    EnemyExplosion,   // short-lived visual effect from ExplosivePlatform
};

const char* spawnedObjectKindName(SpawnedObjectKind kind);

struct SpawnedObjectEvent {
    SpawnedObjectKind kind = SpawnedObjectKind::None;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float targetX = 0.0f;
    float targetY = 0.0f;
    float delay = 0.0f;
    float lifetime = 0.0f;
    float damage = 0.0f;
    u32 sequence = 0;
    u32 ownerObjectIndex = 0xFFFFFFFFu;
    // Nonzero for state cues sent to an already-spawned object (for example
    // WilyMachinePillar event 0xf = Drop). Zero requests a new object.
    u32 event = 0;
};

// Enemy archetypes implemented by the platform-neutral simulation. Telly,
// EZBlock, MineE, SawPole, SuperCannon, Flasher and Electrode are reconstructed
// from original vftables, constructor state registrations and callback edges.
// Patrol/Platform are generic fallbacks and Generic is a recognised enemy whose
// callbacks are not decoded yet.
enum class EnemyKind : u8 {
    None,      // pickups, scenery, projectiles: no controller
    Telly,     // homing eye: Idle / MoveH / MoveV
    Patrol,    // generic ground patrol with a home range
    Platform,  // generic platform: Wait / Idle
    EZBlock,   // appearing block: Wait / Idle
    MineE,     // proximity mine: Scan / Explode
    SawPole,   // patrolling saw: Patrol
    SuperCannon, // turret: Stand / Turn / Attack
    Flasher,   // 0x692024: Move / Attack, with a FlasherAttack cue on Attack
    Electrode, // 0x691554: Init / Move / Shoot, with an Electrode cue on Shoot
    ExplosivePlatform, // 0x69500c: Idle / Flash, one-shot fuse
    YokuBlock, // 0x6950dc: Wait / Appear / Idle
    YokuSpike, // 0x69514c: same machine as YokuBlock with spike art
    MetStand,  // 0x68e2bc: Stand / Pop / Fire / Hidden
    MetSwim,   // 0x68e35c: Stand / SwimUp / FallDown / Hidden
    MetLaser,  // 0x69101c: Hidden / Pop / Exposed / Hiding
    MetTank,   // 0x6910ac: Stand / MoveBack / Pop / Fire / Charge / Hidden
    CrossShooter, // 0x691e4c: Wait / Shoot turret
    SpikePlatform, // 0x69522c: Wait / Shoot platform turret
    ShieldAttacker, // 0x690c84: Init / Move / Turn
    CannonBall, // 0x6914ac: Up / ShootUp / TurnUp / Down / ShootDown / TurnDown
    // RTTI-verified owners from docs/enemy_state_machines.txt. Their initial
    // states come from generated constructor setState evidence; no transition
    // timing or gameplay meaning is assigned beyond the raw graph edges.
    LuggageMan,     // 0x691cec: Move
    Skoripion,      // 0x6918c4: Walk / Shoot
    ChickenBomb,    // 0x6917dc: Patrol
    BulletTrap,     // 0x69176c: Stand
    SniperJoe,      // 0x691304: Stand / Fire / Jump
    TankJoe,        // 0x69128c: Move / Fire
    GrenadeSoldier, // 0x690e0c: Hidden / Pop
    MineP,          // 0x690d84: Scan / Burst / Flash
    Leprechaun,     // 0x690c0c: Hidden / Pop
    LaserSpreader,  // 0x690b9c: Stand
    MetTrois,       // 0x69115c: Hidden / Popping / Hiding / Popped / Appear / Disappear
    SpreadMine,     // 0x68ef6c: Idle / Explode
    Boss,      // 0x6900cc: Spawn / Pose / attack hook / Hurt / Death
    MidBoss,   // 0x691b94: base only registers MidBossExplosion
    WilyBoss,  // 0x692794: Idle / EnergyFill / WilyBossExplosion
    WilyMachine, // 0x692cc4: Wait / Hover / GotoAttack1..4 / four attacks
    WilyCapsule, // 0x692e94: FlyUp / Wait / FlyDown (position-conditional)
    Generic,   // recognized enemy, behavior not implemented yet
};

// State values 0..4 match the original console command set; new states follow.
enum class EnemyState : u8 {
    Idle,
    MoveH,
    MoveV,
    Wait,
    Death,
    Scan,
    Explode,
    Stand,
    Turn,
    Attack,
    Intro,
    Pose,
    Hurt,
    Patrol,
    Hover,
    PillarDrop,
    Lightning,
    Ram,
    Attack3,
    GotoAttack2,
    Fly,
    FlyDown,
    BigExplosion,
    MidBossExplosion,

    // Appended so the original state values 0..23 remain stable. These come
    // from the recovered vtable triples for Flasher, Electrode and the bosses.
    Move,
    Shoot,
    Init,
    RamReturn,
    Spawn,             // base Boss initial state (0x5a42ca)
    FlyUp,             // WilyCapsule initial state (0x56bbb1)
    GotoAttack1,       // WilyMachine setup: -> PillarDrop (0x56d510)
    GotoAttack3,       // WilyMachine setup: -> Attack3 (0x56d2b0)
    GotoAttack4,       // WilyMachine setup: -> Ram (0x56d240)
    Crush,             // WilyMachine damaged/crush state (0x56d940)
    EnergyFill,        // WilyBoss energy meter (0x575490 triple)
    WilyBossExplosion, // WilyBoss death (0x575780 triple)

    // Appended for enemy archetypes recovered from constructor registrations.
    Up,                // CannonBall initial state
    Down,              // CannonBall downward traversal
    Flash,             // ExplosivePlatform fuse
    Appear,            // YokuBlock
    Hidden,            // shared Met base
    Pop,               // Met family
    Fire,              // MetStand / MetTank
    SwimUp,            // MetSwim
    FallDown,          // MetSwim
    Hiding,            // MetLaser
    Exposed,           // MetLaser
    MoveBack,          // MetTank
    Charge,            // MetTank
    ShootUp,           // CannonBall
    ShootDown,         // CannonBall
    TurnUp,            // CannonBall
    TurnDown,          // CannonBall

    // Raw state names appearing in the thirteen generated enemy state machines.
    Burst,             // MineP
    Jump,              // SniperJoe
    Walk,              // Skoripion
    Glued,             // LuggageMan edge target
    Popping,           // MetTrois
    Popped,            // MetTrois
    Disappear,         // MetTrois
};

const char* enemyKindName(EnemyKind kind);
const char* enemyStateName(EnemyState state);
bool parseEnemyKind(std::string_view name, EnemyKind& out);
bool parseEnemyState(std::string_view name, EnemyState& out);

// Derives an archetype from a GameSpawnObject type string (case-insensitive).
EnemyKind inferEnemyKind(std::string_view type);

// One live enemy in a scene. World owns a parallel array of these controllers
// and copies x/y/facing back into the corresponding WorldObject after each
// fixed step.
struct EnemyController {
    static constexpr u32 kNoObject = 0xFFFFFFFFu;

    EnemyKind kind = EnemyKind::None;
    EnemyState state = EnemyState::Idle;
    EnemyState previous = EnemyState::Idle;
    float stateTime = 0.0f;
    u32 objectIndex = kNoObject;

    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float homeX = 0.0f;
    float homeY = 0.0f;

    // Telly: 0x57469074 -> speed, 0xda1f1441 -> detection radius. The
    // path-derived Telly gun object (0x3f5532f8) stores 25/16; the alternate
    // object 0xaabc064d stores 30/60. The latter is kept as the default until
    // a scene identifies which .xgun variant it spawns.
    float speed = 30.0f;
    float range = 60.0f;
    float fireInterval = 1.2f;

    // Approximate contact box in logical pixels, relative to the spawn handle.
    float halfWidth = 6.0f;
    float height = 14.0f;
    float yOffset = 0.0f;
    bool contactDamage = true;
    bool facingRight = true;

    // Boss health and SuperCannon's projectile counter, exposed to the console.
    float hp = 100.0f;
    u32 shotsFired = 0;
    u32 phase = 0;
    bool activated = false;

    // Last player position passed to update(); used when a state is forced
    // from the debug console between frames.
    float targetX = 0.0f;
    float targetY = 0.0f;

    // Direction chosen on state entry, mirroring the original enter callbacks.
    float moveSign = 1.0f;

    // Spawn requests produced by the current update. World consumes them after
    // stepping all enemies, so a volley sees a stable object vector.
    std::vector<SpawnedObjectEvent> spawnEvents;

    // WilyMachine's persistent pillar is spawned once by the Wait state's
    // update callback (0x56cec0) and stored at WilyMachine+0x36c. The machine
    // later sends events 0xf (Drop) and 0x20 (Follow) to that same object.
    bool spawnedPillar = false;

    void reset(EnemyKind kind, float spawnX, float spawnY, bool faceRight = true);
    void setState(EnemyState next);
    void hurt(float amount);
    void update(float dt, float playerX, float playerY,
                const CollisionMap* collision = nullptr);
};

} // namespace mmu
