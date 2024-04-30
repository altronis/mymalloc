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

static void lock_heap(Heap* heap) {
    DPRINT("  Locking heap %p...", heap);
    pthread_mutex_lock(&heap->mutex);
}

static void unlock_heap(Heap* heap) {
    DPRINT("  Unlocking heap %p...", heap);
    pthread_mutex_unlock(&heap->mutex);
}

static Superblock* get_sb_from_global(size_t size_class) {
    lock_heap(&global_heap);
    Superblock* s_ptr = NULL;
    int bin_idx = size2idx(size_class);

    BinManager* bin_manager = &global_heap.size_bins[bin_idx];
    Superblock* recycling_bin = global_heap.recycled_superblock;

    if (bin_manager->num_nonfull_superblocks > 0) {
        int eidx;

        // Find a superblock with free space
        for (eidx = 1; eidx < NUM_EMPTINESS_CLASSES; eidx++) {
            s_ptr = bin_manager->emptiness_bins[eidx];
            if (s_ptr != NULL) {
                DPRINT("    Transferring superblock %p from global heap", s_ptr);
                break;
            }
        }
        ASSERT(s_ptr != NULL);

        // Remove it from the global heap
        DPRINT("    Removing superblock %p from global heap (bin %d, eidx %d)", s_ptr, bin_idx, eidx);
        delete_from_bin(bin_manager, eidx, s_ptr);

    } else if (recycling_bin != NULL) { // Check recycling bin too
        s_ptr = recycling_bin;
        ASSERT(recycling_bin->header.next != NULL);

        global_heap.recycled_superblock = recycling_bin->header.next;
        DPRINT("    Removing superblock %p from global heap (recycling bin)", s_ptr);
    }

    unlock_heap(&global_heap);
    return s_ptr;
}

static Superblock* find_emptiest_sb(Heap* heap, int* bin_idx, int* eidx) {
    Superblock* s_ptr = NULL;

    for (int e = NUM_EMPTINESS_CLASSES - 1; e > 0; e--) {
        for (int b = 0; b < num_size_bins; b++) {
            BinManager* bin_manager = &heap->size_bins[b];
            s_ptr = bin_manager->emptiness_bins[e];

            if (s_ptr != NULL) {
                *bin_idx = b;
                *eidx = e;
                return s_ptr;
            }
        }
    }

    ASSERT(false);
    return NULL;
}

static void transfer_sb_to_global(Heap* heap) {
    Superblock* s_ptr = NULL;
    Superblock* recycling_bin = heap->recycled_superblock;

    int bin_idx = -1;
    int eidx = -1;

    // Check recycling bin first.
    if (recycling_bin != NULL) {
        s_ptr = recycling_bin;
        DPRINT("    Transferring superblock %p from heap %p (recycling bin) to globl heap...", s_ptr, heap);
        heap->recycled_superblock = recycling_bin->header.next;
    }
    else {
        s_ptr = find_emptiest_sb(heap, &bin_idx, &eidx);
        DPRINT("    Transferring superblock %p from heap %p (bin_idx=%d, eidx=%d) to globl heap..."
            , s_ptr, heap, eidx, bin_idx);
        delete_from_bin(&heap->size_bins[bin_idx], eidx, s_ptr);
    }

    ASSERT(s_ptr != NULL);
    lock_heap(&global_heap);
    s_ptr->header.owner = &global_heap;

    // Insert into global heap.
    if (eidx == -1) {
        // Insert into recycling bin.
        s_ptr->header.next = global_heap.recycled_superblock;
        global_heap.recycled_superblock = s_ptr;
    } else {
        push_into_bin(&global_heap.size_bins[bin_idx], eidx, s_ptr);
    }

    unlock_heap(&global_heap);
}

// Actual allocation/free functions.
static void* bin_alloc(Heap* heap, BinManager* bin_manager, size_t size_class) {
    Superblock* s_ptr = NULL;  // Ptr to the superblock we are allocating from
    bool sb_already_in_bin = false;  // True if the superblock was in the bin before this call

    // Start looking from bin 1 (fullest to emptiest)
    for (int eidx = 1; eidx < NUM_EMPTINESS_CLASSES; eidx++) {
        s_ptr = bin_manager->emptiness_bins[eidx];
        if (s_ptr != NULL) {
            DPRINT("    Allocating from emptiness class %d at superblock %p", eidx, s_ptr);
            sb_already_in_bin = true;
            break;
        }
    }

    // Recycle an empty superblock, if there is any.
    if (s_ptr == NULL) {
        Superblock *recycled_superblock = heap->recycled_superblock;
        if (recycled_superblock != NULL) {
            s_ptr = recycled_superblock;
            reset_superblock(s_ptr, size_class);

            heap->recycled_superblock = recycled_superblock->header.next;
            DPRINT("    Allocating from recycled superblock %p", s_ptr);
        }
    }

    // None found in this bin. Check global heap.
    if (s_ptr == NULL) {
        s_ptr = get_sb_from_global(size_class);

        // Update stats of this heap
        if (s_ptr != NULL) {
            s_ptr->header.owner = heap;
            inc_usage(heap, used_bytes(s_ptr));
            inc_alloced(heap, SUPERBLOCK_SIZE);
        }
    }

    // None in global heap either. Allocate a new one.
    if (s_ptr == NULL) {
        s_ptr = init_superblock(size_class);
        if (s_ptr == NULL)
            return NULL;

        // Add it to the emptiest class.
        s_ptr->header.owner = heap;
        inc_alloced(heap, SUPERBLOCK_SIZE);
    }

    // Allocate from the superblock.
    unsigned int old_eidx = get_eidx(s_ptr);
    void* ret_ptr = superblock_alloc(s_ptr);
    ASSERT(ret_ptr != NULL);

    if (sb_already_in_bin)
        update_emptiness_class(bin_manager, old_eidx, s_ptr);
    else {
        unsigned int new_eidx = get_eidx(s_ptr);
        push_into_bin(bin_manager, new_eidx, s_ptr);
        DPRINT("    Assigned superblock %p to emptiness class %u", s_ptr, new_eidx);
    }

    return ret_ptr;
}

static void bin_free(Heap* heap, BinManager* bin_manager, void* ptr) {
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

    lock_heap(heap);
    void* ret_ptr = bin_alloc(heap, &heap->size_bins[bin_idx], size_class);
    inc_usage(heap, size_class);

    unlock_heap(heap);
    return ret_ptr;
}

size_t get_block_size(void* ptr) {
    Superblock* s_ptr = (Superblock*) ((uintptr_t) ptr & ~(SUPERBLOCK_SIZE - 1));
    return s_ptr->header.block_size;
}

void heap_free(void* ptr) {
    // Find the superblock the ptr resides in.
    Superblock* s_ptr = (Superblock*) ((uintptr_t) ptr & ~(SUPERBLOCK_SIZE - 1));
    lock_superblock(s_ptr);

    size_t size_class = s_ptr->header.block_size;
    int bin_idx = size2idx(size_class);

    Heap* heap = s_ptr->header.owner;
    DPRINT("  Freeing from superblock %p, heap %p, bin %d (size class = %zu)",
           s_ptr, heap, bin_idx, size_class);

    lock_heap(heap);
    bin_free(heap, &heap->size_bins[bin_idx], ptr);

    if (heap != &global_heap) {
        dec_usage(heap, size_class);

        if (is_empty_enough(heap)) {
            transfer_sb_to_global(heap);
            dec_alloced(heap, SUPERBLOCK_SIZE);
        }
    }

    unlock_heap(heap);
    unlock_superblock(s_ptr);
}

bool is_empty_enough(Heap* heap) {
    size_t u = heap->in_use;
    size_t a = heap->alloced;

    return u + FREE_SB_THRESH * SUPERBLOCK_SIZE < a &&
        u < (size_t) ((1 - EMPTY_FRAC) * (double) a);
}