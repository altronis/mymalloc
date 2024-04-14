#include "mymalloc.h"
#include "largealloc.h"
#include "macros.h"

bool is_heap_initialized = false;
Heap global_heap;

void* mymalloc(size_t size) {
    if (size == 0)
        return NULL;

    if (!is_heap_initialized) {
        initialize_heap();
    }

    if (size > SUPERBLOCK_SIZE / 2)
        return large_alloc(size);

    return NULL;
}

void myfree(void* ptr) {
    if (ptr == NULL)
        return;

    if (is_large_alloc(ptr))
        return large_free(ptr);
}