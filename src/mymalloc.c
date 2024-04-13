#include "mymalloc.h"
#include "largealloc.h"
#include "constants.h"

void* mymalloc(size_t size) {
    if (size > SUPERBLOCK_SIZE / 2)
        return large_alloc(size);

    return NULL;
}

void myfree(void* ptr) {

}