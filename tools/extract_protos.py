#!/usr/bin/env python3
"""Recover the original proto2 schemas embedded in MMU.exe.

MSVC protobuf-generated code stores a serialized google.protobuf.FileDescriptorProto
as a NUL-terminated C string for every *.pb.cc.  This script locates those blobs,
parses them, and emits equivalent *.proto files.
"""
import argparse
import pathlib
from google.protobuf import descriptor_pb2

PROTO_FILES = {
    "sdl": 0x259938,
    "animation": 0x25B488,
    "cutscene": 0x25B028,
    "gun": 0x25AC48,
    "level": 0x25A8F0,
    "savedgame": 0x259F58,
}

TYPE_NAMES = {
    1: "double", 2: "float", 3: "int64", 4: "uint64", 5: "int32", 6: "fixed64",
    7: "fixed32", 8: "bool", 9: "string", 10: "group", 11: "message", 12: "bytes",
    13: "uint32", 14: "enum", 15: "sfixed32", 16: "sfixed64", 17: "sint32", 18: "sint64",
}
LABELS = {1: "optional", 2: "required", 3: "repeated"}


def recover(data: bytes, offset: int):
    """Find the longest valid FileDescriptorProto parse starting at offset."""
    best = None
    for length in range(10, 8192):
        try:
            fd = descriptor_pb2.FileDescriptorProto()
            fd.ParseFromString(data[offset:offset + length])
            best = fd
        except Exception:
            pass
    if best is None or not best.name:
        raise ValueError(f"no descriptor at {offset:#x}")
    return best


def emit_message(msg, indent=""):
    out = [f"{indent}message {msg.name} {{"]
    inner = indent + "  "
    for field in msg.field:
        label = LABELS.get(field.label, "optional")
        typename = TYPE_NAMES.get(field.type, "bytes")
        if field.type in (10, 11, 14):
            typename = field.type_name.lstrip(".")
        line = f"{inner}{label} {typename} {field.name} = {field.number}"
        if field.HasField("default_value"):
            line += f" [default = {field.default_value}]"
        out.append(line + ";")
    for nested in msg.nested_type:
        out.append("")
        out.extend(emit_message(nested, inner))
    for enum in msg.enum_type:
        out.append("")
        out.extend(emit_enum(enum, inner))
    out.append(f"{indent}}}")
    return out


def emit_enum(enum, indent=""):
    out = [f"{indent}enum {enum.name} {{"]
    inner = indent + "  "
    for value in enum.value:
        out.append(f"{inner}{value.name} = {value.number};")
    out.append(f"{indent}}}")
    return out


def to_proto(fd):
    out = ['syntax = "proto2";', ""]
    out += [f'import "{dep}";' for dep in fd.dependency]
    if fd.dependency:
        out.append("")
    if fd.package:
        out += [f"package {fd.package};", ""]
    for i, msg in enumerate(fd.message_type):
        if i:
            out.append("")
        out.extend(emit_message(msg))
    for enum in fd.enum_type:
        out.append("")
        out.extend(emit_enum(enum))
    return "\n".join(out) + "\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", required=True, help="path to MMU.exe")
    ap.add_argument("--out", required=True, help="output directory for .proto files")
    args = ap.parse_args()

    data = pathlib.Path(args.exe).read_bytes()
    outdir = pathlib.Path(args.out)
    outdir.mkdir(parents=True, exist_ok=True)
    for name, offset in PROTO_FILES.items():
        fd = recover(data, offset)
        assert fd.name == f"{name}.proto", (fd.name, name)
        path = outdir / f"{name}.proto"
        path.write_text(to_proto(fd))
        print(f"{path}: {len(fd.message_type)} messages")


if __name__ == "__main__":
    main()
