#include "mmu/core/StateDispatcher.h"

#include <utility>

namespace mmu {

u32 stateNameHash(std::string_view name) {
    // Exact model of 0x43c010: for each byte, shift the accumulator left one
    // bit eight times and XOR the polynomial when the old top bit was set.
    u32 value = 0;
    for (char ch : name) {
        const u32 byte = static_cast<unsigned char>(ch);
        for (u32 mask = 0x80u; mask != 0; mask >>= 1) {
            const u32 topBit = value & 0x80000000u;
            value = (value << 1) | ((byte & mask) != 0 ? 1u : 0u);
            if (topBit != 0) value ^= 0x4c11db7u;
        }
    }
    return value;
}

void NamedStateDispatcher::reset() {
    current_ = kInvalidStateHash;
    previous_ = kInvalidStateHash;
    requested_ = kInvalidStateHash;
    transitioning_ = false;
    enabled_ = true;
}

void NamedStateDispatcher::registerState(std::string_view name, StateHooks hooks) {
    const u32 hash = stateNameHash(name);
    const float timerRemaining = hooks.timerSeconds;
    if (Entry* entry = find(hash)) {
        entry->name.assign(name.data(), name.size());
        entry->hooks = std::move(hooks);
        entry->timerRemaining = timerRemaining;
        return;
    }
    Entry entry;
    entry.hash = hash;
    entry.name.assign(name.data(), name.size());
    entry.hooks = std::move(hooks);
    entry.timerRemaining = timerRemaining;
    entries_.push_back(std::move(entry));
}

bool NamedStateDispatcher::setState(std::string_view name) {
    return setStateHash(stateNameHash(name));
}

bool NamedStateDispatcher::setStateHash(u32 hash) {
    if (transitioning_)
        return false;

    requested_ = hash;
    transitioning_ = true;

    if (current_ != kInvalidStateHash) {
        if (const Entry* old = find(current_)) {
            if (old->hooks.onExit) old->hooks.onExit();
        }
    }

    previous_ = current_;
    current_ = hash;
    requested_ = kInvalidStateHash;

    if (const Entry* next = find(current_)) {
        if (next->hooks.onEnter) next->hooks.onEnter();
    }

    transitioning_ = false;
    return true;
}

void NamedStateDispatcher::update() {
    if (!enabled_ || transitioning_ || current_ == kInvalidStateHash)
        return;
    if (const Entry* entry = find(current_)) {
        if (entry->hooks.onUpdate) entry->hooks.onUpdate();
    }
}

void NamedStateDispatcher::tickTimers(float dt, u32 activeMask, bool paused) {
    if (!enabled_ || transitioning_ || paused || dt <= 0.0f)
        return;

    std::vector<std::function<void()>> due;
    for (Entry& entry : entries_) {
        if (entry.hooks.timerSeconds <= 0.0f || !entry.hooks.onTimer)
            continue;
        if ((activeMask & entry.hooks.timerMask) == 0)
            continue;

        entry.timerRemaining -= dt;
        if (entry.timerRemaining > 0.0f)
            continue;

        due.push_back(entry.hooks.onTimer);
        entry.timerRemaining = entry.hooks.timerSeconds;
    }

    for (std::function<void()>& callback : due)
        callback();
}

NamedStateDispatcher::Entry* NamedStateDispatcher::find(u32 hash) {
    for (Entry& entry : entries_)
        if (entry.hash == hash) return &entry;
    return nullptr;
}

const NamedStateDispatcher::Entry* NamedStateDispatcher::find(u32 hash) const {
    for (const Entry& entry : entries_)
        if (entry.hash == hash) return &entry;
    return nullptr;
}

std::string_view NamedStateDispatcher::nameForHash(u32 hash) const {
    if (hash == kInvalidStateHash) return {};
    if (const Entry* entry = find(hash)) return entry->name;
    return {};
}

} // namespace mmu
