#include "mymalloc.h"

int main(void) {
    void* ptr = mymalloc(400);
    myfree(ptr);
    ptr = mymalloc(442);
    myfree(ptr);
    return 0;
}
