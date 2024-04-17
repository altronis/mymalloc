#ifndef MYMALLOC_SUPERBLOCK_H
#define MYMALLOC_SUPERBLOCK_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include "macros.h"

typedef struct heap Heap;

typedef struct free_block {
    struct free_block* next;
} FreeBlock;

typedef struct superblock_header {
    Heap* owner;  // Owner of this superblock.
    pthread_mutex_t mutex;

    size_t block_size;  // Size of each individual block
    unsigned int total_blocks; // Total number of blocks in the suprblock

    struct superblock* prev;  // Prev superblock in doubly linked list
    struct superblock* next;  // Next superblock

    unsigned int reapable_blocks;  // Number of blocks available to be reaped
    unsigned int num_free_blocks;  // Number of free blocks
    FreeBlock* free_list;  // LIFO Singly linked list of free blocks

    char* buffer_start;  // Start of buffer
    char* reap_position;  // Cursor into buffer for reap allocation

} __attribute__ ((aligned (16))) SuperblockHeader;

enum { buffer_size = SUPERBLOCK_SIZE - sizeof(SuperblockHeader) };

// Superblocks are aligned to SUPERBLOCK_SIZE boundary
typedef struct superblock {
    SuperblockHeader header;
    char buf[buffer_size];
} Superblock;

// Allocate and initialize a new superblock.
Superblock* init_superblock(size_t block_size);

void lock_superblock(Superblock* superblock);

void unlock_superblock(Superblock* superblock);

// Reuse an existing (and empty) superblock, changing its block size and clearing its free list.
void reset_superblock(Superblock* superblock, size_t block_size);

void* superblock_alloc(Superblock* superblock);

void superblock_free(Superblock* superblock, void* ptr);

bool is_superblock_empty(Superblock* superblock);

// Get number of used bytes
int used_bytes(Superblock* superblock);

#endif //MYMALLOC_SUPERBLOCK_H
