#!/usr/bin/env python3
"""Determine the top-level protobuf type of each .mmu asset file.

A candidate type is correct when the file parses cleanly, has no unknown
fields, and re-serializes to exactly the original bytes.
"""
import argparse
import pathlib
import sys
import time

from google.protobuf import message_factory

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from inspect_assets import build_pool

CANDIDATES = ["mmu.Database", "mmu.CutScene", "mmu.Guns", "mmu.AllSprites",
              "mmu.PNG", "mmu.Level", "mmu.SavedGame"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", required=True)
    ap.add_argument("--assets", required=True)
    ap.add_argument("--types", nargs="*", default=CANDIDATES)
    args = ap.parse_args()

    pool = build_pool(args.exe)
    prototypes = {n: message_factory.GetMessageClass(pool.FindMessageTypeByName(n))
                  for n in args.types}

    for path in sorted(pathlib.Path(args.assets).glob("*.mmu")):
        raw = path.read_bytes()
        matches = []
        for name, cls in prototypes.items():
            msg = cls()
            started = time.time()
            try:
                msg.ParseFromString(raw)
                msg.DiscardUnknownFields()
                out = msg.SerializeToString()
                ok = out == raw
                matches.append((ok, name, len(msg.ListFields()), time.time() - started))
            except Exception as exc:
                matches.append((False, name, 0, time.time() - started))
        matches.sort(reverse=True)
        print(f"{path.name} ({len(raw):,} bytes)")
        for ok, name, fields, elapsed in matches[:3]:
            print(f"   {'MATCH' if ok else 'no':<6} {name:<20} top_fields={fields} {elapsed:.1f}s")


if __name__ == "__main__":
    main()
