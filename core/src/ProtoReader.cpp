#include "mmu/core/ProtoReader.h"

namespace mmu {

bool ProtoReader::next(Field& field) {
    u64 tag = 0;
    if (!readVarint(tag) || tag == 0) return false;
    field.number = static_cast<u32>(tag >> 3);
    field.wire = static_cast<u8>(tag & 7);
    if (field.wire > Fixed32 || field.number == 0) return false;
    currentWire_ = field.wire;
    return true;
}

bool ProtoReader::readVarint(u64& out) {
    u64 value = 0;
    for (unsigned shift = 0; shift < 70; shift += 7) {
        if (pos_ >= bytes_.size) return false;
        const u8 byte = bytes_.data[pos_++];
        if (shift == 63 && byte > 1) return false;
        value |= static_cast<u64>(byte & 0x7f) << shift;
        if (!(byte & 0x80)) {
            out = value;
            return true;
        }
    }
    return false;
}

bool ProtoReader::readU32(u32& out) {
    if (currentWire_ != Varint) return false;
    u64 value = 0;
    if (!readVarint(value)) return false;
    out = static_cast<u32>(value);
    return true;
}

bool ProtoReader::readI32(i32& out) {
    u32 value = 0;
    if (!readU32(value)) return false;
    out = static_cast<i32>(value);
    return true;
}

bool ProtoReader::readBool(bool& out) {
    u32 value = 0;
    if (!readU32(value)) return false;
    out = value != 0;
    return true;
}

bool ProtoReader::readFloat(float& out) {
    if (currentWire_ != Fixed32 || bytes_.size - pos_ < 4) return false;
    u32 bits = static_cast<u32>(bytes_.data[pos_])
             | static_cast<u32>(bytes_.data[pos_ + 1]) << 8
             | static_cast<u32>(bytes_.data[pos_ + 2]) << 16
             | static_cast<u32>(bytes_.data[pos_ + 3]) << 24;
    pos_ += 4;
    out = bitsToFloat(bits);
    return true;
}

bool ProtoReader::readDouble(double& out) {
    if (currentWire_ != Fixed64 || bytes_.size - pos_ < 8) return false;
    u64 bits = 0;
    for (unsigned i = 0; i < 8; ++i)
        bits |= static_cast<u64>(bytes_.data[pos_ + i]) << (i * 8);
    pos_ += 8;
    out = bitsToDouble(bits);
    return true;
}

bool ProtoReader::readBytes(ByteView& out) {
    if (currentWire_ != LengthDelimited) return false;
    u64 length = 0;
    if (!readVarint(length) || length > bytes_.size - pos_) return false;
    out = bytes_.subview(pos_, static_cast<std::size_t>(length));
    pos_ += static_cast<std::size_t>(length);
    return true;
}

bool ProtoReader::readString(std::string_view& out) {
    ByteView bytes;
    if (!readBytes(bytes)) return false;
    out = std::string_view(reinterpret_cast<const char*>(bytes.data), bytes.size);
    return true;
}

bool ProtoReader::readMessage(ProtoReader& out) {
    ByteView bytes;
    if (!readBytes(bytes)) return false;
    out = ProtoReader(bytes);
    return true;
}

bool ProtoReader::skipField() {
    switch (currentWire_) {
    case Varint: {
        u64 ignored = 0;
        return readVarint(ignored);
    }
    case Fixed64:
        if (bytes_.size - pos_ < 8) return false;
        pos_ += 8;
        return true;
    case LengthDelimited: {
        ByteView ignored;
        return readBytes(ignored);
    }
    case Fixed32:
        if (bytes_.size - pos_ < 4) return false;
        pos_ += 4;
        return true;
    default:
        // Groups are not used by the MMU schemas.
        return false;
    }
}

} // namespace mmu
