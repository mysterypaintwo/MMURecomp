#!/usr/bin/env python3
"""Recover terminal destinations of menu action codes from MMU.exe.

`FUN_00503550` copies the +0x1c action code of the selected menu row into the
menu object, and `FUN_00503060` forwards it to the global menu manager at
0x6c55a0. The manager's action dispatcher is `FUN_0050f7d0`; its switch on
`param_2` constructs the next screen or performs a terminal effect.

This generator reads the Ghidra decompilation of that dispatcher and the RTTI
constructor table produced by `tools/analyze_state_graph.py`. It does not infer
destinations from label text.

Outputs:
  docs/menu_actions.txt
  core/include/mmu/core/MenuActionTable.h
"""

import os
import re
import sys

from analyze_game_flow import short_class
from analyze_rtti import PE
from analyze_state_graph import collect

DISPATCHER = 0x50F7D0

# These two functions write a vftable directly but were not recovered by the
# RTTI constructor scan. The vftable names are printed by their Ghidra
# decompilations (docs/decompiled/0x502820.c and 0x6182b0.c).
MANUAL_CONSTRUCTORS = {
    0x502820: "MenuInGame",
    0x6182B0: "CharacterSelectMenu",
}
KIND_SCREEN = "Screen"
KIND_CONTINUE = "ContinueGame"
KIND_RESUME = "ResumeGame"
KIND_EXIT_STAGE = "ExitStage"
KIND_EXIT_GAME = "ExitGame"
KIND_MANAGER_PHASE = "ManagerPhase"
KIND_CUTSCENE = "Cutscene"
KIND_CHARACTER_SELECTED = "CharacterSelected"
KIND_NONE = "None"

# These rows are handled by CharacterSelectMenu's own timer callback at
# 0x617e10, not by the global action dispatcher. The decompiled CALL order is
# 0x61f5e0(U), 0x61dc80(U, character), 0x402790(M, 5). Action 0x41 there skips
# 0x61f5e0 and uses 0x402790(M, 2).
LOCAL_ACTIONS = {
    0x48: {
        "kind": KIND_CHARACTER_SELECTED,
        "note": ("CharacterSelectMenu timer 0x617e10: 0x61f5e0(U), "
                 "0x61dc80(U,0), 0x402790(M,5)"),
    },
    0x49: {
        "kind": KIND_CHARACTER_SELECTED,
        "note": ("CharacterSelectMenu timer 0x617e10: 0x61f5e0(U), "
                 "0x61dc80(U,1), 0x402790(M,5)"),
    },
}


def cpp_literal(text):
    """Escape one runtime string for a generated C++ string literal."""
    return text.replace("\\", "\\\\").replace('"', '\\"')


def parse_action(text):
    value = text.strip()
    return int(value, 16) if value.lower().startswith("0x") else int(value, 10)


def parse_screen_classes(root):
    path = os.path.join(root, "core", "include", "mmu", "core", "GameFlow.h")
    text = open(path, encoding="utf-8").read()
    result = {}
    for enum_name, class_name in re.findall(
            r"\{FlowScreen::(\w+),\s*\"([^\"]+)\"", text):
        result[class_name] = enum_name
    return result


def parse_supplier_classes(root):
    """Map lazy menu-item vector initializers to classes from the item report."""
    path = os.path.join(root, "docs", "menu_items.txt")
    result = {}
    if not os.path.exists(path):
        return result
    for line in open(path, encoding="utf-8", errors="replace"):
        match = re.match(r"^=== (\w+) supplier=0x([0-9a-fA-F]+)", line)
        if match:
            result[int(match.group(2), 16)] = match.group(1)
    return result


def parse_supplier_names(root):
    """Read the name string passed to each supplier's fixed vector allocator."""
    result = {}
    decompiled = os.path.join(root, "docs", "decompiled")
    if not os.path.isdir(decompiled):
        return result
    for path in os.listdir(decompiled):
        if not path.endswith(".c"):
            continue
        text = open(os.path.join(decompiled, path), encoding="utf-8",
                    errors="replace").read()
        header = re.search(r"// FUN_[0-9A-Fa-f]+ @ 0x([0-9a-fA-F]+)", text)
        name = re.search(r'basic_string<char[^\n]*?>\s*\(\s*local_2c,\s*"([^"]*)"\)',
                         text)
        if header and name:
            result[int(header.group(1), 16)] = name.group(1).strip()
    return result


def parse_case_blocks(text):
    """Return action -> decompiled source text for each switch case."""
    start = text.find("switch(param_2)")
    if start < 0:
        return {}
    matches = list(re.finditer(r"^\s*case\s+([^\s:]+)\s*:", text[start:], re.M))
    blocks = {}
    for i, match in enumerate(matches):
        begin = start + match.end()
        end = start + matches[i + 1].start() if i + 1 < len(matches) else len(text)
        blocks[parse_action(match.group(1))] = text[begin:end]
    return blocks


def parse_pre_switch(text):
    """Recover the two actions handled before the switch."""
    effects = {}
    if re.search(r"if\s*\(param_2 == 0xe\)", text):
        effects[0x0E] = KIND_RESUME
    if re.search(r"else if\s*\(param_2 == 0xf\)", text):
        effects[0x0F] = KIND_EXIT_STAGE
    return effects


def parse_cutscene(block):
    """Return the literal .xcut path built by a dispatcher case.

    The decompiled C source escapes each backslash as ``\\\\``. Convert those
    to single backslashes so the value is the real runtime path; the header
    writer re-escapes it for the generated C++ literal.
    """
    match = re.search(r'"([^"]+\.xcut)"', block)
    return match.group(1).replace("\\\\", "\\") if match else ""


def find_evidence_line(text, action, constructor_vas):
    lines = text.splitlines()
    pattern = re.compile(r"^\s*case\s+(0x%x|%d)\s*:" % (action, action))
    for i, line in enumerate(lines):
        if pattern.match(line):
            for j in range(i, min(i + 80, len(lines))):
                for va in constructor_vas:
                    if ("FUN_%08x" % va) in lines[j]:
                        return j + 1
            return i + 1
    return 0


def collect_actions(pe, root):
    decompiled = os.path.join(root, "docs", "decompiled", "0x%06x.c" % DISPATCHER)
    if not os.path.exists(decompiled):
        raise SystemExit("missing %s; run the Ghidra decompile target first" % decompiled)
    text = open(decompiled, encoding="utf-8", errors="replace").read()

    constructor_class = {}
    for cls in collect(pe):
        constructor_class[cls["constructor"]] = short_class(cls["class"])
    constructor_class.update(MANUAL_CONSTRUCTORS)
    screen_classes = parse_screen_classes(root)
    supplier_classes = parse_supplier_classes(root)
    supplier_names = parse_supplier_names(root)
    blocks = parse_case_blocks(text)
    pre_effects = parse_pre_switch(text)

    rows = []
    for action in sorted(set(blocks) | set(pre_effects) | set(LOCAL_ACTIONS)):
        if action in LOCAL_ACTIONS and action not in blocks:
            local = LOCAL_ACTIONS[action]
            rows.append({
                "action": action,
                "kind": local["kind"],
                "screens": [],
                "constructors": [],
                "variants": [],
                "cutscene": "",
                "conditionalCutscenes": [],
                "note": local["note"],
                "evidenceLine": 0,
            })
            continue
        block = blocks.get(action, "")
        calls = [int(va, 16) for va in re.findall(
            r"FUN_([0-9A-Fa-f]{6,8})\s*\(", block)]
        screens = []
        constructors = []
        variants = []
        conditional = []
        for va in calls:
            if supplier_classes.get(va) == "MainMenu" and va not in variants:
                variants.append(va)
        for va in calls:
            cls = constructor_class.get(va)
            if not cls:
                continue
            screen = screen_classes.get(cls)
            if screen and screen not in screens:
                screens.append(screen)
            constructors.append((va, cls, screen))

        if action in pre_effects:
            kind = pre_effects[action]
            note = "handled before the switch in FUN_0050f7d0"
        elif "FUN_0050e610" in block:
            kind, note = KIND_CONTINUE, "loads saved stage through FUN_0050e610"
        elif action == 0x10:
            kind, note = KIND_EXIT_GAME, "sets manager byte +0x5d (quit flag) at 0x5100a1"
        elif action == 0x41:
            kind, note = (KIND_MANAGER_PHASE,
                          "sets manager phase through 0x402790(param_1[0xd]); "
                          "CharacterSelectMenu slot 2 uses literal phase 2")
        elif "CSN" in block and not screens:
            path = parse_cutscene(block)
            kind, note = KIND_CUTSCENE, "plays a cutscene path in FUN_0050f7d0"
            if path:
                note += ": " + path
        elif screens:
            kind = KIND_SCREEN
            note = ", ".join("%s via 0x%x" % (cls or "?", va)
                             for va, cls, _screen in constructors)
            if len(screens) > 1:
                note += " (conditional)"
            if re.search(r"FUN_00504ce0[^\n]*,\s*0\)", block):
                note += "; save flag 0"
            if re.search(r"FUN_00504ce0[^\n]*,\s*1\)", block):
                note += "; save/data mode (param3=1, SaveMenu-BG)"
            # Some screen cases also play a cutscene under an extra predicate
            # (action 0x06 in 4-boss/8-boss modes). Record it as evidence but
            # do not attach it as the unconditional event cutscene path.
            conditional = []
            if "CSN" in block:
                conditional = [parse_cutscene('"%s"' % path)
                               for path in re.findall(r'"([^"]+\.xcut)"', block)]
                conditional = [path for path in conditional if path]
                note += "; conditional cutscene(s): " + ", ".join(conditional)
        else:
            kind, note = KIND_NONE, "no screen or terminal effect recovered"

        if variants:
            detail = ", ".join(
                "0x%x '%s'" % (va, supplier_names.get(va, ""))
                for va in variants)
            note += "; item table " + detail
            if len(variants) > 1 and "FUN_00403d40" in block:
                note += " selected by manager byte +0x59"

        rows.append({
            "action": action,
            "kind": kind,
            "screens": screens,
            "constructors": constructors,
            "variants": variants,
            "cutscene": parse_cutscene(block) if kind == KIND_CUTSCENE else "",
            "conditionalCutscenes": conditional if kind == KIND_SCREEN else [],
            "note": note,
            "evidenceLine": find_evidence_line(text, action, [va for va, _, _ in constructors]),
        })
    return rows


def write_report(rows, path):
    with open(path, "w", encoding="utf-8") as out:
        out.write("Generated by tools/analyze_menu_actions.py\n")
        out.write("Source: switch(param_2) in docs/decompiled/0x050f7d0.c plus\n")
        out.write("RTTI constructor names from MMU.exe. Labels are not used to\n")
        out.write("infer destinations.\n")
        out.write("actions=%d screenActions=%d\n\n" % (
            len(rows), sum(row["kind"] == KIND_SCREEN for row in rows)))
        for row in rows:
            out.write("action=0x%02x kind=%-11s screens=%-40s items=%-22s cutscene=%-48s line=%d\n" % (
                row["action"], row["kind"],
                ",".join(row["screens"]) or "-",
                ",".join("0x%x" % va for va in row["variants"]) or "-",
                row["cutscene"] or "-",
                row["evidenceLine"]))
            out.write("    %s\n" % row["note"])
            if row.get("conditionalCutscenes"):
                out.write("    conditional=%s\n" %
                          ", ".join(row["conditionalCutscenes"]))


def write_header(rows, path):
    with open(path, "w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write("#include \"mmu/core/GameFlow.h\"\n")
        out.write("#include \"mmu/core/Types.h\"\n\n")
        out.write("namespace mmu {\n\n")
        out.write("// Generated by tools/analyze_menu_actions.py. Screens come from\n")
        out.write("// the action dispatcher at 0x50f7d0 and RTTI constructor names;\n")
        out.write("// menu labels are not used as evidence.\n")
        out.write("enum class MenuActionKind : u8 {\n")
        out.write("    None, Screen, ContinueGame, ResumeGame, ExitStage, ExitGame,\n")
        out.write("    ManagerPhase, Cutscene, CharacterSelected,\n")
        out.write("};\n\n")
        out.write("struct RecoveredMenuAction {\n")
        out.write("    u32 actionCode;\n")
        out.write("    MenuActionKind kind;\n")
        out.write("    u8 screenCount;\n")
        out.write("    u8 variantCount;\n")
        out.write("    FlowScreen screens[2];\n")
        out.write("    u32 variantSuppliers[2];\n")
        out.write("    const char* cutscenePath;\n")
        out.write("    const char* note;\n")
        out.write("    const char* conditionalCutscenes[2];\n")
        out.write("    u8 conditionalCutsceneCount;\n")
        out.write("};\n\n")
        out.write("inline constexpr RecoveredMenuAction kRecoveredMenuActions[] = {\n")
        for row in rows:
            screens = row["screens"][:2]
            while len(screens) < 2:
                screens.append(screens[0] if screens else "TitleMenu")
            entries = ", ".join("FlowScreen::%s" % screen for screen in screens)
            variants = row["variants"][:2]
            while len(variants) < 2:
                variants.append(0)
            supplier_entries = ", ".join("0x%02xu" % va for va in variants)
            note = cpp_literal(row["note"])
            cutscene = cpp_literal(row["cutscene"])
            conditional = [cpp_literal(path)
                           for path in row.get("conditionalCutscenes", [])[:2]]
            while len(conditional) < 2:
                conditional.append("")
            conditional_entries = ", ".join('"%s"' % path for path in conditional)
            out.write(
                "    {0x%02xu, MenuActionKind::%s, %du, %du, {%s}, {%s}, \"%s\", \"%s\", {%s}, %du},\n" % (
                    row["action"], row["kind"], len(row["screens"]),
                    len(row["variants"]), entries, supplier_entries, cutscene,
                    note, conditional_entries,
                    len(row.get("conditionalCutscenes", []))))
        out.write("};\n\n")
        out.write("inline constexpr std::size_t kRecoveredMenuActionsCount =\n")
        out.write("    sizeof(kRecoveredMenuActions) / sizeof(kRecoveredMenuActions[0]);\n\n")
        out.write("inline const RecoveredMenuAction* findMenuAction(u32 actionCode) {\n")
        out.write("    for (const RecoveredMenuAction& action : kRecoveredMenuActions)\n")
        out.write("        if (action.actionCode == actionCode) return &action;\n")
        out.write("    return nullptr;\n")
        out.write("}\n\n")
        out.write("inline bool menuActionHasScreen(u32 actionCode) {\n")
        out.write("    const RecoveredMenuAction* action = findMenuAction(actionCode);\n")
        out.write("    return action && action->kind == MenuActionKind::Screen &&\n")
        out.write("           action->screenCount != 0;\n")
        out.write("}\n\n")
        out.write("inline FlowScreen menuActionScreen(u32 actionCode, u8 index = 0) {\n")
        out.write("    const RecoveredMenuAction* action = findMenuAction(actionCode);\n")
        out.write("    if (!action || action->kind != MenuActionKind::Screen ||\n")
        out.write("        index >= action->screenCount)\n")
        out.write("        return FlowScreen::TitleMenu;\n")
        out.write("    return action->screens[index];\n")
        out.write("}\n\n")
        out.write("inline u32 menuActionSupplier(u32 actionCode, u8 index = 0) {\n")
        out.write("    const RecoveredMenuAction* action = findMenuAction(actionCode);\n")
        out.write("    if (!action || index >= action->variantCount) return 0;\n")
        out.write("    return action->variantSuppliers[index];\n")
        out.write("}\n\n")
        out.write("inline const char* menuActionCutscene(u32 actionCode) {\n")
        out.write("    const RecoveredMenuAction* action = findMenuAction(actionCode);\n")
        out.write("    return action ? action->cutscenePath : \"\";\n")
        out.write("}\n\n")
        out.write("inline const char* menuActionConditionalCutscene(u32 actionCode,\n")
        out.write("                                                   u8 index = 0) {\n")
        out.write("    const RecoveredMenuAction* action = findMenuAction(actionCode);\n")
        out.write("    if (!action || index >= action->conditionalCutsceneCount) return \"\";\n")
        out.write("    return action->conditionalCutscenes[index];\n")
        out.write("}\n\n")
        out.write("inline const char* menuActionKindName(MenuActionKind kind) {\n")
        out.write("    switch (kind) {\n")
        for kind in [KIND_NONE, KIND_SCREEN, KIND_CONTINUE, KIND_RESUME,
                     KIND_EXIT_STAGE, KIND_EXIT_GAME, KIND_MANAGER_PHASE,
                     KIND_CUTSCENE, KIND_CHARACTER_SELECTED]:
            out.write("    case MenuActionKind::%s: return \"%s\";\n" % (kind, kind))
        out.write("    }\n")
        out.write("    return \"None\";\n")
        out.write("}\n\n")
        out.write("} // namespace mmu\n")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    report = os.path.join(root, "docs", "menu_actions.txt")
    header = os.path.join(root, "core", "include", "mmu", "core",
                          "MenuActionTable.h")
    pe = PE(open(sys.argv[1], "rb").read())
    rows = collect_actions(pe, root)
    write_report(rows, report)
    write_header(rows, header)
    print("actions=%d screenActions=%d report=%s header=%s" % (
        len(rows), sum(row["kind"] == KIND_SCREEN for row in rows),
        report, header))
    return 0


if __name__ == "__main__":
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    raise SystemExit(main())
