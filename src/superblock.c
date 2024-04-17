#include <sys/mman.h>
#include <stdint.h>
#include "superblock.h"

static Superblock* allocate_new_superblock() {
    // Allocate 2S bytes
    char *ptr = (char *) mmap(NULL, 2 * SUPERBLOCK_SIZE, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANON, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }

    // Find alignment position
    char *aligned_ptr;
    uintptr_t alignment = (uintptr_t) ptr & (SUPERBLOCK_SIZE - 1);
    uintptr_t prologue_bytes;

    if (alignment == 0) {
        prologue_bytes = 0;
        aligned_ptr = ptr;
    } else {
        prologue_bytes = SUPERBLOCK_SIZE - alignment;
        aligned_ptr = ptr + prologue_bytes;
    }

    // Unmap prologue and epilogue
    if (prologue_bytes > 0)
        munmap(ptr, prologue_bytes);
    munmap(aligned_ptr + SUPERBLOCK_SIZE, SUPERBLOCK_SIZE - prologue_bytes);

    return (Superblock *) aligned_ptr;
}

Superblock* init_superblock(size_t block_size) {
    Superblock* superblock = allocate_new_superblock();
    if (superblock == NULL)
        return superblock;

    reset_superblock(superblock, block_size);
    DPRINT("      Allocated new superblock at %p with block size = %zu", superblock, block_size);
    return superblock;
}

void reset_superblock(Superblock* superblock, size_t block_size) {
    SuperblockHeader* header = &superblock->header;
    header->block_size = block_size;
    header->total_blocks = (SUPERBLOCK_SIZE - sizeof(SuperblockHeader)) / block_size;

    header->prev = header->next = NULL;
    header->reapable_blocks = header->num_free_blocks = header->total_blocks;
    header->free_list = NULL;
    header->reap_position = header->buffer_start = (char*) superblock + sizeof(SuperblockHeader);
}

void* superblock_alloc(Superblock* superblock) {
    SuperblockHeader* header = &(superblock->header);
    void* ptr;

    if (header->reapable_blocks > 0) {
        // Reap mode
        ptr = header->reap_position;
        header->reap_position += header->block_size;
        header->reapable_blocks--;
        DPRINT("      Reap mode: Allocating block at %p", ptr);
    } else {
        // Freelist mode
        ptr = header->free_list;
        header->free_list = header->free_list->next;
        DPRINT("      Freelist mode: Allocating block at %p", ptr);
    }

    header->num_free_blocks--;
    return ptr;
}

void superblock_free(Superblock* superblock, void* ptr) {
    if ((uintptr_t) ptr & (MIN_BLOCK_SIZE - 1))
        return;

    SuperblockHeader* header = &(superblock->header);

    // Add to free list
    FreeBlock* new_head = ptr;
    new_head->next = header->free_list;
    header->free_list = ptr;

    header->num_free_blocks++;
    DPRINT("      Free'd block at %p", ptr);
}

bool is_superblock_empty(Superblock* superblock) {
    SuperblockHeader* header = &(superblock->header);
    return header->num_free_blocks == header->total_blocks;
}