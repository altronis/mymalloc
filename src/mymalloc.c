#include "mymalloc.h"

void* malloc(size_t size) {
    printf("foo\n");
    return NULL;
}

void free(void* ptr) {

}