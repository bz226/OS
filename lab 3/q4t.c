#include <stdio.h>

#include "assignment.h"

#define TRUE  1
#define FALSE 0

int jobdone = FALSE;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void* worker(void* arg) {
    printf("First\n");
    pthread_mutex_lock(&lock);
    jobdone = TRUE;
    pthread_cond_signal(&c);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void thr_join(){
    pthread_mutex_lock(&lock);
    while (jobdone == FALSE){
        pthread_cond_wait(&c, &lock);
    }
    pthread_mutex_unlock(&lock);
}
int main(int argc, char *argv[]) {
    pthread_t p;
    Pthread_create(&p, NULL, worker, NULL);
    thr_join();
    printf("Last\n");
    return 0;
}
