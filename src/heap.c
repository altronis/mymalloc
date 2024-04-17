#include <stdint.h>
#include "heap.h"
#include "macros.h"

#define HEAP_INITIALIZER { \
    .mutex = PTHREAD_MUTEX_INITIALIZER, \
}

Heap global_heap = HEAP_INITIALIZER;
Heap thread_heaps[MAX_HEAPS] = {HEAP_INITIALIZER};

Heap* get_thread_heap() {
    pthread_t tid = pthread_self();
    unsigned long idx = (tid * 11400714819323198485UL) >> (64 - LG_MAX_HEAPS);  // Knuth hash
    ASSERT(idx >= 0 && idx < MAX_HEAPS);
    return &thread_heaps[idx];
}

void inc_usage(Heap* heap, size_t added_usage) {
    heap->in_use += added_usage;
    if (heap->in_use > heap->max_in_use)
        heap->max_in_use = heap->in_use;
}

void inc_alloced(Heap* heap, size_t added_alloc) {
    heap->alloced += added_alloc;
    if (heap->alloced > heap->max_alloced)
        heap->max_alloced = heap->alloced;
}

void dec_usage(Heap* heap, size_t bytes) {
    heap->in_use -= bytes;
}

void dec_alloced(Heap* heap, size_t bytes) {
    heap->alloced -= bytes;
}

// Actual allocation/free functions.
void* bin_alloc(Heap* heap, BinManager* bin_manager, size_t size_class) {
    Superblock* s_ptr = NULL;  // Ptr to the superblock we are allocating from

    // Start looking from bin 1 (fullest to emptiest)
    for (int eidx = 1; eidx < NUM_EMPTINESS_CLASSES; eidx++) {
        s_ptr = bin_manager->emptiness_bins[eidx];
        if (s_ptr != NULL) {
            DPRINT("    Allocating from emptiness class %d at superblock %p", eidx, s_ptr);
            break;
        }
    }
    // Recycle an empty superblock, if there is any.
    Superblock* recycled_superblock = heap->recycled_superblock;

    if (recycled_superblock != NULL) {
        s_ptr = recycled_superblock;
        reset_superblock(s_ptr, size_class);

        heap->recycled_superblock = recycled_superblock->header.next;
        push_into_bin(bin_manager, NUM_EMPTINESS_CLASSES - 1, s_ptr);
        DPRINT("    Allocating from recycled superblock %p", s_ptr);
    }
    // None found in this bin. Allocate a new superblock.
    if (s_ptr == NULL) {
        s_ptr = init_superblock(size_class);
        if (s_ptr == NULL)
            return NULL;

        // Add it to the emptiest class.
        s_ptr->header.owner = heap;
        inc_alloced(heap, SUPERBLOCK_SIZE);
        push_into_bin(bin_manager, NUM_EMPTINESS_CLASSES - 1, s_ptr);
    }
    // Allocate from the superblock.
    unsigned int old_eidx = get_eidx(s_ptr);
    void* ret_ptr = superblock_alloc(s_ptr);

    ASSERT(ret_ptr != NULL);
    update_emptiness_class(bin_manager, old_eidx, s_ptr);
    return ret_ptr;
}

void bin_free(Heap* heap, BinManager* bin_manager, void* ptr) {
    // Find the superblock the ptr resides in.
    Superblock* s_ptr = (Superblock*) ((uintptr_t) ptr & ~(SUPERBLOCK_SIZE - 1));

    // Free it from the superblock.
    unsigned int old_eidx = get_eidx(s_ptr);
    superblock_free(s_ptr, ptr);

    // Update the superblock's emptiness class.
    if (is_superblock_empty(s_ptr)) {
        delete_from_bin(bin_manager, old_eidx, s_ptr);

        DPRINT("    Superblock %p is now empty. Recycling it", s_ptr);
        Superblock* recycle_list_head = heap->recycled_superblock;

        if (recycle_list_head == NULL)
            s_ptr->header.next = NULL;
        else
            s_ptr->header.next = recycle_list_head;
        heap->recycled_superblock = s_ptr;

    } else {
        update_emptiness_class(bin_manager, old_eidx, s_ptr);
    }
}

void* heap_alloc(Heap* heap, size_t size) {
    int bin_idx = size2idx(size);
    size_t size_class = idx2class(bin_idx);
    DPRINT("  Allocating %zu bytes on bin %d (size class = %zu)", size, bin_idx, size_class);

    void* ret_ptr = bin_alloc(heap, &heap->size_bins[bin_idx], size_class);
    inc_usage(heap, size_class);
    return ret_ptr;
}

void heap_free(void* ptr) {
    // Find the superblock the ptr resides in.
    Superblock* s_ptr = (Superblock*) ((uintptr_t) ptr & ~(SUPERBLOCK_SIZE - 1));
    size_t size_class = s_ptr->header.block_size;
    int bin_idx = size2idx(size_class);

    Heap* heap = s_ptr->header.owner;
    DPRINT("  Freeing from heap %p, bin %d (size class = %zu)", heap, bin_idx, size_class);

    bin_free(heap, &heap->size_bins[bin_idx], ptr);
    dec_usage(heap, size_class);
}