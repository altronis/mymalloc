#ifndef MYMALLOC_LARGEALLOC_H
#define MYMALLOC_LARGEALLOC_H

#include <stddef.h>
#include <stdbool.h>

// Handles allocation for objects of size greater than S/2
void* large_alloc(size_t size);

void large_free(void* ptr);

// Determine if a pointer was allocated using large_alloc
bool is_large_alloc(void* ptr);

#endif //MYMALLOC_LARGEALLOC_H
