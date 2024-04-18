#include <stdlib.h>

int main(void) {
    void* ptr = malloc(400);
    free(ptr);
    ptr = malloc(442);
    free(ptr);
    return 0;
}
