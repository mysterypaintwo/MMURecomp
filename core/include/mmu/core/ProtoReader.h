#pragma once

#include "mmu/core/Types.h"

#include <string_view>

namespace mmu {

// Minimal proto2 wire-format reader. The original game ships its database as
// protobuf messages, but pulling the full protobuf runtime onto every console
// is unnecessary: reading is all we need, and the wire format is stable.
class ProtoReader {
public:
    enum WireType : u8 {
        Varint = 0,
        Fixed64 = 1,
        LengthDelimited = 2,
        StartGroup = 3,
        EndGroup = 4,
        Fixed32 = 5,
    };

    struct Field {
        u32 number = 0;
        u8 wire = 0;
    };

    explicit constexpr ProtoReader(ByteView bytes = {}) : bytes_(bytes) {}

    // Reads the next tag. After this succeeds, exactly one read*() call must
    // consume the field's value (or skipField() must discard it).
    bool next(Field& field);

    bool readVarint(u64& out);
    bool readU32(u32& out);
    bool readI32(i32& out);
    bool readFloat(float& out);
    bool readDouble(double& out);
    bool readBool(bool& out);
    bool readBytes(ByteView& out);
    bool readString(std::string_view& out);
    bool readMessage(ProtoReader& out);
    bool skipField();

    constexpr bool done() const { return pos_ >= bytes_.size; }
    constexpr ByteView remaining() const { return bytes_.subview(pos_, bytes_.size - pos_); }

private:
    ByteView bytes_{};
    std::size_t pos_ = 0;
    u8 currentWire_ = 0;
};

} // namespace mmu
