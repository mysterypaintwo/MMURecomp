#!/usr/bin/env python3
"""Recover additional GameParam key names with a domain-token dictionary.

`crack_hash.py` proved the key hash is the non-reflected CRC-32 at MMU.exe
0x43c010. Most original key names do not survive as strings in the binary, so
this tool generates candidate names from gameplay vocabulary observed in the
recovered state/cue tables and tests them against every key in guns.mmu and
levels.mmu.

Only exact hash matches are reported; a name is never inferred from a partial
match or from the token that produced it. Results are written to
docs/key_names_discovered.txt for review before being merged into the verified
dictionary in crack_hash.py.

Usage: python3 tools/sweep_key_names.py Assets/ [docs/key_names_discovered.txt]
"""

import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from crack_hash import KNOWN_KEY_NAMES, collect_keys, mmu_crc32

def _step(value, bit):
    """One bit of the recovered LFSR: feedback from bit 31, input at bit 0."""
    top = value & 0x80000000
    value = ((value << 1) | bit) & 0xffffffff
    return value ^ 0x04c11db7 if top else value


def _eight_steps(value, byte):
    for i in range(7, -1, -1):
        value = _step(value, (byte >> i) & 1)
    return value


# The step function is linear over GF(2), so eight steps split into a term from
# the old top byte and a term from the eight input bits. (A standard CRC table
# is NOT equivalent: this CRC shifts input bits in from the low end.)
_REDUCE = [_eight_steps(byte << 24, 0) for byte in range(256)]
_CONTRIB = [_eight_steps(0, byte) for byte in range(256)]


def hash_fast(text):
    value = 0
    for byte in text.encode('ascii'):
        value = (((value << 8) & 0xffffffff) ^ _REDUCE[value >> 24] ^
                 _CONTRIB[byte])
    return value


PREFIXES = [
    'Appear', 'Attack', 'Air', 'Ball', 'Big', 'Blade', 'Block', 'Bomb', 'Bounce',
    'Bubble', 'Bullet', 'Cannon', 'Charge', 'Circle', 'Container', 'Converge',
    'Crush', 'Dash', 'Death', 'Detection', 'Dodge', 'Disappear', 'Drill',
    'Drop', 'Electrode', 'Endurance', 'Escape',
    'Enemy', 'Energy', 'Explode', 'Explosion', 'Expose', 'Fall', 'Fire', 'Form',
    'First', 'Flash', 'Fly', 'FlyUp', 'FlyDown', 'Frame', 'Glue', 'Ground',
    'GroundIdle', 'Hide', 'Hover', 'Hurt', 'Hyper', 'Ice', 'Idle', 'Initial',
    'Invincible', 'Invinsibility', 'Item', 'Jump', 'Ladder', 'Land', 'Laser',
    'Leave', 'Lift',
    'Life', 'Lightning', 'Magnet', 'Max', 'Mega', 'Met', 'Mid', 'Mine', 'Min',
    'Move', 'Moving', 'Number', 'Object', 'Pause', 'Phase', 'Pickup', 'Player',
    'Patrol', 'Pound', 'Pop', 'Power', 'Projectile', 'Pull', 'Push', 'Raise',
    'Ram', 'Reflect', 'Reset', 'Respawn', 'Return', 'Resurrect', 'Rotate', 'Run',
    'Saw', 'Score', 'Second', 'Shield', 'Shoot', 'Shot', 'Stab', 'Stomp',
    'Slide', 'Spawn', 'Spark', 'Spread', 'Spike', 'Stage', 'Stand', 'State',
    'Star', 'Stun', 'Super', 'SuperCannon', 'Swim', 'Swing', 'Target', 'Telly',
    'Third', 'Thunder', 'Time', 'Turn', 'Vanish', 'Velocity', 'Wait', 'Walk',
    'Warning', 'Water', 'Weapon', 'Wily', 'Wind', 'Yoku',
]

SUFFIXES = [
    'Acc', 'Acceleration', 'ActiveDuration', 'Angle', 'AngleDetection',
    'AngleOffset',
    'Center', 'CenterX', 'CenterY', 'Chance', 'Color', 'Count', 'Damage',
    'Delay', 'DelayMax', 'DelayMin', 'Detection', 'Direction', 'Distance',
    'Duration', 'End', 'Frame', 'Frames', 'Frequency', 'Gravity', 'Height',
    'HitBox', 'HotSpotX', 'HotSpotY', 'IdleDelay', 'Interval', 'Length',
    'Life', 'LifeDuration', 'Lifetime', 'Limit', 'Max', 'Min', 'MoveSpeed',
    'MovingDelay', 'Number', 'Offset', 'OffsetX', 'OffsetY', 'Pause',
    'Points', 'Position', 'Probability', 'Radius', 'Range', 'Rate', 'Ratio',
    'RespawnDelay', 'Rotation', 'Scale', 'ShotAngle', 'ShotSpeed', 'Size',
    'Speed', 'SpeedDown', 'SpeedUp', 'Spread', 'Start', 'Step', 'Steps',
    'Threshold', 'Time', 'Timer', 'Velocity', 'Width', 'X', 'Y',
]

WORDS = [
    'Active', 'AirSpeed', 'Angle', 'Attack', 'AttackDelay', 'AttackRatio',
    'Bounce', 'Bullet', 'Charge', 'Collision', 'Count', 'Damage', 'Death',
    'DeathDelay', 'Delay', 'Distance', 'Duration', 'Explode', 'Explosion',
    'FallSpeed', 'Fire', 'Flash', 'Fly', 'Gravity', 'Ground', 'Height',
    'Hidden', 'HitPoints', 'Hover', 'Idle', 'IdleDelay', 'Invincibility',
    'InvinsibilityDuration', 'Jump', 'JumpHeight', 'Laser', 'Life',
    'Lightning', 'Move', 'MoveDistance', 'MoveSpeed', 'Number', 'Offset',
    'Pause', 'Player', 'Position', 'Radius', 'Range', 'Ratio', 'Respawn',
    'Rotate', 'Scale', 'Shoot', 'ShootDelay', 'Shot', 'ShotAngle',
    'ShotSpeed', 'Spawn', 'Speed', 'Spread', 'Time', 'Turn', 'Velocity',
    'Wait', 'WaitDelay', 'Walk', 'Width',
]


def variants(stem):
    """Yield the spelling conventions seen in the already-resolved keys."""
    yield stem
    yield stem.lower()
    yield stem.upper()
    snake = ''.join('_' + c.lower() if c.isupper() and i else c.lower()
                    for i, c in enumerate(stem))
    yield snake
    yield snake.lstrip('_')
    yield stem[0].lower() + stem[1:]


def candidates():
    seen = set()
    for word in WORDS:
        for name in variants(word):
            if name not in seen:
                seen.add(name)
                yield name
    for prefix in PREFIXES:
        for suffix in SUFFIXES:
            for name in variants(prefix + suffix):
                if name not in seen:
                    seen.add(name)
                    yield name
    for prefix in PREFIXES:
        for middle in ('Move', 'Shot', 'Attack', 'Ground', 'Explosion', 'Fly'):
            for suffix in ('Speed', 'Distance', 'Delay', 'Duration', 'Height',
                           'Ratio', 'Angle', 'Offset', 'Count', 'Radius'):
                for name in variants(prefix + middle + suffix):
                    if name not in seen:
                        seen.add(name)
                        yield name


def main():
    asset_dir = sys.argv[1] if len(sys.argv) > 1 else 'Assets'
    out_path = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else
                            'docs/key_names_discovered.txt')
    for name in KNOWN_KEY_NAMES.values():
        assert hash_fast(name) == mmu_crc32(name.encode()), name
    for probe in ('A', 'Wait', 'MoveSpeed', 'TargetHotSpotY', 'anything else!'):
        assert hash_fast(probe) == mmu_crc32(probe.encode()), probe

    keys = collect_keys(asset_dir)
    known = set(KNOWN_KEY_NAMES)
    found = {}
    total = 0
    for name in candidates():
        total += 1
        key = hash_fast(name)
        if key in keys and key not in known and key not in found:
            found[key] = name

    lines = [
        'Generated by tools/sweep_key_names.py (candidate names must hash exactly).',
        'Merge verified additions into KNOWN_KEY_NAMES in tools/crack_hash.py.',
        '',
        f'candidates={total} collected={len(keys)} previously_known={len(known)}',
        f'new_matches={len(found)}',
        '',
    ]
    lines += [f'0x{key:08x} {name}' for key, name in sorted(found.items())]
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text('\n'.join(lines) + '\n')
    print('\n'.join(lines[:6]))
    for key, name in sorted(found.items()):
        print(f'  0x{key:08x} {name}')
    print(f'wrote {out_path}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
