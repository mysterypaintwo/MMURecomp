#!/usr/bin/env python3
"""Recover menu item tables from MMU.exe.

Menu screens do not store their labels inline. Each menu class has a supplier
function that fills a vector of 32-byte records with:

  * a small-string label copied from a static basic_string object, and
  * an action code copied from that object's +0x1c field.

The static objects are initialized by tiny thunks in the .rdata initializer
region: `push <string literal>; mov ecx,<object>; call basic_string_ctor;
mov dword ptr [<object>+0x1c],<action>`. The supplier then writes records with
`push <object>; push <index>; mov ecx,<vector>; call 0x402b30; call 0x402850`.

Recovering both patterns gives the ordered menu labels and their action codes
without guessing from gameplay. `FUN_00503550` shows how a code is used:
0 and the blank spacer label do nothing, 1/0x10 enter Confirm (a follow-up
question), and every other nonzero code posts Confirm + Flash (navigation).

Outputs:
  docs/menu_items.txt
  core/include/mmu/core/MenuItemTables.h
"""

import bisect
import os
import struct
import sys

from analyze_game_flow import cstring, short_class
from analyze_rtti import PE
from analyze_state_graph import collect, text_regions
from disasm import find_vftables

SET_STRING = 0x402850
GET_ITEM = 0x402B30
BASIC_STRING_CTOR_IMPORT = 0x64810C

# These two suppliers are called by the menu manager for the in-game pause
# menu. Its vftable (0x696960) has no recoverable registered states, so the
# constructor/class attribution used for the other menus does not apply.
MANUAL_SUPPLIER_CLASSES = {
    0x5081A0: "MenuInGame",
    0x508340: "MenuInGame",
}

# These constructors write a vftable directly but were not recovered by the
# RTTI constructor scan. Their class names come from the decompilations.
MANUAL_CONSTRUCTORS = {
    0x502820: "MenuInGame",
    0x6182B0: "CharacterSelectMenu",
}


def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]


def call_target(pe, base, data, site):
    off = site - base
    if off < 0 or off >= len(data) - 5 or data[off] != 0xE8:
        return None
    rel = struct.unpack_from("<i", data, off + 1)[0]
    return site + 5 + rel


def refine_function_bounds(base, data, bounds):
    """Split coarse text regions at `ret` followed by a new stack prologue.

    `text_regions` only splits on runs of two or more `int3` padding bytes.
    Some suppliers are immediately preceded by a single `int3`, so their
    writes were being attributed to the preceding large function. Splitting
    on `ret [; padding] push ebp; mov ebp,esp` recovers the real function
    start without guessing from labels.
    """
    refined = []
    for index in range(len(bounds) - 1):
        start, end = bounds[index], bounds[index + 1]
        if not refined or refined[-1] != start:
            refined.append(start)
        for offset in range(start - base, end - base - 4):
            if data[offset] == 0xC3:
                next_offset = offset + 1
            elif data[offset] == 0xC2:  # ret imm16
                next_offset = offset + 3
            else:
                continue
            while next_offset < end - base and data[next_offset] in (0xCC, 0x90):
                next_offset += 1
            if data[next_offset:next_offset + 3] != b"\x55\x8b\xec":
                continue
            if base + next_offset < end and (not refined or
                                              refined[-1] != base + next_offset):
                refined.append(base + next_offset)
    return refined


def function_range(bounds, va):
    index = bisect.bisect_right(bounds, va) - 1
    if index < 0 or index + 1 >= len(bounds):
        return None
    return bounds[index], bounds[index + 1]


def scan_string_initializers(pe, base, data):
    """Map static basic_string object VA -> (label, label VA, action, init VA)."""
    found = {}
    for off in range(len(data) - 24):
        if data[off] != 0x68:  # push <string literal>
            continue
        label_va = u32(data, off + 1)
        if not pe.in_rdata(label_va - pe.image_base):
            continue
        label = cstring(pe, label_va, 0, 80)
        if label is None:
            continue
        if label and (not label.isprintable() or not label.strip()):
            continue
        if data[off + 5] != 0xB9:  # mov ecx, <object>
            continue
        object_va = u32(data, off + 6)
        if data[off + 10:off + 16] != b"\xff\x15\x0c\x81\x64\x00":
            continue  # call dword ptr [0x64810c]
        if data[off + 16] != 0xC7 or data[off + 17] != 0x05:
            continue  # mov dword ptr [<object>+0x1c], <action>
        if u32(data, off + 18) != object_va + 0x1C:
            continue
        found[object_va] = {
            "label": label,
            "labelVa": label_va,
            "action": u32(data, off + 22),
            "initVa": base + off,
        }
    return found


def scan_suppliers(pe, base, data, bounds, strings):
    """Map supplier function VA -> ordered list of (index, static object VA)."""
    found = {}
    for off in range(len(data) - 24):
        if data[off] != 0x68:  # push <static string object>
            continue
        object_va = u32(data, off + 1)
        if object_va not in strings:
            continue
        cursor = off + 5
        if data[cursor] == 0x6A:  # push imm8 index
            index = data[cursor + 1]
            cursor += 2
        elif data[cursor] == 0x68:  # push imm32 index
            index = u32(data, cursor + 1)
            cursor += 5
        else:
            continue
        if data[cursor] != 0xB9:  # mov ecx, <vector>
            continue
        vector_va = u32(data, cursor + 1)
        cursor += 5
        if cursor + 5 > len(data) or data[cursor] != 0xE8:
            continue
        if call_target(pe, base, data, base + cursor) != GET_ITEM:
            continue
        cursor += 5
        if data[cursor:cursor + 2] != b"\x8b\xc8" or data[cursor + 2] != 0xE8:
            continue  # mov ecx,eax; call 0x402850
        if call_target(pe, base, data, base + cursor + 2) != SET_STRING:
            continue
        region = function_range(bounds, base + off)
        if region is None:
            continue
        fn = region[0]
        row = found.setdefault(fn, {"vector": vector_va, "items": [], "sites": []})
        row["items"].append((index, object_va))
        row["sites"].append(base + off)
    for row in found.values():
        row["items"].sort()
    return found


def map_supplier_classes(pe, base, data, bounds, suppliers, classes):
    """Attribute supplier functions to menu classes using binary evidence."""
    ctor_class = {cls["constructor"]: cls["class"] for cls in classes}
    ctor_class.update(MANUAL_CONSTRUCTORS)
    function_class = {}
    for cls, entries in find_vftables(pe).items():
        simple = short_class(cls)
        for _vft, funcs in entries:
            for func in funcs:
                function_class.setdefault(func, simple)

    result = {}
    for supplier, row in suppliers.items():
        names = set()
        # The supplier itself only fills the vector. Attribution comes from
        # its callers: either a menu constructor calls it directly, or a menu
        # manager calls it, pushes the vector, then calls the constructor.
        call_sites = []
        for off in range(len(data) - 5):
            if data[off] != 0xE8:
                continue
            if call_target(pe, base, data, base + off) == supplier:
                call_sites.append(base + off)
        for site in call_sites:
            region = function_range(bounds, site)
            if region and region[0] in ctor_class:
                names.add(ctor_class[region[0]])
            # A menu manager may call the supplier, push its result, then call
            # the menu constructor a few instructions later.
            for cursor in range(site + 5, min(site + 40, len(data) + base) - 5):
                off = cursor - base
                if off < 0 or off >= len(data) - 5:
                    break
                if data[off] == 0xE8:
                    break
                if data[off] != 0x50:  # push eax
                    continue
                for call_off in range(off + 1, min(off + 16, len(data) - 5)):
                    target = call_target(pe, base, data, base + call_off)
                    if target in ctor_class:
                        names.add(ctor_class[target])
                        break
            if names:
                break
        if not names and supplier in function_class:
            names.add(function_class[supplier])
        if not names and supplier in MANUAL_SUPPLIER_CLASSES:
            names.add(MANUAL_SUPPLIER_CLASSES[supplier])
        result[supplier] = sorted(names)
    return result


def behavior_name(action):
    if action == 0:
        return "spacer"
    if action in (0x01, 0x10):
        return "confirm"
    return "flash"


def display_label(label):
    # Menu labels use leading/trailing font control bytes ($ and ^) and
    # padding spaces. Keep the exact raw string in the table; this is only a
    # readable rendering for reports and the debug console.
    return label.strip().strip("$^").strip()


def scan_slot_counts(root):
    """Read vector sizes from the Ghidra decompilations of supplier functions."""
    counts = {}
    decompiled = os.path.join(root, "docs", "decompiled")
    if not os.path.isdir(decompiled):
        return counts
    import re
    for path in os.listdir(decompiled):
        if not path.endswith(".c"):
            continue
        text = open(os.path.join(decompiled, path), encoding="utf-8",
                    errors="replace").read()
        header = re.search(r"// FUN_[0-9A-Fa-f]+ @ 0x([0-9a-fA-F]+)", text)
        size = re.search(r"FUN_00402720\((\d+),", text)
        if header and size:
            counts[int(header.group(1), 16)] = int(size.group(1))
    return counts


def collect_rows(pe, root):
    base, data, bounds = text_regions(pe)
    bounds = refine_function_bounds(base, data, bounds)
    strings = scan_string_initializers(pe, base, data)
    suppliers = scan_suppliers(pe, base, data, bounds, strings)
    classes = collect(pe)
    attribution = map_supplier_classes(pe, base, data, bounds, suppliers, classes)
    slot_counts = scan_slot_counts(root)

    rows = []
    for supplier in sorted(suppliers):
        supplier_row = suppliers[supplier]
        names = attribution.get(supplier) or ["Unknown"]
        explicit = dict(supplier_row["items"])
        # The supplier helper initializes a fixed-size vector of blank labels
        # and only overwrites the enabled rows. The MainMenu loop in 0x503550
        # skips blanks, so the blank slots are real selectable-array entries.
        count = slot_counts.get(supplier, 0)
        if not count and names and names[0] == "MainMenu":
            count = 10
        for index in range(count):
            object_va = explicit.get(index)
            if object_va is None:
                rows.append({
                    "class": names[0],
                    "allClasses": names,
                    "supplier": supplier,
                    "vector": supplier_row["vector"],
                    "index": index,
                    "label": "",
                    "display": "",
                    "labelVa": 0,
                    "objectVa": 0,
                    "initVa": 0,
                    "action": 0,
                    "behavior": "spacer",
                    "implicit": True,
                })
                continue
            string = strings[object_va]
            rows.append({
                "class": names[0],
                "allClasses": names,
                "supplier": supplier,
                "vector": supplier_row["vector"],
                "index": index,
                "label": string["label"],
                "display": display_label(string["label"]),
                "labelVa": string["labelVa"],
                "objectVa": object_va,
                "initVa": string["initVa"],
                "action": string["action"],
                "behavior": behavior_name(string["action"]),
                "implicit": False,
            })
        for index, object_va in supplier_row["items"]:
            if index in [row["index"] for row in rows if row["supplier"] == supplier and row["class"] == names[0]]:
                continue
            # Suppliers without a recovered slot count keep all explicit rows.
            string = strings[object_va]
            rows.append({
                "class": names[0],
                "allClasses": names,
                "supplier": supplier,
                "vector": supplier_row["vector"],
                "index": index,
                "label": string["label"],
                "display": display_label(string["label"]),
                "labelVa": string["labelVa"],
                "objectVa": object_va,
                "initVa": string["initVa"],
                "action": string["action"],
                "behavior": behavior_name(string["action"]),
                "implicit": False,
            })
    rows.sort(key=lambda row: (row["class"], row["supplier"], row["index"]))
    return rows, suppliers, attribution


def write_report(rows, suppliers, attribution, path):
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_menu_items.py\n")
        out.write("Source: static string initializers and 0x402b30/0x402850 menu\n")
        out.write("supplier patterns in MMU.exe. Labels and action codes are exact;\n")
        out.write("behavior names come from the branch at 0x503550.\n")
        out.write("suppliers=%d items=%d classes=%d\n\n" % (
            len(suppliers), len(rows),
            len({row["class"] for row in rows})))
        current = None
        for row in rows:
            key = (row["class"], row["supplier"])
            if key != current:
                current = key
                aliases = ", ".join(attribution.get(row["supplier"], [row["class"]]))
                out.write("=== %s supplier=0x%x vector=0x%x classes=%s ===\n" % (
                    row["class"], row["supplier"], row["vector"], aliases))
            out.write("  %2d action=0x%02x %-8s %-24s labelVa=0x%x object=0x%x%s\n" % (
                row["index"], row["action"], row["behavior"],
                repr(row["label"]), row["labelVa"], row["objectVa"],
                " implicit-blank" if row.get("implicit") else ""))
            out.write("     display=%s\n" % repr(row["display"]))
        out.write("\nControl codes: leading/trailing $ and ^ bytes in labels are font\n")
        out.write("commands; the raw string is preserved above.\n")


def write_header(rows, path):
    with open(path, "w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write("#include \"mmu/core/Types.h\"\n\n")
        out.write("#include <cstddef>\n")
        out.write("#include <string_view>\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_menu_items.py. The action code is copied\n")
        out.write("// verbatim from the static menu string object at +0x1c. Behavior is\n")
        out.write("// derived from the branch in FUN_00503550, not from gameplay guesses.\n")
        out.write("enum class MenuItemBehavior : u8 {\n")
        out.write("    Spacer,\n")
        out.write("    Confirm,\n")
        out.write("    Flash,\n")
        out.write("};\n\n")
        out.write("struct RecoveredMenuItem {\n")
        out.write("    std::string_view className;\n")
        out.write("    std::string_view label;\n")
        out.write("    u32 actionCode;\n")
        out.write("    u32 supplierVa;\n")
        out.write("    u32 labelVa;\n")
        out.write("    u16 index;\n")
        out.write("    u8 behavior;\n")
        out.write("    u8 reserved;\n")
        out.write("};\n\n")
        out.write("inline constexpr RecoveredMenuItem kRecoveredMenuItems[] = {\n")
        for row in rows:
            behavior = {"spacer": 0, "confirm": 1, "flash": 2}[row["behavior"]]
            out.write("    {\"%s\", \"%s\", 0x%02xu, 0x%06xu, 0x%06xu, %du, %du, 0u},\n" % (
                row["class"], escape(row["label"]), row["action"],
                row["supplier"], row["labelVa"], row["index"], behavior))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kRecoveredMenuItemsCount =\n")
        out.write("    sizeof(kRecoveredMenuItems) / sizeof(kRecoveredMenuItems[0]);\n\n")
        out.write("inline const char* menuItemBehaviorName(u8 behavior) {\n")
        out.write("    switch (static_cast<MenuItemBehavior>(behavior)) {\n")
        out.write("    case MenuItemBehavior::Spacer: return \"spacer\";\n")
        out.write("    case MenuItemBehavior::Confirm: return \"confirm\";\n")
        out.write("    case MenuItemBehavior::Flash: return \"flash\";\n")
        out.write("    }\n")
        out.write("    return \"?\";\n")
        out.write("}\n\n")
        out.write("inline std::size_t menuItemCount(std::string_view className,\n")
        out.write("                                  u32 supplierVa = 0) {\n")
        out.write("    std::size_t count = 0;\n")
        out.write("    for (const RecoveredMenuItem& item : kRecoveredMenuItems) {\n")
        out.write("        if (item.className != className) continue;\n")
        out.write("        if (supplierVa != 0 && item.supplierVa != supplierVa) continue;\n")
        out.write("        ++count;\n")
        out.write("    }\n")
        out.write("    return count;\n")
        out.write("}\n\n")
        out.write("inline const RecoveredMenuItem* findMenuItem(std::string_view className,\n")
        out.write("                                               u16 index,\n")
        out.write("                                               u32 supplierVa = 0) {\n")
        out.write("    for (const RecoveredMenuItem& item : kRecoveredMenuItems) {\n")
        out.write("        if (item.className != className || item.index != index) continue;\n")
        out.write("        if (supplierVa != 0 && item.supplierVa != supplierVa) continue;\n")
        out.write("        return &item;\n")
        out.write("    }\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline bool hasMenuItemLabel(std::string_view label) {\n")
        out.write("    for (const RecoveredMenuItem& item : kRecoveredMenuItems)\n")
        out.write("        if (item.label == label) return true;\n")
        out.write("    return false;\n")
        out.write("}\n\n")
        out.write("} // namespace mmu\n")


def escape(text):
    out = []
    for ch in text:
        if ch == "\\":
            out.append("\\\\")
        elif ch == "\"":
            out.append("\\\"")
        elif ord(ch) < 0x20 or ord(ch) > 0x7E:
            out.append("\\x%02x" % ord(ch))
        else:
            out.append(ch)
    return "".join(out)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    report = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        root, "docs", "menu_items.txt")
    header = sys.argv[3] if len(sys.argv) > 3 else os.path.join(
        root, "core", "include", "mmu", "core", "MenuItemTables.h")
    pe = PE(open(sys.argv[1], "rb").read())
    rows, suppliers, attribution = collect_rows(pe, root)
    write_report(rows, suppliers, attribution, report)
    write_header(rows, header)
    print("suppliers=%d items=%d classes=%d report=%s header=%s" % (
        len(suppliers), len(rows), len({row["class"] for row in rows}),
        report, header))
    return 0


if __name__ == "__main__":
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    raise SystemExit(main())
