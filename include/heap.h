#ifndef MYMALLOC_HEAP_H
#define MYMALLOC_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include "macros.h"
#include "binmanager.h"

typedef struct heap {
    // Recycling bin for empty superblocks.
    Superblock* recycled_superblock;

    // Bins sorted by size
    BinManager size_bins[MAX_NUM_BINS];

    // Usage statistics
    size_t in_use;
    size_t alloced;
    size_t max_in_use;
    size_t max_alloced;
} Heap;

extern Heap global_heap;
extern bool is_global_heap_initialized;

void initialize_global_heap();

// Increase/Decrease the in_use and alloced statistics of the heap
void inc_usage(Heap* heap, size_t added_usage);

void inc_alloced(Heap* heap, size_t added_alloc);

void dec_usage(Heap* heap, size_t bytes);

void dec_alloced(Heap* heap, size_t bytes);

// Allocate on the global heap
void* heap_alloc(Heap* heap, size_t size);
void heap_free(Heap* heap, void* ptr);

#endif //MYMALLOC_HEAP_H
