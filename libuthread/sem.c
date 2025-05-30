#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h" //for uthread_current()
#include "sem.h"


struct semaphore {
	int count;
	struct queue *blocked_queue;
};

sem_t sem_create(size_t count)
{
	sem_t Semaphore = (sem_t) malloc(sizeof(struct semaphore)); 
	if(Semaphore == NULL){ //memory allocation error
		return NULL;
	}

	Semaphore->count = count;
	Semaphore->blocked_queue = queue_create();

	return Semaphore;
}

int sem_destroy(sem_t sem)
{

	if(sem == NULL || queue_length(sem->blocked_queue) > 0){
		return -1;
	}

	preempt_disable();

	if(queue_destroy(sem->blocked_queue) == 0){ //if the queue is destroyed properly
		free(sem);
		preempt_enable();
		return 0;
	}

	preempt_enable();
	
	return -1; //error
}

int sem_down(sem_t sem)
{

	if(sem == NULL){
		return -1;
	}

	preempt_disable();

	/*fix for the corner case*/
	while (sem->count == 0){  //keep checking until a resource is available (the blocked thread resumes control here when switched back)
        struct uthread_tcb *current_thread = uthread_current(); // find the current thread
        queue_enqueue(sem->blocked_queue, current_thread);      // add the current thread to our blocked thread
        uthread_block();										// block the current thread (from the private API) & yields
    }

	sem->count--;  //finally take the resource once it's available

	preempt_enable();

    return 0;
}

int sem_up(sem_t sem)
{

	if(sem == NULL){
		return -1;
	}

	preempt_disable();
	sem->count++;

	if(queue_length(sem->blocked_queue) > 0){ //if there is a thread in our blocked queue
		struct uthread_tcb *thread_to_be_unblocked;
		queue_dequeue(sem->blocked_queue, (void**)&thread_to_be_unblocked); //dequeue the oldest thread

		uthread_unblock(thread_to_be_unblocked);
	}

	preempt_enable();

	return 0;
}

