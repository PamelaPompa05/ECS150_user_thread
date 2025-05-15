#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include "uthread.h"
#include <unistd.h>
//#include "queue.h"

/*Test 1*/
void iterate_five_times(void *arg) {
    int id = *(int *)arg;

    if (id == 0) { //first thread creates others
        for (int i = 1; i < 5; i++) {
            int *thread_id = malloc(sizeof(int)); //allocate memory for each thread ID
            *thread_id = i;
            uthread_create(iterate_five_times, thread_id); //create new thread
        }
    }

    for (int i = 0; i < 5; i++) {
        printf("Thread %d running iteration %d\n", id, i);
        for (volatile int j = 0; j < 100000000; j++); // burn CPU time
    }

    free(arg); //Free allocated memory
}

int main() {
    fprintf(stderr, "*** TEST: Five-Thread-Preemption ***\n");

    int *thread_zero = malloc(sizeof(int));
    *thread_zero = 0;

    uthread_run(true, iterate_five_times, thread_zero); //first thread with argument 0

    return 0;
}