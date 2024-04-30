#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* threadFunction(void* arg) {
    printf("In spawned thread!\n");

    int* dynamicData = (int*)malloc(sizeof(int));
    if(dynamicData == NULL) {
        printf("Failed to allocate memory\n");
        return NULL;
    }

    *dynamicData = 42;
    printf("Dynamic data in the spawned thread: %d\n", *dynamicData);

    free(dynamicData);
    return NULL;
}

int main() {
    pthread_t threadId;
    int returnValue;

    // Create a new thread
    returnValue = pthread_create(&threadId, NULL, threadFunction, NULL);
    if (returnValue != 0) {
        printf("Failed to create a new thread. Error code: %d\n", returnValue);
        return 1;
    }

    printf("This is the main thread!\n");

    // Wait for the spawned thread to finish
    pthread_join(threadId, NULL);

    return 0;
}