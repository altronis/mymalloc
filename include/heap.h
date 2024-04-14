#ifndef MYMALLOC_HEAP_H
#define MYMALLOC_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include "macros.h"
#include "binmanager.h"

typedef struct heap {
    // Bins sorted by size
    BinManager size_bins[MAX_NUM_BINS];

    // Usage statistics
    size_t in_use;
    size_t alloced;
    size_t max_in_use;
    size_t max_alloced;
} Heap;

extern bool is_heap_initialized;

void initialize_heap();

// Increase/Decrease the in_use and alloced statistics of the heap
void inc_usage(size_t added_usage);

void inc_alloced(size_t added_alloc);

void dec_usage(size_t bytes);

void dec_alloced(size_t bytes);

// Allocate on the global heap
void* heap_alloc(size_t size);
void heap_free(void* ptr);

#endif //MYMALLOC_HEAP_H
