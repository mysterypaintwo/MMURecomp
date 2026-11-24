#!/usr/bin/env python3
"""Identify which protobuf message type each MMU asset file contains.

Reads only a small prefix of each file, so the 400+ MB of assets are never
fully loaded just to determine their schema.
"""
import argparse
import pathlib
import sys

from google.protobuf import descriptor_pool, message_factory

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from extract_protos import PROTO_FILES, recover

DEP_ORDER = ["sdl", "animation", "cutscene", "gun", "level", "savedgame"]
TOP_LEVEL = [
    "mmu.SDL_Color", "mmu.SDL_Palette", "mmu.SDL_PixelFormat", "mmu.SDL_Rect",
    "mmu.SDL_Surface", "mmu.PNG",
    "mmu.AnimationFrame", "mmu.Animation", "mmu.Misc", "mmu.Sprite", "mmu.AllSprites",
    "mmu.Command", "mmu.TimeStamp", "mmu.CutScene", "mmu.Database",
    "mmu.GameParam", "mmu.Sound", "mmu.GameObject", "mmu.Guns",
    "mmu.GameSpawnObject", "mmu.Scene", "mmu.Level",
    "mmu.SavedGame", "mmu.SavedGameSlot",
]


def build_pool(exe_path):
    data = pathlib.Path(exe_path).read_bytes()
    pool = descriptor_pool.DescriptorPool()
    for name in DEP_ORDER:
        pool.Add(recover(data, PROTO_FILES[name]))
    return pool


def varint(buf, pos):
    value = 0
    shift = 0
    while pos < len(buf):
        byte = buf[pos]
        pos += 1
        value |= (byte & 0x7F) << shift
        if not byte & 0x80:
            return value, pos
        shift += 7
    raise ValueError("truncated varint")


def field_boundaries(buf):
    """Return byte offsets just after each complete top-level protobuf field."""
    pos = 0
    ends = []
    while pos < len(buf) - 1:
        try:
            tag, pos = varint(buf, pos)
            wire = tag & 7
            if wire == 2:
                length, pos = varint(buf, pos)
                pos += length
            elif wire == 0:
                _, pos = varint(buf, pos)
            elif wire == 5:
                pos += 4
            elif wire == 1:
                pos += 8
            else:
                break
        except ValueError:
            break
        if pos > len(buf):
            break
        ends.append(pos)
    return ends


def score(msg, proto):
    """Prefer parses that explain fields at every level, not just the top.

    upb (the default C backend) has no UnknownFields() accessor, so measure
    unknown bytes by discarding them and comparing serialized lengths.
    """
    before = len(msg.SerializeToString())
    msg.DiscardUnknownFields()
    unknown = before - len(msg.SerializeToString())
    known = 0
    for field, value in msg.ListFields():
        known += 1
        if field.message_type is not None:
            items = value if field.label == field.LABEL_REPEATED else [value]
            for item in items:
                k, u = score(item, None)
                known += k
                unknown += u
    return known, unknown


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", required=True)
    ap.add_argument("--assets", required=True)
    ap.add_argument("--prefix", type=int, default=32768)
    args = ap.parse_args()

    pool = build_pool(args.exe)
    factory = message_factory.MessageFactory(pool)
    prototypes = {n: message_factory.GetMessageClass(pool.FindMessageTypeByName(n)) for n in TOP_LEVEL}

    for path in sorted(pathlib.Path(args.assets).glob("*.mmu")):
        prefix = path.open("rb").read(args.prefix)
        boundaries = [b for b in field_boundaries(prefix) if b <= len(prefix)]
        results = []
        for name, proto in prototypes.items():
            for length in reversed(boundaries[:16]):
                msg = proto()
                try:
                    msg.ParseFromString(prefix[:length])
                except Exception:
                    continue
                try:
                    known, unknown = score(msg, proto)
                except Exception:
                    continue
                if known:
                    fields = ", ".join(f.name for f, _ in msg.ListFields()[:4])
                    results.append((known, -unknown, length, name, fields))
                    break
        results.sort(reverse=True)
        print(f"{path.name} ({path.stat().st_size:,} bytes)")
        for known, neg_unknown, length, name, fields in results[:3]:
            print(f"   {name:<26} prefix={length:<6} known={known:<3} unknown={-neg_unknown:<6} [{fields}]")
        if not results:
            print("   no match")


if __name__ == "__main__":
    main()
