#!/usr/bin/env python3
"""Dump exact player-spawn token evidence from decoded level data.

The tool reads levels.mmu with the checked-in mmu.Level schema from
docs/protos/level.proto and pairs each scene with the editor-side path in
tools/scene_names.inc. It emits only exact GameSpawnObject.type tokens, raw
coordinates, the packed flag word, raw attr1, and exact-token/lowercase
substring counts. No scene is designated as the real player spawn and no
gameplay meaning is assigned to any flag, coordinate or attribute.

Usage:
  python3 tools/analyze_player_spawn_evidence.py LEVELS_MMU SCENE_NAMES_INC REPORT HEADER
"""

import re
import struct
import sys

EXACT_TOKENS = (
    "MegaMan", "ProtoMan", "Bass", "Zero", "Treble", "ShadowMegaMan",
    "ZPrototype", "Z-PROTOTYPE", "MEGA MAN", "player", "Player",
    "MegaTank",
)
SUBSTRING_TOKENS = ("megaman", "player")
FLAG_LINES = (
    "tools/bake_assets.cpp:594 packedSpawn.flags = (spawn.facingRight ? 1u : 0u)",
    "tools/bake_assets.cpp:595                    | (spawn.respawnOnDeath ? 2u : 0u)",
    "tools/bake_assets.cpp:596                    | (spawn.respawnOnScreen ? 4u : 0u)",
    "tools/bake_assets.cpp:597                    | (spawn.deleteOffScreen ? 8u : 0u)",
    "tools/bake_assets.cpp:598                    | (spawn.collectible ? 16u : 0u);",
    "core/src/PackedArchive.cpp:214 spawn.facingRight = (flags & (1u << 0)) != 0;",
    "core/src/PackedArchive.cpp:215 spawn.respawnOnDeath = (flags & (1u << 1)) != 0;",
    "core/src/PackedArchive.cpp:216 spawn.respawnOnScreen = (flags & (1u << 2)) != 0;",
    "core/src/PackedArchive.cpp:217 spawn.deleteOffScreen = (flags & (1u << 3)) != 0;",
    "core/src/PackedArchive.cpp:218 spawn.collectible = (flags & (1u << 4)) != 0;",
)


def varint(data, pos):
    value = shift = 0
    while pos < len(data):
        byte = data[pos]
        pos += 1
        value |= (byte & 0x7f) << shift
        if byte < 0x80:
            return value, pos
        shift += 7
    raise ValueError("truncated varint")


def fields(data):
    pos = 0
    while pos < len(data):
        key, pos = varint(data, pos)
        number, wire = key >> 3, key & 7
        if wire == 0:
            value, pos = varint(data, pos)
        elif wire == 5:
            value = struct.unpack_from("<f", data, pos)[0]
            pos += 4
        elif wire == 1:
            value = struct.unpack_from("<Q", data, pos)[0]
            pos += 8
        elif wire == 2:
            length, pos = varint(data, pos)
            if pos + length > len(data):
                raise ValueError("truncated length-delimited field")
            value = data[pos:pos + length]
            pos += length
        else:
            raise ValueError("unsupported wire type %d" % wire)
        yield number, wire, value


def as_text(value):
    return value.decode("utf-8")


def parse_spawn(data):
    spawn = {
        "x": 0.0, "y": 0.0, "autoRespawn": 0.0,
        "facingRight": False, "respawnOnDeath": False,
        "respawnOnScreen": False, "deleteOffScreen": False,
        "collectible": False, "type": "", "attr1": "", "attr2": "",
    }
    for number, wire, value in fields(data):
        if number == 1 and wire == 5:
            spawn["x"] = value
        elif number == 2 and wire == 5:
            spawn["y"] = value
        elif number in (3, 4, 5, 6, 7) and wire == 0:
            spawn[("facingRight", "respawnOnDeath", "respawnOnScreen",
                   "deleteOffScreen", "collectible")[number - 3]] = value != 0
        elif number == 8 and wire == 5:
            spawn["autoRespawn"] = value
        elif number == 9 and wire == 2:
            spawn["type"] = as_text(value)
        elif number == 10 and wire == 2:
            spawn["attr1"] = as_text(value)
        elif number == 11 and wire == 2:
            spawn["attr2"] = as_text(value)
    return spawn


def parse_scene(data):
    scene = {"key": 0, "spawns": []}
    for number, wire, value in fields(data):
        if number == 1 and wire == 2:
            scene["spawns"].append(parse_spawn(value))
        elif number == 2 and wire == 0:
            scene["key"] = value & 0xffffffff
    return scene


def parse_level(data):
    scenes = []
    for number, wire, value in fields(data):
        if number == 1 and wire == 2:
            scenes.append(parse_scene(value))
    return scenes


def scene_names(text):
    raw = re.findall(r'"((?:\\.|[^"\\])*)"', text)
    return [item.replace("\\\"", "\"").replace("\\\\", "\\") for item in raw]


def flags_for(spawn):
    return ((1 if spawn["facingRight"] else 0) |
            (2 if spawn["respawnOnDeath"] else 0) |
            (4 if spawn["respawnOnScreen"] else 0) |
            (8 if spawn["deleteOffScreen"] else 0) |
            (16 if spawn["collectible"] else 0))


def analyze(levels_path, names_path):
    scenes = parse_level(open(levels_path, "rb").read())
    names = scene_names(open(names_path, "r", encoding="utf-8").read())
    if len(names) < len(scenes):
        raise RuntimeError("fewer scene paths than decoded scenes")

    exact_counts = {token: 0 for token in EXACT_TOKENS}
    substring_counts = {token: 0 for token in SUBSTRING_TOKENS}
    type_counts = {}
    candidates = []
    for scene_index, scene in enumerate(scenes):
        for spawn_index, spawn in enumerate(scene["spawns"]):
            spawn_type = spawn["type"]
            type_counts[spawn_type] = type_counts.get(spawn_type, 0) + 1
            for token in exact_counts:
                if spawn_type == token:
                    exact_counts[token] += 1
            lowered = spawn_type.lower()
            for token in substring_counts:
                if token in lowered:
                    substring_counts[token] += 1
            if "megaman" in lowered or "player" in lowered:
                candidates.append({
                    "sceneIndex": scene_index,
                    "sceneName": names[scene_index],
                    "sceneKey": scene["key"],
                    "spawnIndex": spawn_index,
                    "x": spawn["x"],
                    "y": spawn["y"],
                    "flags": flags_for(spawn),
                    "type": spawn_type,
                    "attr1": spawn["attr1"],
                })
    return {
        "scenes": scenes,
        "names": names,
        "spawnCount": sum(len(scene["spawns"]) for scene in scenes),
        "uniqueTypes": len(type_counts),
        "exactCounts": exact_counts,
        "substringCounts": substring_counts,
        "candidates": candidates,
    }


def write_report(path, data):
    lines = [
        "Generated by tools/analyze_player_spawn_evidence.py",
        "Source schema: docs/protos/level.proto:5-25 (mmu.Level/mmu.Scene/mmu.GameSpawnObject).",
        "Scene paths: tools/scene_names.inc, in levels.mmu scene order.",
        "No scene is designated as the real player spawn; no gameplay meaning is assigned.",
        "",
        "scenes=%d spawns=%d unique_types=%d substring_candidates=%d" % (
            len(data["scenes"]), data["spawnCount"], data["uniqueTypes"],
            len(data["candidates"])),
        "",
        "Exact GameSpawnObject.type checks (case-sensitive):",
    ]
    for token in EXACT_TOKENS:
        lines.append("  %-14s %d" % (token, data["exactCounts"][token]))
    lines.extend(["", "Lowercase substring checks over exact type tokens:"])
    for token in SUBSTRING_TOKENS:
        lines.append("  %-14s %d" % (token, data["substringCounts"][token]))
    lines.extend(["", "Packed flag derivation (verbatim source lines):"])
    lines.extend("  " + line for line in FLAG_LINES)
    lines.extend(["", "Substring-matching records (scene order, then spawn order):"])
    for row in data["candidates"]:
        lines.append(
            "  scene=%u path=%s key=0x%08x spawn=%u type=%s x=%.1f y=%.1f "
            "flags=%u attr1=%s" %
            (row["sceneIndex"], row["sceneName"], row["sceneKey"],
             row["spawnIndex"], row["type"], row["x"], row["y"],
             row["flags"], row["attr1"]))
    lines.append("")
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def cpp_string(value):
    return '"%s"' % value.replace("\\", "\\\\").replace("\"", "\\\"")


def write_header(path, data):
    lines = [
        "#pragma once",
        "",
        '#include "mmu/core/Types.h"',
        "",
        "#include <cstddef>",
        "#include <string_view>",
        "",
        "namespace mmu {",
        "",
        "// Generated by tools/analyze_player_spawn_evidence.py from levels.mmu.",
        "// Exact token evidence only; no real player spawn is designated.",
        "inline constexpr u32 kPlayerSpawnSceneCount = %du;" % len(data["scenes"]),
        "inline constexpr u32 kPlayerSpawnRecordCount = %du;" % data["spawnCount"],
        "inline constexpr u32 kPlayerSpawnUniqueTypeCount = %du;" % data["uniqueTypes"],
        "inline constexpr u32 kPlayerSpawnSubstringMegaManCount = %du;" %
        data["substringCounts"]["megaman"],
        "inline constexpr u32 kPlayerSpawnSubstringPlayerCount = %du;" %
        data["substringCounts"]["player"],
        "",
        "struct PlayerSpawnTypeCount {",
        "    std::string_view type;",
        "    u32 count;",
        "};",
        "",
        "inline constexpr PlayerSpawnTypeCount kPlayerSpawnExactChecks[] = {",
    ]
    for token in EXACT_TOKENS:
        lines.append("    {%s, %du}," % (cpp_string(token),
                                         data["exactCounts"][token]))
    lines.extend([
        "};",
        "",
        "struct PlayerSpawnCandidate {",
        "    u32 sceneIndex;",
        "    u32 sceneKey;",
        "    u32 spawnIndex;",
        "    float x;",
        "    float y;",
        "    u32 flags;",
        "    std::string_view sceneName;",
        "    std::string_view type;",
        "    std::string_view attr1;",
        "};",
        "",
        "inline constexpr PlayerSpawnCandidate kPlayerSpawnCandidates[] = {",
    ])
    for row in data["candidates"]:
        lines.append(
            "    {%du, 0x%08xu, %du, %.1ff, %.1ff, %du, %s, %s, %s}," %
            (row["sceneIndex"], row["sceneKey"], row["spawnIndex"],
             row["x"], row["y"], row["flags"], cpp_string(row["sceneName"]),
             cpp_string(row["type"]), cpp_string(row["attr1"])))
    lines.extend([
        "};",
        "",
        "inline constexpr std::size_t kPlayerSpawnExactCheckCount =",
        "    sizeof(kPlayerSpawnExactChecks) / sizeof(kPlayerSpawnExactChecks[0]);",
        "inline constexpr std::size_t kPlayerSpawnCandidateCount =",
        "    sizeof(kPlayerSpawnCandidates) / sizeof(kPlayerSpawnCandidates[0]);",
        "",
        "inline const PlayerSpawnTypeCount* playerSpawnExactCheck(std::string_view type) {",
        "    for (const PlayerSpawnTypeCount& row : kPlayerSpawnExactChecks)",
        "        if (row.type == type) return &row;",
        "    return nullptr;",
        "}",
        "",
        "inline const PlayerSpawnCandidate* playerSpawnCandidate(u32 sceneIndex,",
        "                                                        u32 spawnIndex) {",
        "    for (const PlayerSpawnCandidate& row : kPlayerSpawnCandidates)",
        "        if (row.sceneIndex == sceneIndex && row.spawnIndex == spawnIndex)",
        "            return &row;",
        "    return nullptr;",
        "}",
        "",
        "} // namespace mmu",
        "",
    ])
    with open(path, "w", encoding="utf-8") as out:
        out.write("\n".join(lines))


def main():
    if len(sys.argv) != 5:
        print(__doc__)
        return 2
    levels_path, names_path, report, header = sys.argv[1:]
    data = analyze(levels_path, names_path)
    write_report(report, data)
    write_header(header, data)
    print("scenes=%d spawns=%d unique_types=%d substring_candidates=%d" % (
        len(data["scenes"]), data["spawnCount"], data["uniqueTypes"],
        len(data["candidates"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
