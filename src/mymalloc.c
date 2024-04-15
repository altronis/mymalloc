#include "mymalloc.h"
#include "largealloc.h"
#include <stdint.h>
#include "binmanager.h"

void* mymalloc(size_t size) {
    if (size == 0)
        return NULL;

    if (!is_heap_initialized)
        initialize_heap();

    if (size > max_block_size)
        return large_alloc(size);

    return heap_alloc(size);
}

void myfree(void* ptr) {
    if (ptr == NULL)
        return;

    if (is_large_alloc(ptr))
        large_free(ptr);
    else
        heap_free(ptr);
}