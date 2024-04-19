#ifndef MYMALLOC_MYMALLOC_H
#define MYMALLOC_MYMALLOC_H

#include <stddef.h>
#include <heap.h>

void* malloc(size_t size);

void* calloc(size_t nmemb, size_t size);

void free(void* ptr);

#endif //MYMALLOC_MYMALLOC_H
