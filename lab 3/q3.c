#include <stdio.h>

#include "assignment.h"

#define TRUE  1
#define FALSE 0

int jobdone = FALSE;

void* worker(void* arg) {
    printf("First\n");
    jobdone = TRUE;
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    Pthread_create(&p, NULL, worker, NULL);
    while (jobdone == FALSE)
	;
    printf("Last\n");
    return 0;
}
