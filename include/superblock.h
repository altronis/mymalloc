#ifndef MYMALLOC_SUPERBLOCK_H
#define MYMALLOC_SUPERBLOCK_H

#include "stddef.h"
#include "macros.h"

typedef struct superblock_header {
    size_t block_size;  // Size of each individual block
    unsigned int total_blocks; // Total number of blocks in the suprblock

    struct superblock* prev;  // Prev superblock in doubly linked list
    struct superblock* next;  // Next superblock

    unsigned int reapable_blocks;  // Number of blocks available to be reaped
    unsigned int free_blocks;  // Number of free blocks

    char* buffer_start;  // Start of buffer
    char* reap_position;  // Cursor into buffer for reap allocation

} __attribute__ ((aligned (16))) SuperblockHeader;

enum { buffer_size = SUPERBLOCK_SIZE - sizeof(SuperblockHeader) };

// Superblocks are aligned to SUPERBLOCK_SIZE boundary
typedef struct superblock {
    SuperblockHeader header;
    char buf[buffer_size];
} Superblock;

#endif //MYMALLOC_SUPERBLOCK_H
