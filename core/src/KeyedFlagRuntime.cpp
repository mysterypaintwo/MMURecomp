#include "mmu/core/KeyedFlagRuntime.h"

namespace mmu {

void KeyedFlagRuntime::reset() {
    bits_.fill(initialBits_);
    override_.fill(0u);
    hasOverride_.fill(false);
    activityMask_ = kAllCategories;
    previousActivityMask_ = kAllCategories;
}

void KeyedFlagRuntime::setInitialFalse(bool value) {
    initialBits_ = value ? kFalse : 0u;
    reset();
}

void KeyedFlagRuntime::evaluate(const u8* keyboard, std::size_t keyboardCount,
                                const u8* manager, std::size_t managerCount) {
    std::array<bool, kKeyCount> condition{};
    condition.fill(false);

    for (const auto& row : kSaveFlagWriters) {
        if (row.key >= kKeyCount || condition[row.key] ||
            (row.category & activityMask_) == 0) {
            continue;
        }
        bool value = false;
        if (row.aux < kKeyedFlagKeyboardLimit) {
            value = keyboard != nullptr && row.aux < keyboardCount &&
                    keyboard[row.aux] != 0;
        } else {
            const u32 offset = row.aux - kKeyedFlagManagerByteBias;
            value = manager != nullptr && offset < managerCount &&
                    manager[offset] != 0;
        }
        if (value) {
            condition[row.key] = true;
        }
    }

    for (std::size_t key = 0; key < kKeyCount; ++key) {
        const bool value = hasOverride_[key] ? override_[key] != 0
                                             : condition[key];
        u8 next = bits_[key];
        if (!value) {
            // FUN_00441820 false branch: current false, falling edge from an
            // old true state, then clear the two true bits.
            next |= kFalse;
            if ((next & kFalling) != 0) {
                next = static_cast<u8>(next & ~kFalling);
            }
            if ((next & kTrue) != 0) {
                next |= kFalling;
            }
            next = static_cast<u8>(next & ~(kTrue | kRising));
        } else {
            // FUN_00441820 true branch: current true, rising edge from an old
            // false state, then clear the two false bits.
            next |= kTrue;
            if ((next & kRising) != 0) {
                next = static_cast<u8>(next & ~kRising);
            }
            if ((next & kFalse) != 0) {
                next |= kRising;
            }
            next = static_cast<u8>(next & ~(kFalse | kFalling));
        }
        bits_[key] = next;
    }
}

} // namespace mmu
