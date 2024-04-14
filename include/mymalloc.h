#ifndef MYMALLOC_MYMALLOC_H
#define MYMALLOC_MYMALLOC_H

#include <stddef.h>
#include <heap.h>

void* mymalloc(size_t size);

void myfree(void* ptr);

#endif //MYMALLOC_MYMALLOC_H
