#include "mymalloc.h"

int main(void) {
    void* ptr = mymalloc(400);
    myfree(ptr);
    return 0;
}
