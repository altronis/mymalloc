#include "mymalloc.h"
#include "debug.h"

void* mymalloc(size_t size) {
    DPRINT("foo\n");
    return NULL;
}

void myfree(void* ptr) {

}