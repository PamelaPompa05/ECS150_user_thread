#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ucontext.h>    // for getcontext()

#include "private.h"     // uthread_ctx_t, uthread_ctx_* API
#include "uthread.h"     // uthread_func_t, uthread_run, etc.
#include "queue.h"       // queue_t

// libuthread/uthread.c
// Phase 2: User‐level thread API 

// Thread states 
typedef enum { RUNNING, READY, BLOCKED, EXITED } uthread_state_t;

// Thread Control Block 
struct uthread_tcb {
    uthread_ctx_t   *uctx;   
    void            *stack;  
    uthread_state_t  state;  
};

// ready/zombie queue of TCB pointers 
static queue_t               ready_q;
static queue_t				 zombie_q;

// currently running thread 
static struct uthread_tcb   *current;

// idle (main) context 
static uthread_ctx_t        *idle_uctx;


void cleanup_zombies(queue_t zombie_q)
{
    struct uthread_tcb *zombie;
    while (queue_dequeue(zombie_q, (void **)&zombie) == 0) { //while the zombie queue is not empty
        uthread_ctx_destroy_stack(zombie->stack);
        free(zombie->uctx);
        free(zombie);
    }
}

struct uthread_tcb * uthread_current(void)
{
    return current;
}


void uthread_yield(void)
{


    if (queue_length(ready_q) == 0){
        return;
	}

	preempt_disable();  // protect ready_q + current

    struct uthread_tcb *next;
    queue_dequeue(ready_q, (void **)&next);

    // re‐enqueue current if it is still running 
    if (current->state == RUNNING) {
        current->state = READY;
        queue_enqueue(ready_q, current);
    }

    // context switch 
    next->state = RUNNING;
    struct uthread_tcb *prev = current;
    current = next;

	preempt_enable();

    uthread_ctx_switch(prev->uctx, next->uctx);
}


void uthread_exit(void)
{

    struct uthread_tcb *prev = current;
    struct uthread_tcb *next;

	preempt_disable();
	prev->state = EXITED;

	//add the exited thread to our zombie queue
	queue_enqueue(zombie_q, prev); 

	preempt_enable();

    if (queue_length(ready_q) > 0) {
        // pick the next READY thread 
        queue_dequeue(ready_q, (void **)&next);
        next->state = RUNNING;
        current = next;

        // swap out of this thread into next 
        uthread_ctx_switch(prev->uctx, next->uctx);
        // never returns here 
    } else {
        // no more threads --> swap back to main 
        uthread_ctx_switch(prev->uctx, idle_uctx);
        // never returns here 
    }

    __builtin_unreachable();
}

int uthread_create(uthread_func_t func, void *arg)
{
    // allocate TCB 
    struct uthread_tcb *tcb = malloc(sizeof(*tcb));
    if (!tcb)
        return -1;

    // allocate a stack 
    tcb->stack = uthread_ctx_alloc_stack();
    if (!tcb->stack) {
        free(tcb);
        return -1;
    }

    // allocate and initialize context 
    tcb->uctx = malloc(sizeof(*tcb->uctx));
    if (!tcb->uctx ||
        uthread_ctx_init(tcb->uctx, tcb->stack, func, arg) < 0)
    {
        if (tcb->uctx) free(tcb->uctx);
        uthread_ctx_destroy_stack(tcb->stack);
        free(tcb);
        return -1;
    }

    tcb->state = READY;

	preempt_disable(); //protect ready_q (shared data)
	queue_enqueue(ready_q, tcb);
	preempt_enable();

    return 0;

}


int uthread_run(bool preempt, uthread_func_t func, void *arg)
{

	preempt_start(preempt); //initialize preemption if user wants it

    // initialize ready queue 
    ready_q = queue_create();
    if(!ready_q){
        return -1;
	}

	zombie_q = queue_create(); //this is where exited threads go to and are freed after all threads finish
	if(!zombie_q){
		return -1;
	}

    // capture main context as idle_uctx 
    idle_uctx = malloc(sizeof(*idle_uctx));
    if (!idle_uctx)
        return -1;
    if (getcontext(idle_uctx) < 0)
        return -1;

    // install TCB for idle  thread 
    current = malloc(sizeof(*current));
    if (!current)
        return -1;
    current->uctx  = idle_uctx;
    current->stack = NULL;
    current->state = RUNNING;

    //create initial user thread 
    if (uthread_create(func, arg) < 0){
        return -1;
	}

    // go until READY threads remain 
    while (queue_length(ready_q) > 0){
		cleanup_zombies(zombie_q);
        uthread_yield();
    }

	preempt_stop(); //stop preemption before exiting

    queue_destroy(ready_q);
	queue_destroy(zombie_q);
	free(idle_uctx);

    return 0;
}

void uthread_block(void)
{
	preempt_disable();
	current->state = BLOCKED; //mark thread as blocked
	preempt_enable();
    uthread_yield(); //switch execution to another READY thread
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	preempt_disable();
    if(uthread && uthread->state == BLOCKED){
        uthread->state = READY;
        queue_enqueue(ready_q, uthread); //re-add to scheduling queue
    }
	preempt_enable();
}

