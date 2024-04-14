#include "macros.h"
#include "binmanager.h"

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
        new_sz -= new_sz % MIN_BLOCK_SIZE;
        while (new_sz <= sz)
            new_sz += MIN_BLOCK_SIZE;

        if (new_sz > half_sb)
            break;

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
    return j;
}