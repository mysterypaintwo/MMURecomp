// Guest heap backing msvcr90 malloc/new and kernel32 Heap*/Local*/Global*.
//
// Simple size-class free lists with a bump region; blocks carry an 8-byte
// header {size, magic}. Addresses differ from Windows' heap (as they do run to
// run on Windows), contents are zero-filled on first use.
#include "recomp/runtime.h"

#include <stdio.h>

#define HDR 16u
#define MAGIC 0x4d4d5548u /* "HUMM" */
#define CLASSES 13          /* 16 B .. 64 KiB size classes */
#define LARGE_MIN (16u << (CLASSES - 1))
static uint32_t large_free;  /* singly linked list of free large blocks */

static uint32_t bump;
static uint32_t free_list[CLASSES]; // guest addresses of free blocks (header)

static unsigned class_of(uint32_t size) {
    unsigned k = 0;
    uint32_t cap = 16;
    while (cap < size && k + 1 < CLASSES) { cap <<= 1; ++k; }
    return k;
}
static uint32_t class_size(unsigned k) { return 16u << k; }

void rt_heap_init(void) {
    bump = RT_HEAP_BASE;
    large_free = 0;
    memset(free_list, 0, sizeof(free_list));
}

static uint32_t alloc_large(uint32_t size) {
    const uint32_t want = (size + 0xfffu) & ~0xfffu;
    uint32_t prev = 0, b = large_free;
    while (b) { /* first fit */
        if (rt_rd32(b) >= want) {
            const uint32_t next = rt_rd32(b + 8);
            if (prev) rt_wr32(prev + 8, next); else large_free = next;
            return b;
        }
        prev = b;
        b = rt_rd32(b + 8);
    }
    if (bump + want + HDR > RT_HEAP_END) return 0;
    b = bump;
    bump += want + HDR;
    rt_wr32(b, want);
    return b;
}

uint32_t rt_malloc(uint32_t size) {
    if (size == 0) size = 1;
    if (size > LARGE_MIN) {
        const uint32_t block = alloc_large(size);
        if (!block) {
            fprintf(stderr, "recomp: guest heap exhausted (%u bytes)\n", size);
            rt_log_memory("guest heap exhausted");
            return 0;
        }
        rt_wr32(block + 4, MAGIC);
        memset(rt_mem + block + HDR, 0, size);
        rt_wr32(block + 12, size);
        return block + HDR;
    }
    const unsigned k = class_of(size);
    const uint32_t cap = class_size(k);
    uint32_t block;
    if (cap >= size && free_list[k]) {
        block = free_list[k];
        free_list[k] = rt_rd32(block + 8);
    } else {
        const uint32_t want = (cap >= size ? cap : ((size + 15u) & ~15u)) + HDR;
        if (bump + want > RT_HEAP_END) {
            fprintf(stderr, "recomp: guest heap exhausted (%u bytes)\n", size);
            rt_log_memory("guest heap exhausted");
            return 0;
        }
        block = bump;
        bump += want;
        rt_wr32(block, want - HDR);
    }
    rt_wr32(block + 4, MAGIC);
    memset(rt_mem + block + HDR, 0, rt_rd32(block));
    rt_wr32(block + 12, size);
    return block + HDR;
}

uint32_t rt_calloc(uint32_t count, uint32_t size) { return rt_malloc(count * size); }

uint32_t rt_msize(uint32_t ptr) { return ptr ? rt_rd32(ptr - HDR + 12) : 0; }

void rt_free(uint32_t ptr) {
    if (!ptr) return;
    const uint32_t block = ptr - HDR;
    if (rt_rd32(block + 4) != MAGIC) {
        fprintf(stderr, "recomp: free of non-heap pointer 0x%x\n", ptr);
        return;
    }
    rt_wr32(block + 4, 0);
    const uint32_t cap = rt_rd32(block);
    if (cap > LARGE_MIN) {
        rt_wr32(block + 8, large_free);
        large_free = block;
        return;
    }
    const unsigned k = class_of(cap);
    rt_wr32(block + 8, free_list[k]);
    free_list[k] = block;
}

uint32_t rt_realloc(uint32_t ptr, uint32_t size) {
    if (!ptr) return rt_malloc(size);
    if (!size) { rt_free(ptr); return 0; }
    const uint32_t old = rt_msize(ptr);
    if (rt_rd32(ptr - HDR) >= size) { rt_wr32(ptr - HDR + 12, size); return ptr; }
    const uint32_t n = rt_malloc(size);
    if (n) memcpy(rt_mem + n, rt_mem + ptr, old < size ? old : size);
    rt_free(ptr);
    return n;
}

uint32_t rt_strdup_guest(const char* text) {
    const uint32_t n = (uint32_t)strlen(text) + 1u;
    const uint32_t p = rt_malloc(n);
    if (p) memcpy(rt_mem + p, text, n);
    return p;
}

// Lockstep snapshots (recomp/src/imports_debug.cpp) save/restore allocator state.
void rt_heap_state(uint32_t* out_bump, uint32_t* out_large, uint32_t* out_lists, int restore) {
    if (restore) {
        bump = *out_bump;
        large_free = *out_large;
        memcpy(free_list, out_lists, sizeof(free_list));
    } else {
        *out_bump = bump;
        *out_large = large_free;
        memcpy(out_lists, free_list, sizeof(free_list));
    }
}

uint32_t rt_heap_used(void) { return bump - RT_HEAP_BASE; }
