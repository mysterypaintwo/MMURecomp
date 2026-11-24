#!/usr/bin/env python3
"""Identify and resolve the 32-bit key hash used by MMU assets.

Collects every hashed key from guns.mmu/levels.mmu, verifies the recovered
non-reflected CRC-32 at 0x43c010, writes docs/key_names.txt from the verified
dictionary, and tests other candidates as negative controls.

Usage: python3 tools/crack_hash.py MMU.exe Assets/
"""

import os
import pathlib
import struct
import sys
import zlib


def varint(d, p):
    v = s = 0
    while p < len(d):
        b = d[p]; p += 1
        v |= (b & 0x7f) << s
        if b < 0x80:
            return v, p
        s += 7
    raise ValueError


def fields(d):
    p = 0
    while p < len(d):
        k, p = varint(d, p)
        n, w = k >> 3, k & 7
        if w == 0:
            v, p = varint(d, p); yield n, w, v
        elif w == 5:
            yield n, w, struct.unpack_from('<f', d, p)[0]; p += 4
        elif w == 1:
            yield n, w, struct.unpack_from('<Q', d, p)[0]; p += 8
        elif w == 2:
            l, p = varint(d, p); yield n, w, d[p:p + l]; p += l
        else:
            raise ValueError(w)


def collect_keys(asset_dir):
    keys = set()
    path = os.path.join(asset_dir, 'guns.mmu')
    if os.path.exists(path):
        d = open(path, 'rb').read()
        for n, w, v in fields(d):
            if n != 1:
                continue
            for fn, fw, fv in fields(v):
                if fn == 2:
                    for gn, gw, gv in fields(fv):
                        if gn == 1:
                            keys.add(gv & 0xffffffff)
    path = os.path.join(asset_dir, 'levels.mmu')
    if os.path.exists(path):
        d = open(path, 'rb').read()
        for n, w, v in fields(d):
            if n == 2:
                keys.add(v & 0xffffffff)
    return keys


def strings(data, minlen=3, maxlen=64):
    cur = bytearray()
    for b in data:
        if 32 <= b < 127:
            cur.append(b)
        else:
            if minlen <= len(cur) <= maxlen:
                yield bytes(cur)
            cur.clear()


# --- candidate hash functions -------------------------------------------------
def zlib_crc32(b):
    """Reflected CRC-32; retained as a negative control.

    MMU.exe does not use this variant. Keeping it with an explicit name makes
    the earlier false negative easier to understand.
    """
    return zlib.crc32(b) & 0xffffffff


def mmu_crc32(b):
    """The hash at MMU.exe 0x43c010: non-reflected CRC-32, seed/xorout 0.

    This is the same algorithm used for state names, cutscene resources and
    GameParam keys. For strings of four bytes or fewer it degenerates to the
    packed bytes, which is why the state-name tables looked like a special case.
    """
    h = 0
    for c in b:
        for i in range(7, -1, -1):
            top = h & 0x80000000
            h = ((h << 1) | ((c >> i) & 1)) & 0xffffffff
            if top:
                h ^= 0x04c11db7
    return h


# Backwards-compatible alias for callers of the old helper name.
crc32 = zlib_crc32


def fnv1a(b):
    h = 0x811c9dc5
    for x in b:
        h = ((h ^ x) * 0x01000193) & 0xffffffff
    return h


def fnv1(b):
    h = 0x811c9dc5
    for x in b:
        h = (h * 0x01000193) & 0xffffffff
        h ^= x
    return h


def msvc_stdhash(b):
    # MSVC's std::hash<string> (FNV-1 style with different seed/multiplier)
    h = 0x811c9dc5
    for x in b:
        h = (h * 0x01000193) & 0xffffffff
        h ^= x
    return h & 0x7fffffff


def djb2(b):
    h = 5381
    for x in b:
        h = ((h * 33) ^ x) & 0xffffffff
    return h


def sdbm(b):
    h = 0
    for x in b:
        h = (x + (h << 6) + (h << 16) - h) & 0xffffffff
    return h


def murmur2(b, seed=0):
    m = 0x5bd1e995
    r = 24
    h = (seed ^ len(b)) & 0xffffffff
    i = 0
    while len(b) - i >= 4:
        k = struct.unpack_from('<I', b, i)[0]
        k = (k * m) & 0xffffffff
        k ^= k >> r
        k = (k * m) & 0xffffffff
        h = (h * m) & 0xffffffff
        h ^= k
        i += 4
    tail = b[i:]
    if len(tail) == 3:
        h ^= tail[2] << 16
    if len(tail) >= 2:
        h ^= tail[1] << 8
    if len(tail) >= 1:
        h ^= tail[0]
        h = (h * m) & 0xffffffff
    h ^= h >> 13
    h = (h * m) & 0xffffffff
    h ^= h >> 15
    return h


def murmur3(b, seed=0):
    c1, c2 = 0xcc9e2d51, 0x1b873593
    h = seed
    i = 0
    while len(b) - i >= 4:
        k = struct.unpack_from('<I', b, i)[0]
        k = (k * c1) & 0xffffffff
        k = ((k << 15) | (k >> 17)) & 0xffffffff
        k = (k * c2) & 0xffffffff
        h ^= k
        h = ((h << 13) | (h >> 19)) & 0xffffffff
        h = (h * 5 + 0xe6546b64) & 0xffffffff
        i += 4
    tail = b[i:]
    k = 0
    if len(tail) >= 3:
        k ^= tail[2] << 16
    if len(tail) >= 2:
        k ^= tail[1] << 8
    if tail:
        k ^= tail[0]
        k = (k * c1) & 0xffffffff
        k = ((k << 15) | (k >> 17)) & 0xffffffff
        k = (k * c2) & 0xffffffff
        h ^= k
    h ^= len(b)
    h ^= h >> 16
    h = (h * 0x85ebca6b) & 0xffffffff
    h ^= h >> 13
    h = (h * 0xc2b2ae35) & 0xffffffff
    h ^= h >> 16
    return h


def jenkins(b):
    h = 0
    for x in b:
        h = (h + x) & 0xffffffff
        h = (h + (h << 10)) & 0xffffffff
        h ^= h >> 6
    h = (h + (h << 3)) & 0xffffffff
    h ^= h >> 11
    h = (h + (h << 15)) & 0xffffffff
    return h


def bkdr(b):
    h = 0
    for x in b:
        h = (h * 31 + x) & 0xffffffff
    return h


# Verified with mmu_crc32() against every key collected from guns.mmu and
# levels.mmu. Mixed naming conventions are genuine: hashing is case-sensitive
# and performs no path or separator normalization.
KNOWN_KEY_NAMES = {
    0x0166019a: 'LiftSpeed',
    0x02e313f6: 'StompSpeed',
    0x045fdd3a: 'RunMoveSpeed',
    0x0588c18c: 'WarningDelay',
    0x05b1e8ba: 'Speed',
    0x07cd41c3: 'FormDelay',
    0x0849240d: 'SpreadDelay',
    0x0cd7796b: 'MoveAngle',
    0x0ef5a8f1: 'LeaveDelay',
    0x10191f1c: 'MoveDelay',
    0x16c2433f: 'slide_speed',
    0x1708e84c: 'AppearDelay',
    0x182a0cf5: 'LifeDuration',
    0x18a3cae8: 'ShotInterval',
    0x195189d8: 'WalkMoveSpeed',
    0x1a19b4b0: 'FirstDashDistance',
    0x1cbabb27: 'AttackMoveSpeed',
    0x1eb09d84: 'FlyUpSpeed',
    0x1fdbc887: 'FlyDownSpeed',
    0x203ae8da: 'DisappearDuration',
    0x210bacd7: 'StabDelay',
    0x2213db76: 'HoverHeight',
    0x24508922: 'ItemDropDelay',
    0x2512d13d: 'HoverSpeed',
    0x28b00af0: 'DropDistance',
    0x2ba52f7b: 'StandDelay',
    0x29aa5740: 'RotateSpeed',
    0x2cbaed73: 'ExplosionDistance',
    0x2e0a9032: 'ContainerDelay',
    0x2f00b66d: 'MaxRange',
    0x2fe90bf1: 'MaxShotAngle',
    0x30556303: 'Placeholder',
    0x337c716a: 'SpawnDelay',
    0x33e61185: 'ShotAngleOffset',
    0x3b86ccf4: 'Nothing',
    0x3ceccf5d: 'SwingSpeed',
    0x3d450f17: 'MoveAcc',
    0x3e087162: 'GroundIdleDelay',
    0x3e97cc6d: 'AttackRatio',
    0x3fb62a92: 'YokuSpikeDelay',
    0x422c14db: 'ChargeFlashDuration',
    0x42714480: 'TargetHotSpotY',
    0x42714481: 'TargetHotSpotX',
    0x4382ef7a: 'DashMoveSpeed',
    0x48f5687b: 'BounceHeight',
    0x4c3a19e7: 'TargetOffset',
    0x4f9b7b62: 'ResetMoveSpeed',
    0x50576724: 'AppearSpeed',
    0x521487ca: 'HoverAttackRatio',
    0x552cbcf8: 'ResurrectSpeed',
    0x57469074: 'MoveSpeed',
    0x594d9da1: 'LightningChargeDelay',
    0x5b4fa65f: 'EscapeSpeed',
    0x5d72f698: 'LaserDelay',
    0x5c496777: 'ExplodeDistance',
    0x643fe84d: 'DropSpeed',
    0x6339d16b: 'MinDistance',
    0x656c9ba3: 'VanishDelay',
    0x665423bf: 'StabSpeed',
    0x6bf06f6e: 'ExplosionCenterOffsetY',
    0x6c1852b9: 'AttackDelay',
    0x6d514c59: 'ShotWarningDelay',
    0x6d7fd691: 'MoveSpeedDown',
    0x6dc4ae65: 'InvinsibilityDuration',
    0x6e86e0d3: 'MinHeight',
    0x72a7a32d: 'JumpMoveSpeed',
    0x733600e5: 'ExplosionVelocityY',
    0x7423fe02: 'SpawnSpeed',
    0x793138c0: 'FlySpeed',
    0x7d68b195: 'DashSpeed',
    0x83797c67: 'LightningAttackRatio',
    0x8846cdf9: 'SlideSpeed',
    0x89c83879: 'max_fall_speed',
    0x8b45f83e: 'MaxDistance',
    0x8b781087: 'RotateRadius',
    0x8cba01ac: 'HoverShotDelay',
    0x8e355bbb: 'MaxHeight',
    0x90bb6912: 'SparkAttackDelay',
    0x927cb008: 'SlideDistance',
    0x93f46ce8: 'SuperCannonLengthCenter',
    0x9410f8c8: 'PoundDelay',
    0x99f8351f: 'ExplosionNumber',
    0x9a220fae: 'TargetAngleDetection',
    0x9f2d439d: 'move_speed',
    0x9f601e69: 'SlideMoveSpeed',
    0xa03a1ae9: 'ShotDuration',
    0xa0fbc058: 'PauseDelay',
    0xa21be30b: 'HitPoints',
    0xa3ffac6c: 'RaiseSpeed',
    0xa7106fa1: 'ConvergePause',
    0xa71422fc: 'SecondDashDistance',
    0xac1d3cc1: 'ShotMoveSpeed',
    0xac25109b: 'ShieldEndurance',
    0xad3194c8: 'WaitDelay',
    0xada1445a: 'ResetDelay',
    0xafda997f: 'RamSpeed',
    0xb05f9893: 'ShotAngle',
    0xb40a8909: 'ChargeTime',
    0xb6ef97c8: 'GroundDelay',
    0xb98e43bd: 'IdleDelay',
    0xbb435f79: 'ConvergeSpeed',
    0xbb85996a: 'MoveRadius',
    0xbc12b4fb: 'FallSpeed',
    0xbc49de4e: 'Fly_Speed',
    0xc167f1a0: 'Damage',
    0xc3ae9999: 'ExplosionDelay',
    0xc7d9bcc0: 'RunSpeed',
    0xc8276d9b: 'MovingDelay',
    0xc9713918: 'DeathDelay',
    0xcf548bdf: 'JumpDelay',
    0xd0c0a2ab: 'GroundMoveSpeed',
    0xd1f6e2b3: 'LightningAttackDelay',
    0xd3da6201: 'BulletSpeed',
    0xd728b63e: 'ProjectileSteps',
    0xd8ace190: 'MoveSpeedUp',
    0xda1f1441: 'MoveDistance',
    0xdd11ec47: 'SwingAcc',
    0xdde4bc4a: 'LiftHeight',
    0xdea1d812: 'DashDistance',
    0xe0595b4a: 'GlueShotHeight',
    0xe1e66eb0: 'ShootDelay',
    0xea13ba61: 'AirMoveSpeed',
    0xeaa08730: 'PatrolSpeed',
    0xeafecb32: 'ResetSpeed',
    0xebce718c: 'ShotSpeed',
    0xebd9cbdb: 'SwingBounceHeight',
    0xed892561: 'CrushChargeSpeed',
    0xf1b018a0: 'GroundSpeed',
    0xf6743f2d: 'ReturnDelay',
    0xfb2481ab: 'JumpHeight',
    0xfbc651d9: 'RamDelayMin',
    0xfbc659cf: 'RamDelayMax',
    0xfc3e6048: 'LightningDuration',
    0xfca554ad: 'LaserFireDelay',
    0xfed98a3f: 'SpreadDuration',
    0xff0c1ff5: 'JumpDistance',
    0xff5e37e7: 'CrushReturnSpeed',
}


HASHES = {
    'mmu_crc32_0x43c010': mmu_crc32,
    'zlib_crc32': zlib_crc32, 'fnv1a': fnv1a, 'fnv1': fnv1,
    'msvc_stdhash': msvc_stdhash, 'djb2': djb2, 'sdbm': sdbm,
    'murmur2': murmur2, 'murmur3': murmur3, 'jenkins': jenkins, 'bkdr': bkdr,
}


def main():
    exe, asset_dir = sys.argv[1], sys.argv[2]
    keys = collect_keys(asset_dir)
    print(f'collected {len(keys)} unique asset keys')

    bad = [(key, name, mmu_crc32(name.encode()))
           for key, name in KNOWN_KEY_NAMES.items()
           if mmu_crc32(name.encode()) != key]
    if bad:
        for key, name, actual in bad:
            print(f'BAD KNOWN MAPPING {key:08x} {name} -> {actual:08x}')
        return 1
    resolved = sorted((key, name) for key, name in KNOWN_KEY_NAMES.items()
                      if key in keys)
    print(f'known key dictionary resolves {len(resolved)}/{len(keys)} collected keys')
    doc = pathlib.Path(__file__).resolve().parent.parent / 'docs' / 'key_names.txt'
    lines = [
        'Generated by tools/crack_hash.py',
        'Hash: MMU.exe 0x43c010, non-reflected CRC-32, poly 0x04c11db7,',
        'seed 0, no final XOR. Hashing is case-sensitive and uses raw bytes.',
        '',
        f'resolved={len(resolved)} collected={len(keys)}',
        '',
    ]
    lines += [f'0x{key:08x} {name}' for key, name in resolved]
    doc.write_text('\n'.join(lines) + '\n')
    print(f'wrote {doc}')

    words = sorted(set(strings(open(exe, 'rb').read())))
    print(f'testing {len(words)} strings')
    for name, fn in HASHES.items():
        for variant, transform in (('raw', lambda b: b),
                                   ('lower', lambda b: b.lower()),
                                   ('upper', lambda b: b.upper())):
            hits = [(fn(transform(w)), w) for w in words if fn(transform(w)) in keys]
            if hits:
                print(f'{name:20s} {variant:5s} hits={len(hits):5d}  e.g. '
                      + ', '.join(f'{w.decode(errors="replace")}=0x{h:08x}'
                                  for h, w in hits[:6]))
    return 0


if __name__ == '__main__':
    sys.exit(main())
