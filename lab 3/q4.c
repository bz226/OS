#include <stdio.h>

#include "assignment.h"

#define TRUE  1
#define FALSE 0

int jobdone = FALSE;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* worker(void* arg) {
    printf("First\n");
    Mutex_lock(&mutex);
    jobdone = TRUE;
    Cond_signal(&cond);
    Mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    Pthread_create(&p, NULL, worker, NULL);
    while (jobdone == FALSE) {
        Mutex_lock(&mutex);
        while (jobdone == FALSE)
            Cond_wait(&cond, &mutex);
        Mutex_unlock(&mutex);
    }
    printf("Last\n");
    return 0;
}
