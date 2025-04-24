#include "assignment.h"
#include <stdio.h>


#define LOOP 1000000

int balance = 0;
pthread_mutex_t lock;

void* worker(void* arg) {
    int i;
   
    for(i = 0; i < LOOP; i++){
        pthread_mutex_lock(&lock);
    	balance++; 
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p, q;
    int i;
    pthread_mutex_init(&lock, NULL);
    Pthread_create(&p, NULL, worker, NULL);
    Pthread_create(&q, NULL, worker, NULL);
	
    for(i = 0; i < LOOP; i++){
        pthread_mutex_lock(&lock);
        balance++;
        pthread_mutex_unlock(&lock);
    }
    
    Pthread_join(p, NULL);
	Pthread_join(q, NULL);
    
    printf("balance = %d\n", balance);
    pthread_mutex_destroy(&lock);
    return 0;
}
