#!/usr/bin/env python3
"""Reverse-engineering helper for MMU.exe.

Ghidra output names read-only constants `_DAT_00697ce0`, `DAT_0067de1c` etc.
This tool resolves them against the PE image so a decompiled function can be
read without bouncing to the listing:

  python3 tools/re_annotate.py c   <file.c> [more.c ...]   # annotate C, print
  python3 tools/re_annotate.py dis <va> [count]            # disassemble + annotate
  python3 tools/re_annotate.py val <va> [va ...]           # show f32/f64/str/u32
  python3 tools/re_annotate.py vt  <vftable_va> [count]    # list vftable slots
  python3 tools/re_annotate.py hash <text> [text ...]      # MMU 0x43c010 CRC-32

Set MMU_EXE to override the default executable path.

Annotations are appended as `/* f=… d=… s="…" */` comments. Writable .data
globals (mutable state) are tagged `/* .data */` with their initial value; they
are not constants and must not be folded into the reconstruction as literals.
"""

import os
import re
import struct
import sys

EXE = os.environ.get('MMU_EXE', os.path.expanduser('~/Downloads/MegaManUnlimited/MMU.exe'))


class Image:
    def __init__(self, path):
        self.data = open(path, 'rb').read()
        pe = struct.unpack_from('<I', self.data, 0x3c)[0]
        count = struct.unpack_from('<H', self.data, pe + 6)[0]
        opt = struct.unpack_from('<H', self.data, pe + 20)[0]
        self.base = struct.unpack_from('<I', self.data, pe + 24 + 28)[0]
        self.sections = []
        for i in range(count):
            o = pe + 24 + opt + i * 40
            name = self.data[o:o + 8].rstrip(b'\0').decode('latin1')
            vsize, va, rsize, raw = struct.unpack_from('<IIII', self.data, o + 8)
            self.sections.append((name, self.base + va, vsize, raw, rsize))

    def section(self, va):
        for s in self.sections:
            if s[1] <= va < s[1] + s[2]:
                return s
        return None

    def read(self, va, size):
        s = self.section(va)
        if not s:
            return None
        off = va - s[1]
        if off + size > s[4]:
            return b'\0' * size  # .bss tail: zero-initialised
        return self.data[s[3] + off:s[3] + off + size]

    def cstr(self, va, limit=96):
        raw = self.read(va, limit)
        if not raw:
            return None
        end = raw.find(b'\0')
        if end <= 0:
            return None
        text = raw[:end]
        if all(32 <= b < 127 or b in (9, 10, 13) for b in text):
            return text.decode('latin1')
        return None

    def describe(self, va):
        s = self.section(va)
        if not s:
            return None
        parts = []
        if s[0] == '.data':
            parts.append('.data')
        raw = self.read(va, 8)
        text = self.cstr(va)
        if text and len(text) >= 2:
            parts.append('s="%s"' % text.replace('\\', '\\\\'))
        else:
            f = struct.unpack('<f', raw[:4])[0]
            d = struct.unpack('<d', raw)[0]
            u = struct.unpack('<I', raw[:4])[0]
            if f == f and abs(f) < 1e9 and (f == 0 or abs(f) > 1e-6):
                parts.append('f=%.9g' % f)
            if d == d and abs(d) < 1e9 and (d == 0 or abs(d) > 1e-6) and (raw[4:] != b'\0\0\0\0'):
                parts.append('d=%.12g' % d)
            parts.append('u=0x%x' % u)
        return ' '.join(parts)


def mmu_crc32(b):
    h = 0
    for c in b:
        for i in range(7, -1, -1):
            top = h & 0x80000000
            h = ((h << 1) | ((c >> i) & 1)) & 0xffffffff
            if top:
                h ^= 0x04c11db7
    return h


SYM = re.compile(r'(?:_DAT_|DAT_|PTR_DAT_|s_[^ ,;)]*_)00([0-9a-f]{6})\b')


_HASHES = None


def hash_names(img):
    """CRC-32 (0x43c010) of every printable string in the image plus the
    verified GameParam names, so hashed keys in code can be labelled."""
    global _HASHES
    if _HASHES is None:
        _HASHES = {}
        for m in re.finditer(rb'[\x20-\x7e]{2,48}', img.data):
            _HASHES.setdefault(mmu_crc32(m.group(0)), m.group(0).decode('latin1'))
        root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        for name in ('key_names.txt',):
            path = os.path.join(root, 'docs', name)
            if os.path.exists(path):
                for line in open(path, encoding='utf8'):
                    m = re.match(r'(0x[0-9a-f]{8}) (\S+)', line)
                    if m:
                        _HASHES[int(m.group(1), 16)] = m.group(2)
    return _HASHES


def annotate_c(img, text):
    names = hash_names(img)
    out = []
    for line in text.splitlines():
        notes = []
        seen = set()
        for m in SYM.finditer(line):
            va = int(m.group(1), 16)
            if va in seen:
                continue
            seen.add(va)
            d = img.describe(va)
            if d:
                notes.append('%x:%s' % (va, d))
        # Bare hex immediates that look like float bit patterns.
        for m in re.finditer(r'0x([34][0-9a-f]{7}|b[e-f][0-9a-f]{6}|c[0-4][0-9a-f]{6})\b', line):
            v = int(m.group(1), 16)
            f = struct.unpack('<f', struct.pack('<I', v))[0]
            if 1e-4 < abs(f) < 1e6:
                notes.append('%s=f%.6g' % (m.group(0), f))
        for m in re.finditer(r'(-?)0x([0-9a-f]{6,8})\b', line):
            v = int(m.group(2), 16)
            if m.group(1):
                v = (-v) & 0xffffffff
            if v in names and v > 0xffff:
                notes.append('#%08x="%s"' % (v, names[v]))
        out.append(line + ('  /* ' + '; '.join(notes) + ' */' if notes else ''))
    return '\n'.join(out)


def disassemble(img, va, count):
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    code = img.read(va, count * 15)
    n = 0
    for ins in md.disasm(code, va):
        notes = []
        for m in re.finditer(r'0x([0-9a-f]{6,8})', ins.op_str):
            v = int(m.group(1), 16)
            s = img.section(v)
            if s and s[0] in ('.rdata', '.data'):
                d = img.describe(v)
                if d:
                    notes.append('%x:%s' % (v, d))
        print('%08x  %-7s %-40s %s' % (ins.address, ins.mnemonic, ins.op_str,
                                       ('; ' + ' | '.join(notes)) if notes else ''))
        n += 1
        if n >= count or ins.mnemonic == 'int3':
            break


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 1
    img = Image(EXE)
    cmd = sys.argv[1]
    if cmd == 'c':
        for path in sys.argv[2:]:
            print(annotate_c(img, open(path, encoding='utf8', errors='replace').read()))
    elif cmd == 'dis':
        disassemble(img, int(sys.argv[2], 16), int(sys.argv[3]) if len(sys.argv) > 3 else 80)
    elif cmd == 'val':
        for a in sys.argv[2:]:
            print(a, img.describe(int(a, 16)))
    elif cmd == 'vt':
        vt = int(sys.argv[2], 16)
        n = int(sys.argv[3]) if len(sys.argv) > 3 else 64
        for i in range(n):
            print(i, '0x%x' % struct.unpack('<I', img.read(vt + 4 * i, 4))[0])
    elif cmd == 'hash':
        for t in sys.argv[2:]:
            print('0x%08x %s' % (mmu_crc32(t.encode()), t))
    return 0


if __name__ == '__main__':
    sys.exit(main())
