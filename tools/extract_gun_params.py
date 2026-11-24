#!/usr/bin/env python3
"""Recover engine gun parameters by scanning MMU.exe for `mov edi,<hash>`.

The engine reads GameParams with a helper at 0x43c800 that takes the key in
EDI and the owning GameObject in EAX. Enemy constructors therefore contain
runs of::

    mov     edi, 0x57469074
    mov     eax, this
    call    0x43c800
    fstp    dword ptr [this + field]

Each hash is also a GameParam.key in guns.mmu. Matching the hash to the
decoded asset recovers the concrete value; a nearby vftable store identifies
the owning class.

Usage:
  python3 tools/extract_gun_params.py MMU.exe Assets/guns.mmu [out.txt]
"""

import pathlib
import re
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from analyze_rtti import PE
from google.protobuf import descriptor_pool, message_factory
from extract_protos import PROTO_FILES, recover

GET_PARAM = 0x43C800
VFTABLE_VAS = {int(m, 16) for m in re.findall(
    r'VA (0x[0-9a-f]+)',
    (pathlib.Path(__file__).resolve().parent.parent / 'docs' / 'vtables.txt')
    .read_text(encoding='utf8'))}


def load_guns(exe_path, guns_path):
    exe = pathlib.Path(exe_path).read_bytes()
    pool = descriptor_pool.DescriptorPool()
    for name in ("sdl", "animation", "cutscene", "gun", "level", "savedgame"):
        pool.Add(recover(exe, PROTO_FILES[name]))
    cls = message_factory.GetMessageClass(pool.FindMessageTypeByName("mmu.Guns"))
    guns = cls()
    guns.ParseFromString(pathlib.Path(guns_path).read_bytes())
    params = {}
    for obj in guns.guns:
        for p in obj.game_params:
            params.setdefault(p.key, []).append((obj.key, p.param))
    return params


def vftable_store(pe, data, lo, hi, pos):
    """Find the nearest preceding `mov [reg], <known vftable VA>` store."""
    for p in range(pos - 1, max(lo, pos - 0x800), -1):
        if data[p] != 0xC7:
            continue
        modrm = data[p + 1]
        if modrm & 0xC0 or (modrm & 0x38) or modrm == 0x04:
            continue
        imm = struct.unpack_from('<I', data, p + 2)[0]
        if imm in VFTABLE_VAS:
            return imm
    return None


def scan(pe, params, out):
    lo = pe.rva_to_off(pe.text_lo)
    hi = lo + next(raw for name, _, _, _, raw in pe.sections if name == '.text')
    data = pe.data
    hits = {}
    for pos in range(lo, hi - 6):
        if data[pos] != 0xBF:  # mov edi, imm32
            continue
        h = struct.unpack_from('<I', data, pos + 1)[0]
        if h not in params:
            continue
        # Look ahead for a direct call to the GameParam helper.
        found = False
        for step in range(pos + 5, min(pos + 41, hi - 5)):
            if data[step] != 0xE8:
                continue
            rel = struct.unpack_from('<i', data, step + 1)[0]
            target = (pe.image_base + pe.off_to_rva(step) + 5 + rel) & 0xFFFFFFFF
            if target == GET_PARAM:
                found = True
                break
        if not found:
            continue
        vft = vftable_store(pe, data, lo, hi, pos)
        if vft is None:
            continue
        va = pe.image_base + pe.off_to_rva(pos)
        hits.setdefault(vft, []).append((va, h))

    lines = []
    for vft in sorted(hits):
        lines.append(f"\nvftable 0x{vft:x}")
        for at, h in hits[vft]:
            # A key shared by every object is not a class-specific constant.
            vals = params[h]
            note = " (shared)" if len(vals) > 40 else ""
            desc = ", ".join(f"obj=0x{o:x} v={v:g}" for o, v in vals[:8])
            lines.append(f"  0x{at:x} key=0x{h:08x} {desc}{note}")
    result = "\n".join(lines) + "\n"
    if out:
        pathlib.Path(out).write_text(result)
    return result, len(hits)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    params = load_guns(sys.argv[1], sys.argv[2])
    text, functions = scan(PE(pathlib.Path(sys.argv[1]).read_bytes()), params,
                           sys.argv[3] if len(sys.argv) > 3 else None)
    print(f"scanned; {functions} functions, {len(text.splitlines())} report lines")
    print(text[:16000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
