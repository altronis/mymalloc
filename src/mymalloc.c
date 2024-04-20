#include <stdint.h>
#include <string.h>
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

void* calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0)
        return NULL;

    if (size > (SIZE_MAX / nmemb))  // Integer overflow
        return NULL;

    size_t num_bytes = nmemb * size;
    void* ptr = malloc(num_bytes);

    if (ptr == NULL)
        return NULL;

    // Set memory to 0
    memset(ptr, 0, num_bytes);
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (ptr == NULL)
        return NULL;

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    bool old_large = is_large_alloc(ptr);
    bool new_large = size > max_block_size;

    // Large size -> large size
    if (old_large && new_large) {
        return large_realloc(ptr, size);
    }
    // All other cases
    void* new_ptr = malloc(size);
    if (new_ptr == NULL) {
        free(ptr);
        return NULL;
    }

    // Copy memory, free old memory
    size_t num_bytes_to_copy;
    if (old_large)   // This means old_size > new_size
        num_bytes_to_copy = size;
    else { // min(old_size, new_size)
        size_t old_size = get_block_size(ptr);
        num_bytes_to_copy = old_size < size ? old_size : size;
    }

    memcpy(new_ptr, ptr, num_bytes_to_copy);
    free(ptr);
    return new_ptr;
}

void free(void* ptr) {
    DPRINT("Freeing %p...", ptr);
    if (ptr == NULL)
        return;

    if (is_large_alloc(ptr))
        large_free(ptr);
    else
        heap_free(ptr);
}