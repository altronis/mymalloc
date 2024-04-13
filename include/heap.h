#ifndef MYMALLOC_HEAP_H
#define MYMALLOC_HEAP_H

#include <stddef.h>
#include <constants.h>

typedef struct heap {
    // Bins sorted by size

    // Usage statistics
    size_t in_use;
    size_t alloced;
    size_t max_in_use;
    size_t max_alloced;
} Heap;

#endif //MYMALLOC_HEAP_H
