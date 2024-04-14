#ifndef MYMALLOC_BINMANAGER_H
#define MYMALLOC_BINMANAGER_H

#include "superblock.h"

typedef struct bin_manager {
    // Linked lists sorted by emptiness class
    Superblock* emptiness_bins[NUM_EMPTINESS_CLASSES];
} BinManager;

// Initialize the size table. Called when the heap is initialized.
void init_size_table();

size_t idx2class(int idx);

int size2idx(size_t size);

extern size_t max_block_size;

#endif //MYMALLOC_BINMANAGER_H
