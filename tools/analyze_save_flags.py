#!/usr/bin/env python3
"""Mine the keyed save-flag store from MMU.exe.

The store is rooted at global 0x6c55fc. Its writer is FUN_004412d0 and its
reader is FUN_00441c80(key, mask); FUN_00441d90 and FUN_00441db0 are thin
wrappers that use masks 8 and 4. This analyzer recovers:

  * writer records: (category, key, auxiliary id, enable flag, min, max)
    from the cdecl push sequence immediately before each direct writer call;
  * reader keys: the literal EAX value loaded before each direct wrapper call;
  * direct reader masks: literal push values at the three direct FUN_00441c80
    sites, including the two with an intervening EDI manager load;
  * writer/reader key coverage, without assigning semantic names.

No semantic names are invented; all fields remain raw integers.

Outputs:
  docs/save_flags.txt
  core/include/mmu/core/SaveFlagTable.h
"""

import collections
import os
import struct
import sys

from analyze_rtti import PE

WRITER_VA = 0x4412D0
READER_VA = 0x441C80
READER8_VA = 0x441D90
READER4_VA = 0x441DB0
MANAGER_GLOBAL_VA = 0x6C55FC
# FUN_00441820 compares each condition id with 0x143. Lower values index the
# SDL key-state array; higher values index manager bytes at id - 0xf9.
KEYBOARD_LIMIT = 0x143
MANAGER_BYTE_BIAS = 0xF9
# Three direct reader sites load the global manager into EDI between the two
# literal pushes and the call: 8b 3d fc 55 6c 00.
DIRECT_READER_EDI_LOAD = bytes.fromhex("8b3dfc556c00")


def text_bytes(pe):
    _name, _rva, _vs, off, size = next(s for s in pe.sections if s[0] == ".text")
    return pe.image_base + _rva, pe.data[off:off + size]


def direct_call_vas(base, data, target):
    result = []
    for pos in range(len(data) - 5):
        if data[pos] != 0xE8:
            continue
        rel = struct.unpack_from("<i", data, pos + 1)[0]
        va = base + pos + 5 + rel
        if va == target:
            result.append(base + pos)
    return result


def u32(data, pos):
    return struct.unpack_from("<I", data, pos)[0]


def decode_literal_push(data, pos):
    """Return (value, byte length) for `push imm8/imm32` at pos."""
    if pos < 0 or pos >= len(data):
        return None
    if data[pos] == 0x6A and pos + 1 < len(data):
        return data[pos + 1], 2
    if data[pos] == 0x68 and pos + 4 < len(data):
        return u32(data, pos + 1), 5
    return None


def push_sequence(data, end, count):
    """Find `count` literal pushes ending exactly before byte offset end."""
    for start in range(max(0, end - 5 * count - 2), end):
        values = []
        pos = start
        for _ in range(count):
            decoded = decode_literal_push(data, pos)
            if decoded is None:
                break
            value, length = decoded
            values.append(value)
            pos += length
        if len(values) == count and pos == end:
            return values, start
    return None, None


def collect_writer_records(base, data):
    """Decode six literal pushes before FUN_004412d0 calls.

    The cdecl order at the writer is category, key, auxiliary id, enable flag,
    min, max, so the six pushes appear in reverse: max, min, enable, aux, key,
    category. The `mov ecx,[ebp+8]` immediately before the call supplies the
    manager pointer.
    """
    records = []
    for call_va in direct_call_vas(base, data, WRITER_VA):
        pos = call_va - base
        # Expected tail: six literal pushes, mov ecx,[ebp+8], call rel32.
        if data[pos - 3:pos] != b"\x8b\x4d\x08":
            continue
        pushes, start = push_sequence(data, pos - 3, 6)
        if pushes is None:
            continue
        max_rank, min_rank, enable, aux, key, category = pushes
        if enable not in (0, 1) or min_rank > max_rank:
            continue
        if aux < KEYBOARD_LIMIT:
            condition = "keyboard[%u]" % aux
        else:
            condition = "manager[0x%02x]" % (aux - MANAGER_BYTE_BIAS)
        records.append({
            "sourceVa": base + start,
            "category": category,
            "key": key,
            "aux": aux,
            "enable": enable,
            "min": min_rank,
            "max": max_rank,
            "condition": condition,
        })
    return records


def collect_reader_keys(base, data):
    """Collect literal EAX keys loaded immediately before wrapper calls."""
    grouped = {READER8_VA: [], READER4_VA: []}
    for target, rows in grouped.items():
        for call_va in direct_call_vas(base, data, target):
            pos = call_va - base
            if pos < 5 or data[pos - 5] != 0xB8:
                continue
            rows.append({
                "sourceVa": call_va,
                "key": u32(data, pos - 4),
            })
    return grouped


def collect_direct_reader_masks(base, data):
    rows = []
    for call_va in direct_call_vas(base, data, READER_VA):
        pos = call_va - base
        end = pos
        if data[end - len(DIRECT_READER_EDI_LOAD):end] == DIRECT_READER_EDI_LOAD:
            end -= len(DIRECT_READER_EDI_LOAD)
        pushes, start = push_sequence(data, end, 2)
        if pushes is None:
            continue
        rows.append({
            "sourceVa": base + start,
            "key": pushes[1],
            "mask": pushes[0],
        })
    return rows


def write_report(path, writers, readers, masks):
    writer_keys = collections.Counter(r["key"] for r in writers)
    reader8_keys = collections.Counter(r["key"] for r in readers[READER8_VA])
    reader4_keys = collections.Counter(r["key"] for r in readers[READER4_VA])
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_save_flags.py\n")
        out.write("Manager global=0x%06x writer=FUN_004412d0 reader=FUN_00441c80\n"
                  % MANAGER_GLOBAL_VA)
        out.write("writer records=%d reader8 calls=%d reader4 calls=%d "
                  "direct reader calls=%d\n\n" % (
                      len(writers), len(readers[READER8_VA]),
                      len(readers[READER4_VA]), len(masks)))
        out.write("Writer records (raw category/key/aux/enable/min/max):\n")
        for r in writers:
            out.write("  0x%06x cat=%-6u key=%-5u aux=%-6u enable=%u "
                      "range=[%u,%u) cond=%s\n" % (
                          r["sourceVa"], r["category"], r["key"], r["aux"],
                          r["enable"], r["min"], r["max"], r["condition"]))
        out.write("\nWriter key histogram:\n")
        for key, count in sorted(writer_keys.items()):
            out.write("  key=%-5u writers=%u\n" % (key, count))
        out.write("\nFUN_00441d90 (mask 8) keys:\n")
        for key, count in sorted(reader8_keys.items()):
            out.write("  key=%-5u calls=%u\n" % (key, count))
        out.write("\nFUN_00441db0 (mask 4) keys:\n")
        for key, count in sorted(reader4_keys.items()):
            out.write("  key=%-5u calls=%u\n" % (key, count))
        out.write("\nDirect FUN_00441c80 calls:\n")
        for r in masks:
            out.write("  0x%06x key=%-5u mask=0x%x\n" % (
                r["sourceVa"], r["key"], r["mask"]))
        out.write("\nWriter/reader key cross-check:\n")
        for name, target in (("mask8", READER8_VA), ("mask4", READER4_VA)):
            reader_keys = {r["key"] for r in readers[target]}
            missing = sorted(reader_keys - set(writer_keys))
            out.write("  %s reader keys=%u missing writers=%u" % (
                name, len(reader_keys), len(missing)))
            if missing:
                out.write(" " + ",".join(str(key) for key in missing))
            out.write("\n")
        direct_keys = {r["key"] for r in masks}
        missing_direct = sorted(direct_keys - set(writer_keys))
        out.write("  direct reader keys=%u missing writers=%u" % (
            len(direct_keys), len(missing_direct)))
        if missing_direct:
            out.write(" " + ",".join(str(key) for key in missing_direct))
        out.write("\n")


def write_header(path, writers, readers, masks):
    with open(path, "w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write("#include \"mmu/core/Types.h\"\n\n")
        out.write("#include <cstddef>\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_save_flags.py. All fields are raw: no\n")
        out.write("// category, key or mask name is inferred from surrounding code.\n")
        out.write("inline constexpr u32 kSaveFlagManagerVa = 0x%06xu;\n" %
                  MANAGER_GLOBAL_VA)
        out.write("inline constexpr u32 kKeyedFlagKeyboardLimit = 0x%03xu;\n" %
                  KEYBOARD_LIMIT)
        out.write("inline constexpr u32 kKeyedFlagManagerByteBias = 0x%03xu;\n" %
                  MANAGER_BYTE_BIAS)
        out.write("struct RecoveredSaveFlagWriter {\n")
        out.write("    u32 sourceVa;\n    u32 category;\n    u32 key;\n")
        out.write("    u32 aux;\n    u8 enable;\n    u32 minRank;\n")
        out.write("    u32 maxRank;\n};\n\n")
        out.write("inline constexpr RecoveredSaveFlagWriter kSaveFlagWriters[] = {\n")
        for r in writers:
            out.write("    {0x%06xu, %uu, %uu, %uu, %uu, %uu, %uu},\n" % (
                r["sourceVa"], r["category"], r["key"], r["aux"],
                r["enable"], r["min"], r["max"]))
        out.write("};\n")
        out.write("inline constexpr std::size_t kSaveFlagWriterCount =\n")
        out.write("    sizeof(kSaveFlagWriters) / sizeof(kSaveFlagWriters[0]);\n\n")
        out.write("inline const RecoveredSaveFlagWriter* findSaveFlagWriter(u32 key) {\n")
        out.write("    for (const auto& row : kSaveFlagWriters)\n")
        out.write("        if (row.key == key) return &row;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")

        for name, target, mask in (("Mask8", READER8_VA, 8),
                                   ("Mask4", READER4_VA, 4)):
            keys = sorted({r["key"] for r in readers[target]})
            out.write("inline constexpr u32 kSaveFlag%sKeys[] = {\n" % name)
            for key in keys:
                out.write("    %uu,\n" % key)
            out.write("};\n")
            out.write("inline constexpr std::size_t kSaveFlag%sKeyCount =\n" % name)
            out.write("    sizeof(kSaveFlag%sKeys) / sizeof(kSaveFlag%sKeys[0]);\n\n" %
                      (name, name))

        out.write("struct RecoveredDirectSaveFlagReader {\n")
        out.write("    u32 sourceVa;\n    u32 key;\n    u32 mask;\n};\n\n")
        out.write("inline constexpr RecoveredDirectSaveFlagReader "
                  "kDirectSaveFlagReaders[] = {\n")
        for r in masks:
            out.write("    {0x%06xu, %uu, 0x%02xu},\n" % (
                r["sourceVa"], r["key"], r["mask"]))
        out.write("};\n")
        out.write("inline constexpr std::size_t kDirectSaveFlagReaderCount =\n")
        out.write("    sizeof(kDirectSaveFlagReaders) / "
                  "sizeof(kDirectSaveFlagReaders[0]);\n")
        out.write("} // namespace mmu\n")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    report = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        root, "docs", "save_flags.txt")
    header = sys.argv[3] if len(sys.argv) > 3 else os.path.join(
        root, "core", "include", "mmu", "core", "SaveFlagTable.h")
    pe = PE(open(sys.argv[1], "rb").read())
    base, data = text_bytes(pe)
    writers = collect_writer_records(base, data)
    readers = collect_reader_keys(base, data)
    masks = collect_direct_reader_masks(base, data)
    write_report(report, writers, readers, masks)
    write_header(header, writers, readers, masks)
    print("writers=%d reader8=%d reader4=%d direct=%d report=%s header=%s" % (
        len(writers), len(readers[READER8_VA]), len(readers[READER4_VA]),
        len(masks), report, header))
    return 0


if __name__ == "__main__":
    sys.exit(main())
