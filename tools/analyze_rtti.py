#!/usr/bin/env python3
"""Locate MSVC RTTI vtables and virtual functions in a PE32 image.

MSVC lays out RTTI for each polymorphic class like this::

  TypeDescriptor        +0 vptr, +4 spare, +8 inline mangled name (.?AVFoo@@)
  CompleteObjectLocator +0 signature=1, +4 offset, +8 cdOffset,
                        +12 VA of TypeDescriptor, +16 VA of class hierarchy
  vftable[-4]           VA of the CompleteObjectLocator
  vftable[0..]          virtual function pointers into .text

No external dependencies; parses PE headers directly.

Usage: python3 tools/analyze_rtti.py MMU.exe [--all] [ClassName ...]

Without class filters the report is capped at 60 classes for interactive use;
--all writes every discovered vtable (used to regenerate docs/vtables.txt).
"""

import struct
import sys


class PE:
    def __init__(self, data):
        self.data = data
        self.pe = struct.unpack_from('<I', data, 0x3c)[0]
        assert data[self.pe:self.pe + 4] == b'PE\0\0'
        nsec, = struct.unpack_from('<H', data, self.pe + 6)
        opt_size, = struct.unpack_from('<H', data, self.pe + 20)
        opt = self.pe + 24
        magic, = struct.unpack_from('<H', data, opt)
        assert magic == 0x10b, 'expected PE32'
        self.image_base, = struct.unpack_from('<I', data, opt + 28)
        self.sections = []
        for i in range(nsec):
            off = opt + opt_size + 40 * i
            name = data[off:off + 8].rstrip(b'\0').decode('latin1')
            vsize, va, rawsize, rawoff = struct.unpack_from('<IIII', data, off + 8)
            self.sections.append((name, va, vsize, rawoff, rawsize))
        self.rdata_lo, self.rdata_hi = self.section_range('.rdata')
        self.text_lo, self.text_hi = self.section_range('.text')

    def section_range(self, name):
        for n, va, vsize, _, rawsize in self.sections:
            if n == name:
                return va, va + max(vsize, rawsize)
        return 0, 0

    def off_to_rva(self, off):
        for name, va, vsize, rawoff, rawsize in self.sections:
            if rawoff <= off < rawoff + rawsize:
                return va + off - rawoff
        return None

    def rva_to_off(self, rva):
        for name, va, vsize, rawoff, rawsize in self.sections:
            if va <= rva < va + max(vsize, rawsize):
                return rawoff + (rva - va)
        return None

    def u32(self, rva):
        off = self.rva_to_off(rva)
        return struct.unpack_from('<I', self.data, off)[0] if off is not None else None

    def in_rdata(self, rva):
        return self.rdata_lo <= rva < self.rdata_hi

    def in_text(self, va):
        # Virtual addresses of .text are image_base + rva.
        return self.text_lo <= va - self.image_base < self.text_hi


def find_all(data, needle):
    start = 0
    while True:
        i = data.find(needle, start)
        if i < 0:
            return
        yield i
        start = i + 1


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    pe = PE(open(sys.argv[1], 'rb').read())
    args = sys.argv[2:]
    all_classes = '--all' in args
    args = [a for a in args if a != '--all']
    wanted = set(args)

    # 1. TypeDescriptors: MSVC stores the mangled name inline at +8.
    type_descriptors = {}  # TypeDescriptor VA -> class name
    for off in find_all(pe.data, b'.?AV'):
        end = pe.data.find(b'@@', off)
        if end < 0:
            continue
        name = pe.data[off:end + 2].decode('latin1')
        if 'google' in name or 'protobuf' in name:
            continue
        rva = pe.off_to_rva(off)
        if rva is None or rva < 8:
            continue
        td_va = pe.image_base + rva - 8
        type_descriptors[td_va] = name

    # 2. Complete Object Locators: +12 holds the VA of the TypeDescriptor.
    cols = {}  # COL VA -> class name
    for td_va, name in type_descriptors.items():
        ptr = struct.pack('<I', td_va)
        for off in find_all(pe.data, ptr):
            rva = pe.off_to_rva(off)
            if rva is None or rva < 12:
                continue
            col_rva = rva - 12
            if pe.u32(col_rva) not in (0, 1):  # 0 = x86, 1 = x64 MSVC
                continue
            if not pe.in_rdata(col_rva):
                continue
            class_desc = pe.u32(col_rva + 16)
            if class_desc is None or not pe.in_rdata(class_desc - pe.image_base):
                continue
            cols[pe.image_base + col_rva] = name

    # 3. vftables: the slot immediately before the table points to the COL.
    vtables = {}  # class name -> list of vftable RVAs
    for col_va, name in cols.items():
        ptr = struct.pack('<I', col_va)
        for off in find_all(pe.data, ptr):
            rva = pe.off_to_rva(off)
            if rva is None:
                continue
            vft_rva = rva + 4
            first = pe.u32(vft_rva)
            if first is None or not pe.in_text(first):
                continue
            vtables.setdefault(name, []).append(vft_rva)

    print(f'image base 0x{pe.image_base:x}, type descriptors={len(type_descriptors)}, '
          f'COLs={len(cols)}, vtables={len(vtables)}')
    if wanted:
        keys = [k for k in vtables if any(w.lower() in k.lower() for w in wanted)]
    else:
        keys = sorted(vtables, key=lambda k: (not any(
            w in k for w in ('MegaMan', 'Enemy', 'Boss', 'Camera', 'Level', 'GameObject',
                             'BaseObject', 'Unit', 'Met', 'Spike', 'Platform')), k))
        if not all_classes:
            keys = keys[:60]
    for cls in keys:
        for vft in vtables[cls]:
            funcs = []
            # Enemy vtables append per-state enter/update/exit triples well
            # past the 24 shared GameObject slots; Telly's last state hook is
            # slot 30 (0x78). 64 covers the largest table in this binary and
            # still stops at the first non-text pointer.
            for i in range(64):
                target = pe.u32(vft + 4 * i)
                if target is None or not pe.in_text(target):
                    break
                funcs.append(target)
            print(f'\n{cls}  vftable RVA 0x{vft:x} (VA 0x{pe.image_base + vft:x})')
            print('  ' + ' '.join(f'0x{f:x}' for f in funcs))
    return 0


if __name__ == '__main__':
    sys.exit(main())
