#include <stdint.h>
#include "macros.h"
#include "binmanager.h"

#define GET_PREV(superblock) ((superblock)->header.prev)
#define GET_NEXT(superblock) ((superblock)->header.next)

#define SET_PREV(superblock, target) (&(superblock)->header)->prev = (target)
#define SET_NEXT(superblock, target) (&(superblock)->header)->next = (target)

static size_t size_table[MAX_NUM_BINS];  // Stores block size for each bin
size_t max_block_size;  // Largest block size before large_alloc handles it
static int num_size_bins;

void init_size_table() {
    size_t half_sb = (SUPERBLOCK_SIZE - sizeof(SuperblockHeader)) / 2;
    size_t sz = MIN_BLOCK_SIZE;
    int idx = 0;

    while (1) {
        size_table[idx] = sz;
        size_t new_sz = (size_t) ((double) sz * SIZE_RATIO);

        // Align
        new_sz = new_sz & ~(MIN_BLOCK_SIZE - 1);
        while (new_sz <= sz)
            new_sz += MIN_BLOCK_SIZE;

        if (new_sz > half_sb)
            break;

        ASSERT((new_sz & (MIN_BLOCK_SIZE - 1)) == 0);  // Check proper alignment
        sz = new_sz;
        idx++;
    }

    // If maximum block size is not large enough, add one more
    size_t max_block_size_threshold = (size_t) ((double) half_sb * MAX_BLOCK_SIZE_THRESHOLD_RATIO);
    if (sz < max_block_size_threshold) {
        sz = half_sb;
        idx++;
        size_table[idx] = sz;
    }

    num_size_bins = idx + 1;
    max_block_size = sz;

    ASSERT(2 * max_block_size + sizeof(SuperblockHeader) <= SUPERBLOCK_SIZE);  // Check if 2 max_blocks can fit
    DPRINT("Size table initialized with max_block_size = %zu and with %d bins", max_block_size, num_size_bins);
}

size_t idx2class(int idx) {
    return size_table[idx];
}

int size2idx(size_t size) {
    int i = 0, j = num_size_bins;

    while (i < j) {
        int s = i + (j - i) / 2;
        if (size > size_table[s]) {
            i = s + 1;
        } else {
            j = s;
        }
    }

    ASSERT(j >= 0 && j < num_size_bins);
    return j;
}

void push_into_bin(BinManager* bin_manager, unsigned int eidx, Superblock* superblock) {
    Superblock* list_head = bin_manager->emptiness_bins[eidx];
    Superblock** list_head_ptr = &(bin_manager->emptiness_bins[eidx]);

    if (list_head == NULL) {
        *list_head_ptr = superblock;
        SET_PREV(superblock, superblock);
        SET_NEXT(superblock, superblock);
    } else {
        Superblock* list_tail = GET_PREV(list_head);
        ASSERT(list_tail != NULL);

        SET_NEXT(list_tail, superblock);
        SET_PREV(superblock, list_tail);

        SET_NEXT(superblock, list_head);
        SET_PREV(list_head, superblock);
        *list_head_ptr = superblock;
    }

    if (eidx > 0)
        bin_manager->num_nonfull_superblocks++;
}

void delete_from_bin(BinManager* bin_manager, unsigned int eidx, Superblock* superblock){
    Superblock* list_head = bin_manager->emptiness_bins[eidx];
    Superblock** list_head_ptr = &(bin_manager->emptiness_bins[eidx]);
    ASSERT(list_head != NULL);

    // 1-element list
    if (GET_PREV(superblock) == superblock) {
        ASSERT(GET_NEXT(superblock) == superblock);
        *list_head_ptr = NULL;
        return;
    }
    // General case
    // If deleting head, make the next element the new head
    Superblock* prev_sb = GET_PREV(superblock);
    Superblock* next_sb = GET_NEXT(superblock);
    ASSERT(prev_sb != NULL);
    ASSERT(next_sb != NULL);

    SET_NEXT(prev_sb, next_sb);
    SET_PREV(next_sb, prev_sb);

    if (list_head == superblock)
        *list_head_ptr = next_sb;

    if (eidx > 0) {
        ASSERT(bin_manager->num_nonfull_superblocks > 0);
        bin_manager->num_nonfull_superblocks--;
    }
}

unsigned int get_eidx(Superblock* superblock) {
    SuperblockHeader* header = &(superblock->header);
    unsigned int num_full_blocks = header->total_blocks - header->num_free_blocks;

    unsigned int fullness = (num_full_blocks * (NUM_EMPTINESS_CLASSES - 1)) /
                            header->total_blocks;
    unsigned int eidx = NUM_EMPTINESS_CLASSES - 1 - fullness;

    ASSERT(eidx >= 0 && eidx < NUM_EMPTINESS_CLASSES);
    return eidx;
}

void update_emptiness_class(BinManager* bin_manager, unsigned int old_eidx, Superblock* superblock) {
    unsigned int new_eidx = get_eidx(superblock);
    delete_from_bin(bin_manager, old_eidx, superblock);
    push_into_bin(bin_manager, new_eidx, superblock);
    DPRINT("  Re-assigned superblock %p to emptiness class %u", superblock, new_eidx);
}
