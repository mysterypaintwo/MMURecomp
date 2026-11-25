#!/usr/bin/env python3
"""Make the .nro icon: the 1UP Mega Man head sprite, scaled up pixel-exact.

    extract_icon.py <game dir> out.jpg

Reads Collectibles/Extra-Life (sprite key 0x45bb8376, 16x16) from the game's
own Assets/gameobjects.mmu, scales it x16 with nearest-neighbour to the 256x256
JPEG an .nro needs, on black. Used by recomp/CMakeLists.txt at build time.
Needs Pillow and protobuf (tools/inspect_assets.py recovers the schemas from
MMU.exe); without them the build falls back to libounce's default icon.
"""
import os
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))

from google.protobuf import message_factory  # noqa: E402
from inspect_assets import build_pool  # noqa: E402
from PIL import Image  # noqa: E402

EXTRA_LIFE_KEY = 0x45bb8376


def surface_rgba(surf):
    pf = surf.pixel_format
    data = b''.join(surf.pixels)
    out = bytearray()
    bpp = pf.bytes_per_pixel
    for y in range(surf.h):
        row = data[y * surf.pitch:]
        for x in range(surf.w):
            v = int.from_bytes(row[x * bpp:(x + 1) * bpp], 'little')
            ch = []
            for mask, shift in ((pf.rmask, pf.rshift), (pf.gmask, pf.gshift), (pf.bmask, pf.bshift),
                                (pf.amask, pf.ashift)):
                ch.append(((v & mask) >> shift) & 0xff if mask else 255)
            out += bytes(ch)
    return Image.frombytes('RGBA', (surf.w, surf.h), bytes(out))


def main():
    game, out = sys.argv[1], sys.argv[2]
    pool = build_pool(os.path.join(game, 'MMU.exe'))
    AllSprites = message_factory.GetMessageClass(pool.FindMessageTypeByName('mmu.AllSprites'))
    sprites = AllSprites()
    sprites.ParseFromString(pathlib.Path(game, 'Assets', 'gameobjects.mmu').read_bytes())
    sprite = next(s for s in sprites.sprites if s.key == EXTRA_LIFE_KEY)
    img = surface_rgba(sprite.animation.frames[0].image)
    scale = 256 // max(img.size)
    img = img.resize((img.size[0] * scale, img.size[1] * scale), Image.NEAREST)
    canvas = Image.new('RGB', (256, 256), (0, 0, 0))
    canvas.paste(img, ((256 - img.size[0]) // 2, (256 - img.size[1]) // 2), img)
    canvas.save(out, 'JPEG', quality=100, subsampling=0)


if __name__ == '__main__':
    main()
