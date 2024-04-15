#ifndef MYMALLOC_BINMANAGER_H
#define MYMALLOC_BINMANAGER_H

#include "superblock.h"

typedef struct bin_manager {
    // Linked lists sorted by emptiness class (fullest to emptiest)
    Superblock* emptiness_bins[NUM_EMPTINESS_CLASSES];
} BinManager;

// Initialize the size table. Called when the heap is initialized.
void init_size_table();

size_t idx2class(int idx);

int size2idx(size_t size);

extern size_t max_block_size;

// Add to the head of the linked list (superblock becomes new head)
void push_into_bin(BinManager* bin_manager, unsigned int eidx, Superblock* superblock);

// Delete superblock from the linked list. Assumes that the superblock is in this list.
void delete_from_bin(BinManager* bin_manager, unsigned int eidx, Superblock* superblock);

// Get the emptiness class of a superblock.
unsigned int get_eidx(Superblock* superblock);

// Update the emptiness class of a superblock, pushing it into the appropriate list.
void update_emptiness_class(BinManager* bin_manager, unsigned int old_eidx, Superblock* superblock);

#endif //MYMALLOC_BINMANAGER_H
