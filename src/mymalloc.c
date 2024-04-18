#include "mymalloc.h"
#include "largealloc.h"
#include "binmanager.h"

void* malloc(size_t size) {
    if (size == 0)
        return NULL;

    if (!size_table_initialized)
        init_size_table();

    if (size > max_block_size)
        return large_alloc(size);

    Heap* heap = get_thread_heap();
    DPRINT("Heap %p: allocating %zu bytes", heap, size);
    return heap_alloc(heap, size);
}

void free(void* ptr) {
    if (ptr == NULL)
        return;

    if (is_large_alloc(ptr))
        large_free(ptr);
    else
        heap_free(ptr);
}