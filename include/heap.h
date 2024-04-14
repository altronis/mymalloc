#ifndef MYMALLOC_HEAP_H
#define MYMALLOC_HEAP_H

#include <stddef.h>

typedef struct heap {
    // Bins sorted by size

    // Usage statistics
    size_t in_use;
    size_t alloced;
    size_t max_in_use;
    size_t max_alloced;
} Heap;

extern Heap global_heap;

void initialize_heap();

void inc_usage(size_t added_usage);

void inc_alloced(size_t added_alloc);

#endif //MYMALLOC_HEAP_H
