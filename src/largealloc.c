#define _GNU_SOURCE
#include <sys/mman.h>

#include "macros.h"
#include "heap.h"
#include "largealloc.h"

typedef struct mmap_header {
    int magic;
    size_t size;  // Size of the allocated memory
} MmapHeader;

void* large_alloc(size_t size) {
    size_t alloced_size = size + sizeof(MmapHeader);
    void* ptr = mmap(NULL, alloced_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON, -1, 0);

    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }

    DPRINT("large_alloc(): Allocated %zu bytes (%zu with header) at %p",
           size, alloced_size, ptr);

    // Place the header at the pointer
    MmapHeader* header = ptr;
    header->magic = MMAP_HEADER_MAGIC;
    header->size = size;

    return GET_BUFFER_PTR(header, MmapHeader);
}

void* large_realloc(void* ptr, size_t size) {
    MmapHeader* old_header = GET_HEADER_PTR(ptr, MmapHeader);
    size_t old_alloced_size = old_header->size + sizeof(MmapHeader);
    size_t new_alloced_size = size + sizeof(MmapHeader);

    void* new_ptr = mremap(old_header, old_alloced_size, new_alloced_size, MREMAP_MAYMOVE);
    if (new_ptr == MAP_FAILED) {
        perror("mremap failed");
        return NULL;
    }
    DPRINT("large_realloc(): Allocated %zu bytes (%zu with header) at %p",
           size, new_alloced_size, new_ptr);

    // Place the header at the pointer
    MmapHeader* new_header = new_ptr;
    new_header->magic = MMAP_HEADER_MAGIC;
    new_header->size = size;

    return GET_BUFFER_PTR(new_header, MmapHeader);
}

void large_free(void* ptr) {
    MmapHeader* header = GET_HEADER_PTR(ptr, MmapHeader);
    size_t alloced_size = header->size + sizeof(MmapHeader);
    DPRINT("large_free(): Freeing %zu bytes (%zu with header) at %p",
           header->size, alloced_size, header);

    int ret = munmap(header, alloced_size);
    if (ret == -1)
        perror("munmap failed");
}

bool is_large_alloc(void* ptr) {
    MmapHeader* header = GET_HEADER_PTR(ptr, MmapHeader);
    return header->magic == MMAP_HEADER_MAGIC;
}