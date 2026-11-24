#pragma once

#include "mmu/core/GameFlow.h"
#include "mmu/core/RegisteredStateGraph.h"
#include "mmu/core/RegisteredStateTransitions.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace mmu {

// Query helpers for the generated vtable-callback setter evidence. They only
// describe calls that were recovered from MMU.exe; they do not invent timing,
// ordering, or screen destinations.
inline constexpr u8 kFlowTransitionSetState = 1u;
inline constexpr u8 kFlowTransitionCue = 2u;

inline const char* flowTransitionKindName(u8 flags) {
    if ((flags & kFlowTransitionSetState) != 0 &&
        (flags & kFlowTransitionCue) != 0)
        return "setState|cue";
    if ((flags & kFlowTransitionSetState) != 0) return "setState";
    if ((flags & kFlowTransitionCue) != 0) return "cue";
    return "?";
}

inline const RegisteredStateTransition* findFirstTransitionTo(
    std::string_view targetName, u8 requiredFlags = 0) {
    for (const RegisteredStateTransition& row : kRegisteredStateTransitions) {
        if (row.targetName != targetName) continue;
        if (requiredFlags != 0 && (row.flags & requiredFlags) == 0) continue;
        return &row;
    }
    return nullptr;
}

inline const RegisteredStateTransition* findFirstTransitionForClass(
    std::string_view className, u16 slot = 0xffffu, u8 requiredFlags = 0) {
    for (const RegisteredStateTransition& row : kRegisteredStateTransitions) {
        if (row.className != className) continue;
        if (slot != 0xffffu && row.vtableSlot != slot) continue;
        if (requiredFlags != 0 && (row.flags & requiredFlags) == 0) continue;
        return &row;
    }
    return nullptr;
}

inline bool hasRegisteredStateTransitionTo(std::string_view targetName) {
    return findFirstTransitionTo(targetName, kFlowTransitionSetState) != nullptr;
}

inline std::size_t countTransitionsTo(std::string_view targetName,
                                      u8 requiredFlags = 0) {
    std::size_t count = 0;
    for (const RegisteredStateTransition& row : kRegisteredStateTransitions) {
        if (row.targetName != targetName) continue;
        if (requiredFlags != 0 && (row.flags & requiredFlags) == 0) continue;
        ++count;
    }
    return count;
}

inline std::size_t countTransitionsForClass(std::string_view className,
                                            u8 requiredFlags = 0) {
    std::size_t count = 0;
    for (const RegisteredStateTransition& row : kRegisteredStateTransitions) {
        if (row.className != className) continue;
        if (requiredFlags != 0 && (row.flags & requiredFlags) == 0) continue;
        ++count;
    }
    return count;
}

// Ordered constructor graphs recovered by tools/analyze_state_graph.py. These
// queries only report what the binary registers or calls; they do not fill in
// unobserved transitions.
inline std::string_view graphInitialStateForClass(std::string_view className) {
    for (const RegisteredStateGraphRow& row : kRegisteredStateGraph)
        if (row.className == className && row.isInitial != 0)
            return row.stateName;
    return {};
}

inline std::string_view graphInitialState(FlowScreen screen) {
    return graphInitialStateForClass(flowScreenName(screen));
}

} // namespace mmu
