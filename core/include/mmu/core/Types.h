#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace mmu {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

// Non-owning view over a byte range. Deliberately tiny: no allocator, no
// exceptions, safe to pass across a platform boundary.
struct ByteView {
    const u8* data = nullptr;
    std::size_t size = 0;

    constexpr ByteView() = default;
    constexpr ByteView(const u8* p, std::size_t n) : data(p), size(n) {}
    explicit ByteView(std::string_view s)
        : data(reinterpret_cast<const u8*>(s.data())), size(s.size()) {}

    constexpr const u8* begin() const { return data; }
    constexpr const u8* end() const { return data + size; }
    constexpr bool empty() const { return size == 0; }

    constexpr ByteView subview(std::size_t offset, std::size_t count) const {
        if (offset > size) return {};
        return ByteView(data + offset, count < size - offset ? count : size - offset);
    }
};

inline float bitsToFloat(u32 bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

inline double bitsToDouble(u64 bits) {
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

} // namespace mmu
