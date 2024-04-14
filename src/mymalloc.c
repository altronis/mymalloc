#include "mymalloc.h"
#include "largealloc.h"
#include "macros.h"

void* mymalloc(size_t size) {
    if (size == 0)
        return NULL;

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