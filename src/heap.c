#include "heap.h"
#include "macros.h"

static Heap global_heap;
bool is_heap_initialized = false;

void initialize_heap() {
    global_heap.in_use = 0;
    global_heap.alloced = 0;
    global_heap.max_in_use = 0;
    global_heap.max_alloced = 0;

    init_size_table();
    is_heap_initialized = true;
}

void inc_usage(size_t added_usage) {
    global_heap.in_use += added_usage;
    if (global_heap.in_use > global_heap.max_in_use)
        global_heap.max_in_use = global_heap.in_use;
}

void inc_alloced(size_t added_alloc) {
    global_heap.alloced += added_alloc;
    if (global_heap.alloced > global_heap.max_alloced)
        global_heap.max_alloced = global_heap.alloced;
}

void dec_usage(size_t bytes) {
    global_heap.in_use -= bytes;
}

void dec_alloced(size_t bytes) {
    global_heap.alloced -= bytes;
}

void* heap_alloc(size_t size) {
    int bin_idx = size2idx(size);
    size_t size_class = idx2class(bin_idx);
    DPRINT("Allocating %zu bytes on bin %d (size class = %zu)", size, bin_idx, size_class);

    return NULL;
}

void heap_free(void* ptr) {

}