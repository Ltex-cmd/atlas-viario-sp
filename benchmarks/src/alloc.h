#ifndef ATLAS_BENCH_ALLOC_H
#define ATLAS_BENCH_ALLOC_H
#include <stdlib.h>
#include <stddef.h>
void *bench_malloc(size_t size);
void *bench_calloc(size_t count, size_t size);
void *bench_realloc(void *pointer, size_t size);
void bench_free(void *pointer);
size_t bench_live_bytes(void);
size_t bench_peak_bytes(void);
void bench_reset_peak(void);
#ifdef ATLAS_TRACK_ALLOCATIONS
#define malloc bench_malloc
#define calloc bench_calloc
#define realloc bench_realloc
#define free bench_free
#endif
#endif
