#!/usr/bin/env python3
"""Recover the raw boss-meter depletion branch from MMU.exe.

FUN_00420380 and FUN_00420690 are the two helpers that subtract a computed
amount from a small integer meter (this+0x150 / this+0x160). When the meter
reaches zero they read GameParam key 0xa96bff8d through FUN_0043c800 and
compare the returned float with the double constant at 0x697ce8 (1.0). The x87
condition is an equality test: only `value == 1.0` selects return code 4, which
FUN_00420450 turns into a spawn plus FUN_004047a0(1).

The decoded guns.mmu value for this key is read back from the asset instead of
being assumed. No gameplay label is assigned to the key, the meter, or the
result codes.

An owner is first taken from the nearest preceding vftable store. When that is
absent, the enclosing handler is resolved only if exactly one RTTI vftable
contains it; this recovers two further owners without guessing between shared
base-class handlers. The raw resolution source is recorded per caller.

Outputs:
  docs/boss_damage.txt
  core/include/mmu/core/BossDamageParams.h
"""

import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from analyze_game_flow import short_class
from analyze_rtti import PE
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from disasm import find_vftables
from extract_gun_params import load_guns

KEY = 0xA96BFF8D
GET_PARAM = 0x43C800
COMPARE_VA = 0x697CE8
EXPECTED_VALUE = 1.0
DEPLETION_CODE = 4

MEM_RE = re.compile(r"\[e(?:ax|cx|dx|bx|si|di) \+ 0x([0-9a-f]+)\]")
CMP_CODE_RE = re.compile(r"dword ptr \[ebp - 4\], (0x[0-9a-f]+|\d+)")
VFTABLE_RE = re.compile(r"\bVA (0x[0-9a-f]+)")


def text_range(pe):
    lo = pe.rva_to_off(pe.text_lo)
    hi = lo + next(raw for name, _, _, _, raw in pe.sections if name == ".text")
    return lo, hi


def key_sites(pe, data, lo, hi):
    needle = struct.pack("<I", KEY)
    sites = []
    pos = lo
    while True:
        pos = data.find(needle, pos, hi)
        if pos < 0:
            break
        va = pe.image_base + pe.off_to_rva(pos)
        sites.append(va)
        pos += 1
    return sites


def enclosing_prologue(pe, data, va):
    off = pe.rva_to_off(va - pe.image_base)
    for pos in range(off, max(0, off - 512), -1):
        if data[pos] == 0x55 and data[pos + 1] == 0x8B and data[pos + 2] == 0xEC:
            return pe.image_base + pe.off_to_rva(pos)
    raise RuntimeError("no prologue before 0x%x" % va)


def disasm_function(pe, data, start):
    off = pe.rva_to_off(start - pe.image_base)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    rows = []
    for ins in md.disasm(data[off:off + 256], start):
        rows.append((ins.address, ins.mnemonic, ins.op_str.strip()))
        if ins.mnemonic == "ret":
            break
    return rows


def callers(pe, data, lo, hi, target):
    found = []
    for pos in range(lo, hi - 5):
        if data[pos] != 0xE8:
            continue
        rel = struct.unpack_from("<i", data, pos + 1)[0]
        va = pe.image_base + pe.off_to_rva(pos)
        if (va + 5 + rel) & 0xFFFFFFFF == target:
            found.append(va)
    return found


def load_vtables(root):
    text = open(os.path.join(root, "docs", "vtables.txt"),
                encoding="utf-8", errors="replace").read()
    result = {}
    for line in text.splitlines():
        match = VFTABLE_RE.search(line)
        if match and line.strip():
            va = int(match.group(1), 16)
            name = line.split()[0].replace(".?AV", "").split("@@")[0]
            result[va] = name
    return result


def vftable_store(data, lo, off):
    for pos in range(off, max(lo, off - 0x800), -1):
        if data[pos] != 0xC7:
            continue
        modrm = data[pos + 1]
        if modrm & 0xC0 or (modrm & 0x38) or modrm == 0x04:
            continue
        return struct.unpack_from("<I", data, pos + 2)[0]
    return None


def vftable_index(pe):
    """Map each handler VA to its RTTI vftable memberships."""
    result = {}
    for class_name, entries in find_vftables(pe).items():
        name = short_class(class_name)
        for vft, funcs in entries:
            for slot, func in enumerate(funcs):
                result.setdefault(func, []).append(
                    (pe.image_base + vft, name, slot))
    return result


def caller_facts(pe, data, lo, vtables, vft_index, va):
    handler = enclosing_prologue(pe, data, va)
    off = pe.rva_to_off(va - pe.image_base)
    vft = vftable_store(data, lo, off)
    resolution = ""
    slot = None
    owner = ""
    if vft in vtables:
        owner = vtables[vft]
        resolution = "vftable-store"
    else:
        vft = None
        members = vft_index.get(handler, [])
        if len(members) == 1:
            vft, owner, slot = members[0]
            resolution = "unique-vftable-member"
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    codes = []
    for ins in md.disasm(data[off:off + 96], va):
        match = CMP_CODE_RE.search(ins.op_str)
        if ins.mnemonic == "cmp" and match:
            code = int(match.group(1), 0)
            if code not in codes:
                codes.append(code)
        if ins.address >= va + 64:
            break
    return {
        "va": va,
        "handler": handler,
        "vft": vft,
        "owner": owner,
        "vtableSlot": slot,
        "resolution": resolution,
        "codes": codes,
    }


def analyze(exe_path, guns_path, root):
    data = open(exe_path, "rb").read()
    pe = PE(data)
    lo, hi = text_range(pe)
    params = load_guns(exe_path, guns_path)
    values = params.get(KEY, [])
    vtables = load_vtables(root)
    handlers_by_vft = vftable_index(pe)

    helpers = []
    for site in key_sites(pe, data, lo, hi):
        start = enclosing_prologue(pe, data, site)
        rows = disasm_function(pe, data, start)
        meter = None
        for va, mnemonic, operands in rows:
            if mnemonic == "cmp" and "ebp" not in operands:
                for match in MEM_RE.finditer(operands):
                    meter = int(match.group(1), 16)
            if mnemonic == "mov" and operands.startswith("eax, dword ptr ["):
                for match in MEM_RE.finditer(operands):
                    meter = int(match.group(1), 16)
        calls = [int(operands.split()[0], 16)
                 for va, mnemonic, operands in rows if mnemonic == "call"]
        codes = []
        for va, mnemonic, operands in rows:
            match = re.fullmatch(r"(?:eax|dword ptr \[ebp - 8\]), "
                                 r"(0x[0-9a-f]+|\d+)", operands)
            if mnemonic == "mov" and match:
                code = int(match.group(1), 0)
                if code not in codes:
                    codes.append(code)
        compare = [operands for va, mnemonic, operands in rows
                   if mnemonic == "fcomp"]
        caller_vas = callers(pe, data, lo, hi, start)
        helpers.append({
            "start": start,
            "site": site,
            "meter": meter,
            "rows": rows,
            "calls": calls,
            "codes": codes,
            "compare": compare,
            "callers": [caller_facts(pe, data, lo, vtables,
                                     handlers_by_vft, va)
                        for va in caller_vas],
        })
    if not helpers:
        raise RuntimeError("no GameParam key sites found")
    return helpers, values


def write_report(path, helpers, values):
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_boss_damage.py\n")
        out.write("GameParam key 0x%08x; decoded guns.mmu entries=%d\n" %
                  (KEY, len(values)))
        for obj, value in values:
            out.write("  obj=0x%08x v=%g\n" % (obj, value))
        out.write("FUN_0043c800 getter return compared with double at "
                  "0x%06x = %g via fcomp/fnstsw/test ah,0x44/jp; equality "
                  "selects code %d.\n\n" %
                  (COMPARE_VA, EXPECTED_VALUE, DEPLETION_CODE))
        for helper in helpers:
            out.write("FUN_0x%06x key site 0x%06x meter=0x%x codes=%s "
                      "callers=%d\n" %
                      (helper["start"], helper["site"], helper["meter"],
                       ",".join(str(code) for code in helper["codes"]),
                       len(helper["callers"])))
            out.write("  helper calls: %s\n" %
                      ", ".join("0x%x" % call for call in helper["calls"]))
            out.write("  fcomp: %s\n" % (helper["compare"] or "none"))
            out.write("  caller map (call VA -> enclosing handler, owner "
                      "vftable, compared result codes):\n")
            for fact in helper["callers"]:
                out.write("    0x%06x -> 0x%06x vft=%s owner=%-18s "
                          "slot=%-4s source=%-21s codes=%s\n" %
                          (fact["va"], fact["handler"],
                           ("0x%06x" % fact["vft"]) if fact["vft"] else "-",
                           fact["owner"] or "-",
                           fact["vtableSlot"] if fact["vtableSlot"] is not None
                           else "-",
                           fact["resolution"] or "-",
                           ",".join(str(code) for code in fact["codes"]) or "-"))
            for va, mnemonic, operands in helper["rows"]:
                out.write("  0x%06x %-6s %s\n" % (va, mnemonic, operands))
            out.write("\n")
        out.write("FUN_00420450 result dispatch (verbatim decompiled calls):\n")
        out.write("  3 -> FUN_005b51f0(param_1)\n")
        out.write("  0 -> (*param_1->vtable[0x54/4])(); FUN_005b6520(param_1, ret)\n")
        out.write("  4 -> FUN_00401930(0x18,..); FUN_005629b0(..); FUN_004047a0(1)\n")
        out.write("  2 -> *param_3 = 0; FUN_0043c800(); FUN_006253a0();\n"
                  "       (*param_1->vtable[0xc/4])(3,piVar1); "
                  "(*piVar1->vtable[0xc/4])(0x1a,0)\n")


def write_header(path, helpers, values):
    meters = [helper["meter"] for helper in helpers if helper["meter"] is not None]
    default = values[0][1] if len(values) == 1 else EXPECTED_VALUE
    lines = [
        "#pragma once",
        "",
        "#include \"mmu/core/Types.h\"",
        "",
        "#include <string_view>",
        "",
        "namespace mmu {",
        "",
        "// Generated by tools/analyze_boss_damage.py from MMU.exe and guns.mmu.",
        "// These are raw binary constants; no gameplay label is assigned.",
        "inline constexpr u32 kBossDamageParamKey = 0xa96bff8du;",
        "inline constexpr u32 kBossDamageGetterVa = 0x43c800u;",
        "inline constexpr u32 kBossDamageCompareVa = 0x697ce8u;",
        "inline constexpr float kBossDamageParamDefault = %r;" % float(default),
        "inline constexpr int kBossDamageDepletionCode = %d;" % DEPLETION_CODE,
    ]
    for index, meter in enumerate(meters):
        lines.append("inline constexpr u32 kBossDamageMeterOffset%d = 0x%x;" %
                     (index, meter))
    for index, helper in enumerate(helpers):
        lines.append("inline constexpr u32 kBossDamageHelperVa%d = 0x%06x;" %
                     (index, helper["start"]))
    lines.extend([
        "",
        "// Each entry is one direct call to a helper above. resolution records",
        "// the raw owner evidence: nearest known vftable store, or a unique RTTI",
        "// vftable membership when no store was found. Empty when unresolved.",
        "// resultCodeBits has bit N set when `cmp dword ptr [ebp-4], N` appears",
        "// within 64 bytes after the call; no code meaning is assigned.",
        "struct BossDamageCaller {",
        "    std::string_view owner;",
        "    u32 callVa;",
        "    u32 handlerVa;",
        "    u32 vftableVa;",
        "    u16 vtableSlot;",
        "    u32 resultCodeBits;",
        "    std::string_view resolution;",
        "};",
        "",
        "inline constexpr BossDamageCaller kBossDamageCallers[] = {",
    ])
    for helper in helpers:
        for fact in helper["callers"]:
            bits = 0
            for code in fact["codes"]:
                if 0 <= code < 32:
                    bits |= 1 << code
            lines.append(
                "    {\"%s\", 0x%06xu, 0x%06xu, 0x%06xu, %du, 0x%08xu, \"%s\"}," %
                (fact["owner"], fact["va"], fact["handler"],
                 fact["vft"] or 0, fact["vtableSlot"] or 0, bits,
                 fact["resolution"]))
    lines.extend([
        "};",
        "",
        "constexpr bool bossDamageDepletionMatches(float paramValue) {",
        "    return paramValue == kBossDamageParamDefault;",
        "}",
        "",
        "} // namespace mmu",
        "",
    ])
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def main():
    if len(sys.argv) != 6:
        print(__doc__)
        return 2
    exe, guns, root, report, header = sys.argv[1:]
    helpers, values = analyze(exe, guns, root)
    write_report(report, helpers, values)
    write_header(header, helpers, values)
    print("helpers=%d meters=%s key_entries=%d default=%g callers=%d" %
          (len(helpers),
           ",".join("0x%x" % h["meter"] for h in helpers if h["meter"]),
           len(values), values[0][1] if values else -1,
           sum(len(h["callers"]) for h in helpers)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
