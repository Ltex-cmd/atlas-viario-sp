#include "alloc.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef union { max_align_t alignment; size_t size; } Header;
static size_t live, peak;
size_t bench_live_bytes(void) { return live; }
size_t bench_peak_bytes(void) { return peak; }
void bench_reset_peak(void) { peak = live; }
void *bench_malloc(size_t size) {
    if (size > SIZE_MAX - sizeof(Header)) return NULL;
    Header *block = malloc(sizeof(Header) + size);
    if (!block) return NULL;
    block->size = size;
    live += size;
    if (live > peak) peak = live;
    return block + 1;
}
void bench_free(void *pointer) {
    if (!pointer) return;
    Header *block = (Header *)pointer - 1;
    assert(live >= block->size);
    live -= block->size;
    free(block);
}
void *bench_calloc(size_t count, size_t size) {
    if (size && count > SIZE_MAX / size) return NULL;
    void *pointer = bench_malloc(count * size);
    if (pointer) memset(pointer, 0, count * size);
    return pointer;
}
void *bench_realloc(void *pointer, size_t size) {
    if (!pointer) return bench_malloc(size);
    if (!size) { bench_free(pointer); return NULL; }
    if (size > SIZE_MAX - sizeof(Header)) return NULL;
    Header *old = (Header *)pointer - 1;
    size_t previous = old->size;
    Header *block = realloc(old, sizeof(Header) + size);
    if (!block) return NULL;
    block->size = size;
    live = live - previous + size;
    if (live > peak) peak = live;
    return block + 1;
}
