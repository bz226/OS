#include "assignment.h"
#include <stdio.h>


#define LOOP 1000000

int balance = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void* worker(void* arg) {
    int i;
   
    for(i = 0; i < LOOP; i++){
        Mutex_lock(&mutex);
    	balance++; 
        Mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p, q;
    int i;

    Pthread_create(&p, NULL, worker, NULL);
    Pthread_create(&q, NULL, worker, NULL);
	
    for(i = 0; i < LOOP; i++){
        Mutex_lock(&mutex);
        balance++;
        Mutex_unlock(&mutex);
    }

    Pthread_join(p, NULL);
	Pthread_join(q, NULL);
    
    printf("balance = %d\n", balance);
    
    return 0;
}
