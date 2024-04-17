#ifndef MYMALLOC_HEAP_H
#define MYMALLOC_HEAP_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include "macros.h"
#include "binmanager.h"

typedef struct heap {
    // Recycling bin for empty superblocks.
    Superblock* recycled_superblock;

    // Bins sorted by size
    BinManager size_bins[MAX_NUM_BINS];

    // Lock for the heap
    pthread_mutex_t mutex;

    // Usage statistics
    size_t in_use;
    size_t alloced;
    size_t max_in_use;
    size_t max_alloced;
} Heap;

extern Heap global_heap;
extern Heap thread_heaps[MAX_HEAPS];

// Get the heap ID of the calling thread based on the TID.
Heap* get_thread_heap();

// Increase/Decrease the in_use and alloced statistics of the heap
void inc_usage(Heap* heap, size_t added_usage);

void inc_alloced(Heap* heap, size_t added_alloc);

void dec_usage(Heap* heap, size_t bytes);

void dec_alloced(Heap* heap, size_t bytes);

// Actual allocation/free functions
void* heap_alloc(Heap* heap, size_t size);
void heap_free(void* ptr);

#endif //MYMALLOC_HEAP_H
