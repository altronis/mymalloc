#include "mymalloc.h"

int main(void) {
    void* ptr = mymalloc(500000);
    myfree(ptr);
    return 0;
}
