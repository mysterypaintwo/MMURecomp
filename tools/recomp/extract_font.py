#!/usr/bin/env python3
"""Write the game's 8x8 sprite font as a C header for the port's own messages.

    extract_font.py <game dir> out.h

Also records the title screen's "PRESS ENTER" (see title_prompt).

The font is the game's own \\Resources\\Font.png: a 128x48 surface in
Assets/misc.mmu (key 0x02113eab), ASCII 32-127 in a 16x6 grid of 8x8 glyphs.
Used at build time by recomp/CMakeLists.txt, so nothing from the game is
committed. On any failure the header says MMU_FONT_OK 0 and the port falls
back to not drawing text with it.
"""
import os
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))

FONT_KEY = 0x02113eab


def rgba(surf):
    pf = surf.pixel_format
    data = b''.join(surf.pixels)
    bpp = pf.bytes_per_pixel
    out = bytearray()
    for y in range(surf.h):
        row = data[y * surf.pitch:]
        for x in range(surf.w):
            v = int.from_bytes(row[x * bpp:(x + 1) * bpp], 'little')
            for mask, shift in ((pf.rmask, pf.rshift), (pf.gmask, pf.gshift), (pf.bmask, pf.bshift),
                                (pf.amask, pf.ashift)):
                out.append(((v & mask) >> shift) & 0xff if mask else 255)
    return bytes(out)


# The title screen's "PRESS ENTER" is painted into Title-BG2.png (the blink
# frame of Title-BG.png; Title-BG2Zero.png for Zero) in menus.mmu. The Switch
# build replaces it with "PRESS +": this records where the text is (pixels
# that differ between the two frames), and the background without it.
TITLE_KEYS = (0x92efadcb, 0x1fdc59c3)  # Title-BG, Title-BGZero


def title_prompt(game):
    from google.protobuf import message_factory
    from inspect_assets import build_pool
    pool = build_pool(os.path.join(game, 'MMU.exe'))
    Sprites = message_factory.GetMessageClass(pool.FindMessageTypeByName('mmu.AllSprites'))
    msg = Sprites()
    msg.ParseFromString(pathlib.Path(game, 'Assets', 'menus.mmu').read_bytes())
    frames = []
    for key in TITLE_KEYS:
        sprite = next(s for s in msg.sprites if s.key == key)
        plain, text = (rgba(f.image) for f in sprite.animation.frames[:2])
        frames.append((plain, text, sprite.animation.frames[0].image.w))
    diff = set()
    for plain, text, w in frames:
        diff |= {(i // 4 % w, i // 4 // w) for i in range(0, len(plain), 4) if plain[i:i + 4] != text[i:i + 4]}
    x0, x1 = min(x for x, _ in diff), max(x for x, _ in diff)
    y0, y1 = min(y for _, y in diff), max(y for _, y in diff)
    bw, bh = x1 - x0 + 1, y1 - y0 + 1

    def argb(img, w, x, y):
        r, g, b, a = img[(y * w + x) * 4:(y * w + x) * 4 + 4]
        return (a << 24) | (r << 16) | (g << 8) | b

    out = '#define MMU_PROMPT_OK 1\n#define MMU_PROMPT_X %d\n#define MMU_PROMPT_Y %d\n' % (x0, y0)
    out += '#define MMU_PROMPT_W %d\n#define MMU_PROMPT_H %d\n#define MMU_PROMPT_VARIANTS %d\n' % (bw, bh, len(frames))
    for name, idx in (('kMmuPromptPlain', 0), ('kMmuPromptText', 1)):
        rows = []
        for f in frames:
            rows.append(','.join('0x%08xu' % argb(f[idx], f[2], x, y)
                                 for y in range(y0, y1 + 1) for x in range(x0, x1 + 1)))
        out += 'static const unsigned %s[%d][%d] = {%s};\n' % (name, len(frames), bw * bh,
                                                             ','.join('{%s}' % r for r in rows))
    return out


# The controller mapping screen's hint, Menus/Main/ControlInfo.png (menus.mmu),
# reads "PUSH [ESC] TO SKIP A FUNCTION" (ESC as a key cap). Switch 2 has no
# Esc: the port skips with - and ZR together, so the Switch build shows two
# key caps, [-] [ZR], instead, in the same 5-pixel lettering.
CONTROL_INFO_KEY = 0xb1eb98a0
SKIP_GLYPHS = {
    '-': ['...', '...', '###', '...', '...'],
    'Z': ['####', '...#', '..#.', '.#..', '####'],
    'R': ['###.', '#..#', '###.', '#.#.', '#..#'],
}


def control_info(game):
    from google.protobuf import message_factory
    from inspect_assets import build_pool
    pool = build_pool(os.path.join(game, 'MMU.exe'))
    Sprites = message_factory.GetMessageClass(pool.FindMessageTypeByName('mmu.AllSprites'))
    msg = Sprites()
    msg.ParseFromString(pathlib.Path(game, 'Assets', 'menus.mmu').read_bytes())
    surf = next(s for s in msg.sprites if s.key == CONTROL_INFO_KEY).animation.frames[0].image
    px = rgba(surf)
    w, h = surf.w, surf.h
    ink = px[4 * (1 * w + 0):4 * (1 * w + 0) + 4]  # the P's top-left pixel
    old = [''.join('#' if px[4 * (y * w + x):4 * (y * w + x) + 4] == ink else '.' for x in range(w))
           for y in range(h)]
    # Two key caps from column 22, [-] [ZR] (pressed together), two columns
    # apart. Each is rounded on both sides like the original: an edge column
    # (rows 1-5), the letters cut out of the cap, an edge column, with the top
    # and bottom bars over the letters only. "PUSH" and "TO SKIP..." stay put.
    new = [list(old[y][:22]) + ['.'] * (w - 22) for y in range(h)]
    x = 22
    for cap in ('-', 'ZR'):
        letters = ['' for _ in range(5)]
        for i, ch in enumerate(cap):
            for r in range(5):
                letters[r] += ('.' if i else '') + SKIP_GLYPHS[ch][r]
        tw = len(letters[0])
        for y in range(h):
            if y in (0, 6):
                for c in range(x + 1, x + tw + 1):
                    new[y][c] = '#'
            elif 1 <= y <= 5:
                new[y][x] = new[y][x + tw + 1] = '#'
                for c in range(tw):
                    new[y][x + 1 + c] = '#' if letters[y - 1][c] == '.' else '.'
        x += tw + 4  # cap width (tw + 2) and a two-column gap
    x1 = x - 3  # last cap column
    if x1 > 40 or any('#' in r[40:45] for r in old):
        raise ValueError('ControlInfo.png layout changed')
    for y in range(h):  # the original [ESC] cap ends at column 39
        new[y][40:] = list(old[y][40:])
    new = [''.join(r[:w]) for r in new]
    r, g, b, _ = ink
    out = '#define MMU_SKIPINFO_OK 1\n#define MMU_SKIPINFO_W %d\n#define MMU_SKIPINFO_H %d\n' % (w, h)
    out += '#define MMU_SKIPINFO_INK 0x%06xu\n' % ((r << 16) | (g << 8) | b)
    for name, rows in (('kMmuSkipOld', old), ('kMmuSkipNew', new)):
        out += 'static const char* const %s[%d] = {%s};\n' % (name, h, ','.join('"%s"' % x for x in rows))
    return out


def main():
    game, out = sys.argv[1], sys.argv[2]
    try:
        from google.protobuf import message_factory
        from inspect_assets import build_pool
        pool = build_pool(os.path.join(game, 'MMU.exe'))
        PNG = message_factory.GetMessageClass(pool.FindMessageTypeByName('mmu.PNG'))
        msg = PNG()
        msg.ParseFromString(pathlib.Path(game, 'Assets', 'misc.mmu').read_bytes())
        surf = next(s for s, k in zip(msg.surface, msg.key) if k == FONT_KEY)
        pixels = rgba(surf)
        body = ','.join(str(b) for b in pixels)
        text = ('#pragma once\n#define MMU_FONT_OK 1\n#define MMU_FONT_W %d\n#define MMU_FONT_H %d\n'
                'static const unsigned char kMmuFont[] = {%s};\n' % (surf.w, surf.h, body))
    except Exception as e:  # missing deps or assets: build without the font
        print('extract_font.py: %s' % e, file=sys.stderr)
        text = '#pragma once\n#define MMU_FONT_OK 0\n'
    try:
        text += control_info(game)
    except Exception as e:
        print('extract_font.py (control info): %s' % e, file=sys.stderr)
        text += '#define MMU_SKIPINFO_OK 0\n'
    try:
        text += title_prompt(game)
    except Exception as e:
        print('extract_font.py (title prompt): %s' % e, file=sys.stderr)
        text += '#define MMU_PROMPT_OK 0\n'
    pathlib.Path(out).write_text('// Generated from the game files by extract_font.py. Do not commit.\n' + text)


if __name__ == '__main__':
    main()
