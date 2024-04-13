#ifndef MYMALLOC_CONSTANTS_H
#define MYMALLOC_CONSTANTS_H

#include <stdio.h>

#ifdef MYMALLOC_DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINT
#endif

#define LG_MIN_BLOCK_SIZE 4  // 16
#define LG_SUPERBLOCK_SIZE 18  // 256K

#define MIN_BLOCK_SIZE (2 << LG_MIN_BLOCK_SIZE)
#define SUPERBLOCK_SIZE (2 << LG_SUPERBLOCK_SIZE)
#define NUM_SIZE_BINS (LG_SUPERBLOCK_SIZE - LG_MIN_BLOCK_SIZE)

#endif //MYMALLOC_CONSTANTS_H
