// Recompiled-guest runtime: guest memory, PE image loading, dispatch.
#include "recomp/cpu.h"
#include "recomp/runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__SWITCH__) || defined(__SWITCH2__)
/* guest memory comes from calloc (see reserve_guest) */
#elif defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#endif

uint8_t* rt_mem = NULL;
static uint64_t rt_mem_size = 0;
Cpu rt_cpu;

static uint8_t* reserve_guest(uint64_t size) {
#if defined(__SWITCH__) || defined(__SWITCH2__)
    return (uint8_t*)calloc(1, (size_t)size);
#elif defined(_WIN32)
    return (uint8_t*)VirtualAlloc(NULL, (SIZE_T)size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    void* p = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON
#ifdef MAP_NORESERVE
                       | MAP_NORESERVE
#endif
                   , -1, 0);
    return p == MAP_FAILED ? NULL : (uint8_t*)p;
#endif
}

static uint32_t rd32(const uint8_t* p) { uint32_t v; memcpy(&v, p, 4); return v; }
static uint16_t rd16(const uint8_t* p) { uint16_t v; memcpy(&v, p, 2); return v; }

static char g_game_dir[1024];
const char* rt_game_dir(void) { return g_game_dir; }

// Map one PE file at its guest base, applying base relocations when the
// guest base differs from the preferred one (the lifted code was generated
// against the same rebased image).
static int load_module(const RtModule* mod) {
    char path[1200];
    snprintf(path, sizeof(path), "%s/%s", g_game_dir, mod->file);
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "recomp: cannot open %s\n", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* file = (uint8_t*)malloc((size_t)size);
    if (!file || fread(file, 1, (size_t)size, f) != (size_t)size) {
        fclose(f);
        free(file);
        return 0;
    }
    fclose(f);
    const uint32_t pe = rd32(file + 0x3c);
    const uint32_t opt = pe + 24;
    const uint32_t base = mod->base;
    const uint32_t headers = rd32(file + opt + 60);
    memcpy(rt_mem + base, file, headers < (uint32_t)size ? headers : (uint32_t)size);
    const uint16_t nsec = rd16(file + pe + 6);
    const uint16_t optsz = rd16(file + pe + 20);
    for (uint16_t i = 0; i < nsec; ++i) {
        const uint8_t* s = file + pe + 24 + optsz + i * 40u;
        const uint32_t vsize = rd32(s + 8), va = rd32(s + 12), rsize = rd32(s + 16), raw = rd32(s + 20);
        const uint32_t n = rsize < vsize || vsize == 0 ? rsize : vsize;
        if (raw + n <= (uint32_t)size) memcpy(rt_mem + base + va, file + raw, n);
    }
    const uint32_t delta = base - mod->original_base;
    const uint32_t rel_rva = rd32(file + opt + 96 + 5 * 8), rel_size = rd32(file + opt + 96 + 5 * 8 + 4);
    if (delta && rel_rva) {
        uint32_t o = base + rel_rva;
        const uint32_t end = o + rel_size;
        while (o < end) {
            const uint32_t page = rd32(rt_mem + o), blk = rd32(rt_mem + o + 4);
            if (blk < 8) break;
            for (uint32_t k = 0; k < (blk - 8) / 2; ++k) {
                const uint16_t e = rd16(rt_mem + o + 8 + 2 * k);
                if ((e >> 12) != 3) continue;
                const uint32_t at = base + page + (e & 0xfffu);
                uint32_t v = rd32(rt_mem + at) + delta;
                memcpy(rt_mem + at, &v, 4);
            }
            o += blk;
        }
    }
    free(file);
    return 1;
}

int rt_init(const char* game_dir) {
#if defined(__SWITCH__) || defined(__SWITCH2__)
    rt_mem_size = RT_GUEST_SIZE; // no overcommit: allocate exactly the layout
#else
    rt_mem_size = 0x100000000ull; // whole 32-bit space: wild guest pointers stay inside
#endif
    rt_mem = reserve_guest(rt_mem_size);
    if (!rt_mem) {
        fprintf(stderr, "recomp: cannot reserve guest memory\n");
        return 0;
    }
    snprintf(g_game_dir, sizeof(g_game_dir), "%s", game_dir);
    for (unsigned m = 0; m < rt_module_count; ++m)
        if (!load_module(&rt_modules[m])) return 0;

    // Point every IAT slot at a fake address the dispatcher recognises.
    for (unsigned i = 0; i < rt_import_count; ++i)
        memcpy(rt_mem + rt_imports[i].iat, &(uint32_t){RT_IMPORT_BASE + i * 16u}, 4);
    for (unsigned i = 0; i < rt_binding_count; ++i)
        memcpy(rt_mem + rt_bindings[i].iat, &rt_bindings[i].value, 4);

    // Minimal TIB: SEH chain end, stack bounds, self pointer.
    const uint32_t tib = RT_FS_BASE;
    uint32_t v = 0xffffffffu;
    memcpy(rt_mem + tib + 0x00, &v, 4);
    v = RT_STACK_TOP + 16u; memcpy(rt_mem + tib + 0x04, &v, 4);
    v = 0x00800000u; memcpy(rt_mem + tib + 0x08, &v, 4);
    v = tib; memcpy(rt_mem + tib + 0x18, &v, 4);
    v = 0x1000u; memcpy(rt_mem + tib + 0x20, &v, 4);  // process id
    v = 0x1004u; memcpy(rt_mem + tib + 0x24, &v, 4);  // thread id

    memset(&rt_cpu, 0, sizeof(rt_cpu));
    rt_cpu.esp = RT_STACK_TOP;
    rt_cpu.fcw = 0x027f; // Windows default: 53-bit precision, round-nearest, masked
    rt_cpu.mxcsr = 0x1f80;
    rt_heap_init();
    return 1;
}

RtFn rt_lookup(uint32_t va) {
    unsigned lo = 0, hi = rt_function_count;
    while (lo < hi) {
        const unsigned mid = (lo + hi) / 2;
        if (rt_functions[mid].va < va) lo = mid + 1;
        else hi = mid;
    }
    return (lo < rt_function_count && rt_functions[lo].va == va) ? rt_functions[lo].fn : NULL;
}

void rt_call_indirect(Cpu* c, uint32_t target) {
    if (target >= RT_IMPORT_BASE && target < RT_IMPORT_BASE + rt_import_count * 16u) {
        rt_imports[(target - RT_IMPORT_BASE) / 16u].fn(c);
        return;
    }
    RtFn fn = rt_lookup(target);
    if (!fn) {
        char msg[96];
        // Record it so tools/recomp/extra_entries.txt can be extended.
        FILE* log = fopen("recomp_missing_entries.txt", "a");
        if (log) { fprintf(log, "0x%x\n", target); fclose(log); }
        snprintf(msg, sizeof(msg), "indirect call to unknown target 0x%x", target);
        rt_fault(target, msg);
        return;
    }
    fn(c);
}

// Indirect jump. A target that is a function entry is a tail call. Anything
// else is a "return by jump": e.g. GCC/MinGW's ___chkstk pops its return
// address, moves ESP and `jmp eax`s back to the call site. The lifted caller
// resumes right after its C call, i.e. at that exact address, with the guest
// stack already adjusted, so returning here reproduces the original.
void rt_jump_indirect(Cpu* c, uint32_t target) {
    if (target >= RT_IMPORT_BASE && target < RT_IMPORT_BASE + rt_import_count * 16u) {
        rt_imports[(target - RT_IMPORT_BASE) / 16u].fn(c);
        return;
    }
    RtFn fn = rt_lookup(target);
    if (fn) {
        fn(c);
        return;
    }
    (void)c; // return-by-jump: nothing to call
}

void rt_cpuid(Cpu* c) {
    // A plain SSE2-capable Intel CPU (EDX bits 25/26), like any machine that
    // runs the game on Windows today, so msvcr90 picks the same SSE2 paths.
    switch (c->eax) {
    case 0: c->eax = 1; c->ebx = 0x756e6547u; c->edx = 0x49656e69u; c->ecx = 0x6c65746eu; break;
    default: c->eax = 0x000006fbu; c->ebx = 0; c->ecx = 0x00000001u; c->edx = 0x0789fbffu; break;
    }
}

uint32_t rt_trace_va[64], rt_trace_esp[64];
void (*rt_entry_hook)(uint32_t va) = NULL;
uint32_t rt_trace_pos;

// Recent guest calls + last synced guest registers (crash logs, rt_fault).
// Registers are cached in C locals inside lifted functions, so rt_cpu shows
// the state at the last call/import boundary; the call ring is exact.
void rt_dump_state(void) {
    fprintf(stderr, "  recent guest calls (oldest first):\n");
    for (uint32_t k = 0; k < 64; ++k) {
        const uint32_t p = (rt_trace_pos + k) & 63u;
        if (rt_trace_va[p]) fprintf(stderr, "    f_%x esp=%08x\n", rt_trace_va[p], rt_trace_esp[p]);
    }
    fprintf(stderr, "  eax=%08x ecx=%08x edx=%08x ebx=%08x esp=%08x ebp=%08x esi=%08x edi=%08x\n",
            rt_cpu.eax, rt_cpu.ecx, rt_cpu.edx, rt_cpu.ebx, rt_cpu.esp, rt_cpu.ebp, rt_cpu.esi,
            rt_cpu.edi);
    fflush(stderr);
}

void rt_fault(uint32_t va, const char* what) {
    fprintf(stderr, "recomp fault at 0x%08x: %s\n", va, what);
    rt_dump_state();
    fflush(stdout);
    abort();
}

void rt_missing_import(const char* dll, const char* name) {
    fprintf(stderr, "recomp: missing import %s!%s\n", dll, name);
    rt_fault(0, "missing import");
}

void rt_memmove_guest(uint32_t dst, uint32_t src, uint32_t n) { memmove(rt_mem + dst, rt_mem + src, n); }

void rt_stos(uint32_t dst, uint32_t value, uint32_t count, int unit) {
    for (uint32_t i = 0; i < count; ++i) {
        if (unit == 4) memcpy(rt_mem + dst + i * 4u, &value, 4);
        else rt_mem[dst + i] = (uint8_t)value;
    }
}

int rt_run_entry(void) {
    Cpu* c = &rt_cpu;
    // DllMain(hinst, DLL_PROCESS_ATTACH, static-load) for each lifted DLL in
    // dependency order (msvcr90 before msvcp90), as the Windows loader does.
    for (unsigned m = 0; m < rt_module_count; ++m) {
        if (!rt_modules[m].is_dll) continue;
        uint32_t args[4] = {0, rt_modules[m].base, 1, 1};
        c->esp -= 16;
        memcpy(rt_mem + c->esp, args, 16);
        rt_call_indirect(c, rt_modules[m].entry); // stdcall: callee pops args
        if (!c->eax) {
            fprintf(stderr, "recomp: %s DllMain failed\n", rt_modules[m].file);
            return 1;
        }
    }
    c->esp -= 4;
    memset(rt_mem + c->esp, 0, 4); // returning from the entry point ends the process
    rt_call_indirect(c, rt_entry_point);
    return (int)c->eax;
}

uint32_t rt_shim_address(const char* name) {
    for (unsigned i = 0; i < rt_import_count; ++i)
        if (strcmp(rt_imports[i].name, name) == 0) return RT_IMPORT_BASE + i * 16u;
    return 0;
}

// Debug aid: functions whose `ret` runs with a different ESP than on entry.
// The CRT's _SEH_prolog4/_SEH_epilog4 helpers legitimately do this.
void rt_stack_note(uint32_t fn, uint32_t esp0, uint32_t esp) {
    static uint32_t seen[256];
    static unsigned count;
    for (unsigned i = 0; i < count; ++i)
        if (seen[i] == fn) return;
    if (count < 256) seen[count++] = fn;
    if (getenv("RECOMP_STACK_NOTES"))
        fprintf(stderr, "recomp: f_%x returns with esp %08x (entered %08x)\n", fn, esp, esp0);
}

// Call a guest function from host code (cdecl or stdcall: ESP is restored
// either way). Caller-saved registers may be clobbered, as on x86.
uint32_t rt_guest_call(uint32_t va, unsigned nargs, const uint32_t* args) {
    Cpu* c = &rt_cpu;
    const uint32_t saved_esp = c->esp;
    c->esp -= 4u * nargs;
    for (unsigned i = 0; i < nargs; ++i) memcpy(rt_mem + c->esp + 4u * i, &args[i], 4);
    c->esp -= 4;
    memset(rt_mem + c->esp, 0, 4);
    rt_call_indirect(c, va);
    c->esp = saved_esp;
    return c->eax;
}
