// Helpers for writing import shims (recomp/src/imports_*.c[pp]).
//
// Calling convention as seen by a shim: the lifted caller has already pushed
// the return address, so argument n is at [esp + 4 + 4n]. A shim must:
//   * cdecl  (CRT, SDL, ...):   pop only the return address  -> RT_RET_CDECL
//   * stdcall (WINAPI, OpenGL): pop return address + args     -> RT_RET_STDCALL(n)
//   * thiscall (MSVCP90 members): `this` in ECX, callee pops args
// and leave integer results in EAX(/EDX) and float results in ST(0).
#pragma once

#include "recomp/cpu.h"

#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern Cpu rt_cpu;

void rt_heap_init(void);
uint32_t rt_malloc(uint32_t size);
uint32_t rt_calloc(uint32_t count, uint32_t size);
uint32_t rt_realloc(uint32_t ptr, uint32_t size);
void rt_free(uint32_t ptr);
uint32_t rt_msize(uint32_t ptr);

// Guest string helpers.
uint32_t rt_strdup_guest(const char* text);

#ifdef __cplusplus
}
#endif

static inline uint32_t rt_arg(Cpu* c, unsigned n) {
    uint32_t v;
    memcpy(&v, rt_mem + c->esp + 4u + 4u * n, 4);
    return v;
}
static inline float rt_argf(Cpu* c, unsigned n) {
    float v;
    memcpy(&v, rt_mem + c->esp + 4u + 4u * n, 4);
    return v;
}
static inline double rt_argd(Cpu* c, unsigned n) { // double occupying args n, n+1
    double v;
    memcpy(&v, rt_mem + c->esp + 4u + 4u * n, 8);
    return v;
}
static inline void* rt_ptr(uint32_t guest) { return guest ? (void*)(rt_mem + guest) : NULL; }
static inline const char* rt_str(uint32_t guest) { return guest ? (const char*)(rt_mem + guest) : NULL; }
static inline uint32_t rt_rd32(uint32_t a) { uint32_t v; memcpy(&v, rt_mem + a, 4); return v; }
static inline void rt_wr32(uint32_t a, uint32_t v) { memcpy(rt_mem + a, &v, 4); }

#define RT_RET_CDECL(c) ((c)->esp += 4u)
#define RT_RET_STDCALL(c, nargs) ((c)->esp += 4u + 4u * (nargs))
// Float results go on the x87 stack like MSVC's cdecl float returns.
#define RT_RET_ST0(c, v) do { (c)->top = ((c)->top - 1u) & 7u; (c)->st[(c)->top] = (double)(v); } while (0)
