#include "heap.h"
#include "macros.h"

void initialize_heap() {
    global_heap.in_use = 0;
    global_heap.alloced = 0;
    global_heap.max_in_use = 0;
    global_heap.max_alloced = 0;
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