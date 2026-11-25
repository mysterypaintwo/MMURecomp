// Inline helpers used by generated code. Keep semantics identical to x86.
#pragma once

#include "recomp/cpu.h"

#include <math.h>
#include <string.h>

#define SYNC_IN  do { eax=c->eax; ecx=c->ecx; edx=c->edx; ebx=c->ebx; esp=c->esp; ebp=c->ebp; esi=c->esi; edi=c->edi; } while (0)
#define SYNC_OUT do { c->eax=eax; c->ecx=ecx; c->edx=edx; c->ebx=ebx; c->esp=esp; c->ebp=ebp; c->esi=esi; c->edi=edi; } while (0)

#define RT_FAULT(va, what) rt_fault((va), (what))
// Call history ring (printed by rt_fault) for debugging the recompilation.
extern uint32_t rt_trace_va[64], rt_trace_esp[64];
extern uint32_t rt_trace_pos;
void rt_stack_note(uint32_t fn, uint32_t esp0, uint32_t esp);
#define RT_RET_CHECK(fn) do { if (esp != esp0_) rt_stack_note((fn), esp0_, esp); \
    if (esp - 0x00800000u > 0x00800000u) { SYNC_OUT; rt_fault((fn), "ret with esp outside the guest stack"); } } while (0)
extern void (*rt_entry_hook)(uint32_t va);
#define RT_ENTER(va) do { uint32_t p_ = rt_trace_pos++ & 63u; rt_trace_va[p_] = (va); rt_trace_esp[p_] = esp; \
    if (rt_entry_hook) rt_entry_hook(va); } while (0)
#define RT_UNSUPPORTED(va, what) rt_fault((va), "unsupported: " what)

static inline uint8_t R8(uint32_t a) { return rt_mem[a]; }
static inline uint16_t R16(uint32_t a) { uint16_t v; memcpy(&v, rt_mem + a, 2); return v; }
static inline uint32_t R32(uint32_t a) { uint32_t v; memcpy(&v, rt_mem + a, 4); return v; }
static inline uint64_t R64(uint32_t a) { uint64_t v; memcpy(&v, rt_mem + a, 8); return v; }
static inline void W8(uint32_t a, uint32_t v) { rt_mem[a] = (uint8_t)v; }
static inline void W16(uint32_t a, uint32_t v) { uint16_t t = (uint16_t)v; memcpy(rt_mem + a, &t, 2); }
static inline void W32(uint32_t a, uint32_t v) { memcpy(rt_mem + a, &v, 4); }
static inline void W64(uint32_t a, uint64_t v) { memcpy(rt_mem + a, &v, 8); }

static inline double RF32(uint32_t a) { float f; memcpy(&f, rt_mem + a, 4); return (double)f; }
static inline double RF64(uint32_t a) { double d; memcpy(&d, rt_mem + a, 8); return d; }
static inline void WF32(uint32_t a, double v) { float f = (float)v; memcpy(rt_mem + a, &f, 4); }
static inline void WF64(uint32_t a, double v) { memcpy(rt_mem + a, &v, 8); }

// 80-bit extended <-> double (values stay within double range in practice).
static inline double RF80(uint32_t a) {
    uint64_t mant = R64(a);
    uint16_t se = R16(a + 8);
    int exp = se & 0x7fff;
    double sign = (se & 0x8000) ? -1.0 : 1.0;
    if (exp == 0 && mant == 0) return sign * 0.0;
    if (exp == 0x7fff) return (mant << 1) ? NAN : sign * INFINITY;
    return sign * ldexp((double)mant, exp - 16383 - 63);
}
static inline void WF80(uint32_t a, double v) {
    uint16_t se = signbit(v) ? 0x8000 : 0;
    uint64_t mant = 0;
    if (isnan(v)) { se |= 0x7fff; mant = 0xc000000000000000ull; }
    else if (isinf(v)) { se |= 0x7fff; mant = 0x8000000000000000ull; }
    else if (v != 0.0) {
        int e;
        double m = frexp(fabs(v), &e); // [0.5,1)
        mant = (uint64_t)ldexp(m, 64);
        se |= (uint16_t)(e - 1 + 16383);
    }
    W64(a, mant);
    W16(a + 8, se);
}

static inline int PARITY8(uint32_t v) {
    v &= 0xff; v ^= v >> 4; v ^= v >> 2; v ^= v >> 1;
    return (int)(~v & 1u);
}

// x87 register stack. ST(i) is physical register (TOP+i)&7.
#define ST(i) (c->st[(c->top + (i)) & 7u])
#define FPUSH(v) do { double v_ = (v); c->top = (c->top - 1u) & 7u; c->st[c->top] = v_; } while (0)
#define FPOP() do { c->top = (c->top + 1u) & 7u; } while (0)

static inline void rt_fcom(Cpu* c, double a, double b) {
    if (isnan(a) || isnan(b)) { c->c3 = 1; c->c2 = 1; c->c0 = 1; }
    else if (a > b) { c->c3 = 0; c->c2 = 0; c->c0 = 0; }
    else if (a < b) { c->c3 = 0; c->c2 = 0; c->c0 = 1; }
    else { c->c3 = 1; c->c2 = 0; c->c0 = 0; }
}
static inline uint32_t rt_fsw(Cpu* c) {
    return ((uint32_t)c->c3 << 14) | ((c->top & 7u) << 11) | ((uint32_t)c->c2 << 10) |
           ((uint32_t)c->c1 << 9) | ((uint32_t)c->c0 << 8);
}

// FIST/FISTP: round per FCW.RC; out-of-range stores the integer indefinite.
static inline double rt_fround(Cpu* c, double v) {
    switch ((c->fcw >> 10) & 3u) {
    case 0: return nearbyint(v); // round to nearest even (default env)
    case 1: return floor(v);
    case 2: return ceil(v);
    default: return trunc(v);
    }
}
static inline int16_t rt_fist16(Cpu* c, double v) {
    double r = rt_fround(c, v);
    return (r >= -32768.0 && r <= 32767.0) ? (int16_t)r : (int16_t)0x8000;
}
static inline int32_t rt_fist32(Cpu* c, double v) {
    double r = rt_fround(c, v);
    return (r >= -2147483648.0 && r <= 2147483647.0) ? (int32_t)r : (int32_t)0x80000000u;
}
static inline int64_t rt_fist64(Cpu* c, double v) {
    double r = rt_fround(c, v);
    return (r >= -9223372036854775808.0 && r < 9223372036854775808.0) ? (int64_t)r
                                                                       : (int64_t)0x8000000000000000ull;
}
static inline uint32_t rt_cvtt_i32(double v) {
    return (v > -2147483649.0 && v < 2147483648.0) ? (uint32_t)(int32_t)v : 0x80000000u;
}

// FXAM: classify ST(0) into C3/C2/C0 (+C1 = sign). Empty registers are not
// tracked, so ST(0) is always classified as a value.
static inline void rt_fxam(Cpu* c) {
    double v = ST(0);
    c->c1 = signbit(v) ? 1 : 0;
    if (isnan(v)) { c->c3 = 0; c->c2 = 0; c->c0 = 1; }
    else if (isinf(v)) { c->c3 = 0; c->c2 = 1; c->c0 = 1; }
    else if (v == 0.0) { c->c3 = 1; c->c2 = 0; c->c0 = 0; }
    else if (fpclassify(v) == FP_SUBNORMAL) { c->c3 = 1; c->c2 = 1; c->c0 = 0; }
    else { c->c3 = 0; c->c2 = 1; c->c0 = 0; }
}
// 28-byte protected-mode FPU environment.
static inline void rt_fnstenv(Cpu* c, uint32_t a) {
    W32(a + 0, 0xffff0000u | c->fcw);
    W32(a + 4, 0xffff0000u | rt_fsw(c) | c->fsw_exc);
    W32(a + 8, 0xffffffffu); // all registers tagged empty
    for (int k = 12; k < 28; k += 4) W32(a + (uint32_t)k, 0);
}
static inline void rt_fldenv(Cpu* c, uint32_t a) {
    c->fcw = R16(a);
    uint16_t sw = R16(a + 4);
    c->fsw_exc = sw & 0x3f;
    c->c0 = (sw >> 8) & 1; c->c1 = (sw >> 9) & 1; c->c2 = (sw >> 10) & 1; c->c3 = (sw >> 14) & 1;
}

// SSE2 helpers.
static inline double rt_round_mxcsr(Cpu* c, double v) {
    switch ((c->mxcsr >> 13) & 3u) {
    case 0: return nearbyint(v);
    case 1: return floor(v);
    case 2: return ceil(v);
    default: return trunc(v);
    }
}
static inline uint32_t rt_cvt_i32(Cpu* c, double v) {
    double r = rt_round_mxcsr(c, v);
    return (r >= -2147483648.0 && r <= 2147483647.0) ? (uint32_t)(int32_t)r : 0x80000000u;
}
static inline uint64_t rt_cmp_mask(double a, double b, int pred) {
    int r;
    const int un = isnan(a) || isnan(b);
    switch (pred & 7) {
    case 0: r = !un && a == b; break;
    case 1: r = !un && a < b; break;
    case 2: r = !un && a <= b; break;
    case 3: r = un; break;
    case 4: r = un || a != b; break;
    case 5: r = un || !(a < b); break;
    case 6: r = un || !(a <= b); break;
    default: r = !un; break;
    }
    return r ? ~0ull : 0ull;
}
static inline double rt_minsd(double a, double b) { return (a < b) ? a : b; } /* returns b on NaN/equal */
static inline double rt_maxsd(double a, double b) { return (a > b) ? a : b; }

static inline Xmm rt_ldx(uint32_t a, int n) { Xmm x; memset(&x, 0, 16); memcpy(&x, rt_mem + a, (size_t)n); return x; }
static inline void rt_stx(uint32_t a, const Xmm* x, int n) { memcpy(rt_mem + a, x, (size_t)n); }
static inline uint32_t rt_pmovmskb(const Xmm* x) {
    uint32_t m = 0;
    for (int k = 0; k < 16; ++k) m |= (uint32_t)(x->b[k] >> 7) << k;
    return m;
}
static inline void rt_pshift_bytes(Xmm* x, uint32_t n, int right) {
    Xmm r; memset(&r, 0, 16);
    if (n < 16) {
        if (right) memcpy(r.b, x->b + n, 16 - n);
        else memcpy(r.b + n, x->b, 16 - n);
    }
    *x = r;
}

// FNSAVE/FRSTOR: 108-byte image (environment + 8 x 80-bit registers), then
// FNSAVE reinitialises the FPU like FNINIT.
static inline void rt_fnsave(Cpu* c, uint32_t a) {
    rt_fnstenv(c, a);
    for (int k = 0; k < 8; ++k) WF80(a + 28u + 10u * (uint32_t)k, ST(k));
    c->fcw = 0x037f; c->top = 0; c->c0 = c->c1 = c->c2 = c->c3 = 0; c->fsw_exc = 0;
}
static inline void rt_frstor(Cpu* c, uint32_t a) {
    rt_fldenv(c, a);
    c->top = (R16(a + 4) >> 11) & 7u;
    for (int k = 0; k < 8; ++k) ST(k) = RF80(a + 28u + 10u * (uint32_t)k);
}
