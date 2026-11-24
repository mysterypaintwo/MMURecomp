#pragma once

#include "mmu/core/Types.h"

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace mmu {

// ---------------------------------------------------------------------------
// Portable form of the named-state dispatcher at global 0x6c55cc in MMU.exe.
//
// Recovered anchors:
//   0x43c010  CRC-like state-name hash (poly 0x4c11db7, seed 0)
//   0x4405c0  global setState(char const*)
//   0x4404c0  hash, exit old state (+0x40 callback), enter new state (+0x20)
//   0x446ca0  register state (hash + two callback payloads)
//   0x461380  reentrancy guard byte at +0x20
//   0x4464a0  per-frame dispatcher update
//
// The original registration table is populated at runtime from asset/object
// data, so this core type provides the same semantics with string names instead
// of hardcoded MMU.exe pointers. See docs/gameplay_reconstruction.md.
// ---------------------------------------------------------------------------

constexpr u32 kInvalidStateHash = 0xffffffffu;

u32 stateNameHash(std::string_view name);

struct StateHooks {
    std::function<void()> onEnter;
    std::function<void()> onExit;
    std::function<void()> onUpdate;

    // Optional per-state timer recovered from 0x446760: remaining seconds,
    // input/activity mask, and callback fired when the timer expires.
    float timerSeconds = 0.0f;
    u32 timerMask = 0xffffffffu;
    std::function<void()> onTimer;
};

class NamedStateDispatcher {
public:
    void reset();

    // Replaces an existing registration with the same name.
    void registerState(std::string_view name, StateHooks hooks = {});

    // Ignores the request while a transition callback is running, matching the
    // guard checked by 0x4405c0 through 0x461380.
    bool setState(std::string_view name);
    bool setStateHash(u32 hash);

    // Calls the current state's onUpdate callback when updates are enabled.
    void update();

    // Mirrors the per-registration timer pass at 0x4464a0/0x446760. Due
    // callbacks are collected first and fired after the scan so they can safely
    // change dispatcher state. activeMask models the value returned by the
    // original 0x00507ec0 input/activity query.
    void tickTimers(float dt, u32 activeMask = 0xffffffffu, bool paused = false);

    bool transitioning() const { return transitioning_; }
    bool enabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }

    u32 currentHash() const { return current_; }
    u32 previousHash() const { return previous_; }
    u32 requestedHash() const { return requested_; }
    bool hasState() const { return current_ != kInvalidStateHash; }

    std::string_view currentName() const { return nameForHash(current_); }
    std::string_view previousName() const { return nameForHash(previous_); }

private:
    struct Entry {
        u32 hash = kInvalidStateHash;
        std::string name;
        StateHooks hooks;
        float timerRemaining = 0.0f;
    };

    Entry* find(u32 hash);
    const Entry* find(u32 hash) const;
    std::string_view nameForHash(u32 hash) const;

    std::vector<Entry> entries_;
    u32 current_ = kInvalidStateHash;
    u32 previous_ = kInvalidStateHash;
    u32 requested_ = kInvalidStateHash;
    bool transitioning_ = false;
    bool enabled_ = true;
};

} // namespace mmu
