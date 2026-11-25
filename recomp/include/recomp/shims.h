// Shared helpers between the hand-written import shims.
#pragma once

#include "recomp/cpu.h"

#ifdef __cplusplus
#include <string>
// Windows path (relative, or under the virtual C:\MMU root) -> host path in
// the game directory.
std::string rt_host_path(const char* win);
extern "C" {
#endif

// Fake guest address of an imported shim by symbol name (0 if not imported),
// used by GetProcAddress on the kernel32 pseudo-module.
uint32_t rt_shim_address(const char* name);

#ifdef __cplusplus
}
#endif
