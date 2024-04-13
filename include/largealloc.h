#ifndef MYMALLOC_LARGEALLOC_H
#define MYMALLOC_LARGEALLOC_H

#include <stddef.h>

// Handles allocation for objects of size greater than S/2
void* large_alloc(size_t size);

void large_free();

#endif //MYMALLOC_LARGEALLOC_H
