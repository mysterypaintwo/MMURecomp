#pragma once

#include "mmu/core/SaveFlagTable.h"
#include "mmu/core/Types.h"

#include <array>
#include <cstddef>

namespace mmu {

// Portable model of the keyed save-flag state machine in FUN_00441820.
//
// Each key has a four-bit state stored at save-record +0x24:
//   bit 0: condition currently false
//   bit 1: condition just became false (falling edge)
//   bit 2: condition currently true
//   bit 3: condition just became true (rising edge)
//
// FUN_00441c80(key, mask) tests those bits. The recovered wrappers use mask 8
// (FUN_00441d90) and mask 4 (FUN_00441db0). Direct sites use mask 8 for keys 1
// and 30, plus mask 2 for key 30.
// Conditions come from FUN_004412d0 writer records: ids below 0x143 index the
// keyboard-state array, while higher ids index manager bytes at id - 0xf9.
// FUN_00441820 skips records whose category has no bit in the global activity
// mask (FUN_00507ec0); the harness defaults that mask to all categories.
// New records start with bits 0, while FUN_00441b70 can activate them to bit 0.
class KeyedFlagRuntime {
public:
    static constexpr u8 kFalse = 1u;
    static constexpr u8 kFalling = 2u;
    static constexpr u8 kTrue = 4u;
    static constexpr u8 kRising = 8u;
    static constexpr std::size_t kKeyCount = 64;
    // The harness has no recovered per-state mask for most screens, so it
    // starts with every recorded category enabled until a setter call is
    // applied.
    static constexpr u32 kAllCategories = 0xFFFFFFFFu;

    KeyedFlagRuntime() { reset(); }

    void reset();
    // Mirrors FUN_00441b70: false seeds every record with bit 0 (currently
    // false), while true keeps the registration default of 0.
    void setInitialFalse(bool value);
    // Boot writes manager +0 directly and does not touch +4.
    void setBootActivityMask(u32 mask) { activityMask_ = mask; }
    // Mirrors FUN_00507e90: the old mask is copied to manager +4 before +0 is
    // overwritten.
    void setActivityMask(u32 mask) {
        previousActivityMask_ = activityMask_;
        activityMask_ = mask;
    }
    u32 activityMask() const { return activityMask_; }
    u32 previousActivityMask() const { return previousActivityMask_; }

    // Re-evaluate every recovered key from condition sources. keyboard[] is
    // indexed directly by ids < 0x143; manager[] is indexed by actual manager
    // byte offsets (writer id - 0xf9 for ids >= 0x143).
    void evaluate(const u8* keyboard, std::size_t keyboardCount,
                  const u8* manager, std::size_t managerCount);

    bool query(u32 key, u8 mask) const {
        return key < kKeyCount && (bits_[key] & mask) != 0;
    }
    u8 bits(u32 key) const { return key < kKeyCount ? bits_[key] : 0u; }

    // Test-hook override: the next evaluate() treats this key's condition as
    // the supplied value regardless of the source arrays.
    void setConditionOverride(u32 key, bool value) {
        if (key < kKeyCount) {
            override_[key] = static_cast<u8>(value);
            hasOverride_[key] = true;
        }
    }
    void clearConditionOverride(u32 key) {
        if (key < kKeyCount) hasOverride_[key] = false;
    }

private:
    std::array<u8, kKeyCount> bits_{};
    std::array<u8, kKeyCount> override_{};
    std::array<bool, kKeyCount> hasOverride_{};
    u8 initialBits_ = 0u;
    u32 activityMask_ = kAllCategories;
    u32 previousActivityMask_ = kAllCategories;
};

} // namespace mmu
