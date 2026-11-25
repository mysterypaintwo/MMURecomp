// Guest CPU state and runtime interface shared by generated code and shims.
// The generated code (tools/recomp/lift.py) is C; shims may be C or C++.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union Xmm {
    uint8_t b[16];
    uint16_t w[8];
    uint32_t d[4];
    uint64_t q[2];
    float f[4];
    double dd[2];
} Xmm;

typedef struct Cpu {
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    // x87: physical registers + TOP, condition codes and control word.
    double st[8];
    uint32_t top;
    uint16_t fcw;
    uint8_t c0, c1, c2, c3;
    Xmm xmm[8];
    union { uint64_t q; uint32_t d[2]; uint16_t w[4]; uint8_t b[8]; } mm[8]; // MMX
    uint32_t mxcsr;
    uint32_t eflags_extra; // EFLAGS.ID toggled by the CRT's cpuid probe
    uint32_t df;           // EFLAGS.DF (string op direction)
    uint16_t fsw_exc;      // x87 exception flags (kept for fnstenv; never raised)
} Cpu;

typedef void (*RtFn)(Cpu*);
typedef struct { uint32_t va; RtFn fn; } RtFunc;
typedef struct { uint32_t iat; const char* dll; const char* name; RtFn fn; } RtImport;
// IAT slot bound to a lifted DLL export (code or data VA).
typedef struct { uint32_t iat; uint32_t value; } RtBinding;
// Loaded module: file name (in the game directory), preferred and guest base.
typedef struct { const char* file; uint32_t original_base; uint32_t base; uint32_t entry; int is_dll; } RtModule;

// Guest address space (a compact 32-bit Windows-style process layout):
//   0x00001000  thread information block (FS segment base)
//   0x00400000  MMU.exe image (original base; the PE has no relocations)
//   0x00800000  guest stack (8 MiB, grows down from 0x01000000)
//   0x01000000  guest heap (to 0x58000000, 1.36 GiB; the game peaks near 1 GiB)
//   0x58000000  lifted DLLs (msvcr90, msvcp90, SDL, SDL_gfx/image/mixer, ogg/vorbis)
//   0x58f00000  fake import addresses (one 16-byte slot per IAT entry)
//   0x59000000  end of the guest space consoles must provide (RT_GUEST_SIZE)
#define RT_FS_BASE 0x00001000u
#define RT_STACK_TOP 0x00fffff0u
#define RT_HEAP_BASE 0x01000000u
#define RT_HEAP_END 0x58000000u
#define RT_IMPORT_BASE 0x58f00000u
#define RT_GUEST_SIZE 0x59000000u

extern uint8_t* rt_mem;

extern const RtFunc rt_functions[];
extern const unsigned rt_function_count;
extern const RtImport rt_imports[];
extern const unsigned rt_import_count;
extern const uint32_t rt_entry_point;
extern const RtBinding rt_bindings[];
extern const unsigned rt_binding_count;
extern const RtModule rt_modules[];
extern const unsigned rt_module_count;
const char* rt_game_dir(void);

void rt_call_indirect(Cpu* c, uint32_t target);
void rt_jump_indirect(Cpu* c, uint32_t target);
void rt_cpuid(Cpu* c);
void rt_fault(uint32_t va, const char* what);
void rt_dump_state(void);  // guest call ring + registers to stderr
// recomp/src/log.cpp: log files / crash handlers / periodic memory status.
void rt_log_init(const char* log_dir);
void rt_crumb(const char* fmt, ...);
void rt_log_memory(const char* why);
void rt_log_status(unsigned frames);
void rt_missing_import(const char* dll, const char* name);
void rt_memmove_guest(uint32_t dst, uint32_t src, uint32_t n);
void rt_stos(uint32_t dst, uint32_t value, uint32_t count, int unit);
RtFn rt_lookup(uint32_t va);

// Process setup / entry (recomp/src/runtime.c).
int rt_init(const char* game_dir);
int rt_run_entry(void);

#if defined(_MSC_VER) || defined(_WIN32)
// PE/COFF linkers mishandle weak definitions across objects: the stubs are
// plain functions there and the runtime's own definitions, linked first, win
// (MinGW: --allow-multiple-definition, recomp/CMakeLists.txt).
#define RT_WEAK
#else
#define RT_WEAK __attribute__((weak))
#endif

#ifdef __cplusplus
}
#endif
