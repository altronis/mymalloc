#include <sys/mman.h>

#include "constants.h"
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
    header->magic = LARGEALLOC_HEADER_MAGIC;
    header->size = size;

    return ptr + sizeof(MmapHeader);
}

void large_free(void* ptr) {
    MmapHeader* header = ptr - sizeof(MmapHeader);
    size_t alloced_size = header->size + sizeof(MmapHeader);
    int ret = munmap(header, alloced_size);

    if (ret == -1)
        perror("munmap failed");
}

bool is_large_alloc(void* ptr) {
    MmapHeader* header = ptr - sizeof(MmapHeader);
    return header->magic == LARGEALLOC_HEADER_MAGIC;
}