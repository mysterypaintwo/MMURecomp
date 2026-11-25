// Logging wrappers around selected lifted exports (tools/recomp/lift.py
// INTERPOSED). Each logs its arguments/result and calls the original code.
#include "recomp/runtime.h"

#include <cstdio>
#include <cstdlib>
#include <string>

extern "C" void (*rt_entry_hook)(uint32_t va);
extern "C" void rt_heap_state(uint32_t* bump, uint32_t* large, uint32_t* lists, int restore);
extern "C" void rt_k32_state(uint32_t* tls128, uint32_t* last_error);

extern "C" uint32_t rt_guest_call(uint32_t va, unsigned nargs, const uint32_t* args);
extern "C" uint32_t rt_find_export(const char* module, const char* name);

namespace {
uint32_t call_lifted(Cpu* c, const char* module, const char* name, unsigned nargs) {
    uint32_t args[8];
    for (unsigned i = 0; i < nargs; ++i) args[i] = rt_arg(c, i);
    const uint32_t r = rt_guest_call(rt_find_export(module, name), nargs, args);
    c->eax = r;
    RT_RET_CDECL(c);
    return r;
}
const char* sdl_error() {
    const uint32_t r = rt_guest_call(rt_find_export("SDL.dll", "SDL_GetError"), 0, nullptr);
    return r ? rt_str(r) : "";
}
// ---- lockstep snapshot (tools/recomp/lockstep.py)
FILE* g_trace = nullptr;
void trace_entry(uint32_t va) { fwrite(&va, 4, 1, g_trace); }

void write_region(FILE* f, uint32_t lo, uint32_t hi) {
    fwrite(&lo, 4, 1, f);
    fwrite(&hi, 4, 1, f);
    fwrite(rt_mem + lo, 1, hi - lo, f);
}

// Snapshot layout: "MMUSNAP1", Cpu struct, heap state (bump, large_free,
// 13 class lists), TLS[128], last_error, the target VA, then regions
// {lo, hi, bytes} until lo == hi == 0.
void save_snapshot(const std::string& dir, Cpu* c, uint32_t target) {
    FILE* f = fopen((dir + "/snapshot.bin").c_str(), "wb");
    if (!f) return;
    fwrite("MMUSNAP1", 1, 8, f);
    fwrite(c, sizeof(Cpu), 1, f);
    uint32_t heap[15];
    rt_heap_state(&heap[0], &heap[1], &heap[2], 0);
    fwrite(heap, 4, 15, f);
    uint32_t tls[129];
    rt_k32_state(tls, &tls[128]);
    fwrite(tls, 4, 129, f);
    fwrite(&target, 4, 1, f);
    write_region(f, 0x00001000u, 0x00002000u);  // TIB
    write_region(f, 0x00400000u, 0x006d0000u);  // MMU.exe
    write_region(f, 0x00800000u, 0x01000000u);  // stack
    write_region(f, 0x01000000u, (heap[0] + 0xfffu) & ~0xfffu);  // heap in use
    write_region(f, RT_HEAP_END, RT_HEAP_END + 0xa00000u);       // lifted DLLs
    write_region(f, RT_IMPORT_BASE, RT_IMPORT_BASE + 0x10000u);  // fake import slots
    const uint32_t zero[2] = {0, 0};
    fwrite(zero, 4, 2, f);
    fclose(f);
}

uint32_t call_traced(Cpu* c, const char* module, const char* name, unsigned nargs) {
    static const char* dir = getenv("RECOMP_LOCKSTEP");
    static bool done = false;
    if (!dir || done) return call_lifted(c, module, name, nargs);
    done = true;
    const uint32_t target = rt_find_export(module, name);
    save_snapshot(dir, c, target);
    g_trace = fopen((std::string(dir) + "/lifted_trace.bin").c_str(), "wb");
    rt_entry_hook = trace_entry;
    const uint32_t r = call_lifted(c, module, name, nargs);
    rt_entry_hook = nullptr;
    fclose(g_trace);
    fprintf(stderr, "[lockstep] snapshot + trace written to %s (result %x)\n", dir, r);
    return r;
}
} // namespace

extern "C" void imp_SDL_MIXER__Mix_LoadMUS_RW(Cpu* c) {
    const uint32_t rw = rt_arg(c, 0);
    // SDL 1.2 SDL_RWops: 4 callbacks, type, then hidden.mem {base, here, stop}.
    const uint32_t type = rt_rd32(rw + 16), base = rt_rd32(rw + 20), here = rt_rd32(rw + 24), stop = rt_rd32(rw + 28);
    if (getenv("RECOMP_TRACE_MIXER")) fprintf(stderr, "[mixer] rw type=%u base=%x here=%x stop=%x first=%.4s\n", type, base, here, stop,
            base ? (const char*)rt_mem + here : "");
    const uint32_t r = call_traced(c, "SDL_mixer.dll", "Mix_LoadMUS_RW", 1);
    if (getenv("RECOMP_TRACE_MIXER")) fprintf(stderr, "[mixer] Mix_LoadMUS_RW(rw=%x) -> %x %s\n", rw, r, r ? "" : sdl_error());
}
extern "C" void imp_SDL_MIXER__Mix_PlayMusic(Cpu* c) {
    const uint32_t m = rt_arg(c, 0), loops = rt_arg(c, 1);
    const uint32_t r = call_lifted(c, "SDL_mixer.dll", "Mix_PlayMusic", 2);
    if (getenv("RECOMP_TRACE_MIXER")) fprintf(stderr, "[mixer] Mix_PlayMusic(%x, %d) -> %d %s\n", m, (int)loops, (int)r, r ? sdl_error() : "");
}
extern "C" void imp_SDL_MIXER__Mix_VolumeMusic(Cpu* c) {
    const uint32_t v = rt_arg(c, 0);
    const uint32_t r = call_lifted(c, "SDL_mixer.dll", "Mix_VolumeMusic", 1);
    if (getenv("RECOMP_TRACE_MIXER")) fprintf(stderr, "[mixer] Mix_VolumeMusic(%d) -> %d\n", (int)v, (int)r);
}
