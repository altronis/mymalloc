#ifndef MYMALLOC_MACROS_H
#define MYMALLOC_MACROS_H

#include <stdio.h>

#ifdef MYMALLOC_DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#else
#define DPRINT
#endif

#define LG_MIN_BLOCK_SIZE 4  // 16
#define LG_SUPERBLOCK_SIZE 18  // 256K

#define MIN_BLOCK_SIZE (2 << LG_MIN_BLOCK_SIZE)
#define SUPERBLOCK_SIZE (2 << LG_SUPERBLOCK_SIZE)
#define NUM_SIZE_BINS (LG_SUPERBLOCK_SIZE - LG_MIN_BLOCK_SIZE)

#define LARGEALLOC_HEADER_MAGIC 0x8BADF00D

// Given ptr to buffer and header type, get ptr to header
#define GET_HEADER_PTR(ptr, header_type) ((header_type*) ((char*) ptr - sizeof(header_type)))

// Given ptr to header and header type, get ptr to buffer
#define GET_BUFFER_PTR(header_ptr, header_type) ((char*) header_ptr + sizeof(header_type))

#endif //MYMALLOC_MACROS_H