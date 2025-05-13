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

// ready queue of TCB pointers 
static queue_t               ready_q;

// currently running thread 
static struct uthread_tcb   *current;

// idle (main) context 
static uthread_ctx_t        *idle_uctx;


struct uthread_tcb * uthread_current(void)
{
    return current;
}


void uthread_yield(void)
{
    if (queue_length(ready_q) == 0)
        return;

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
    uthread_ctx_switch(prev->uctx, next->uctx);
}


void uthread_exit(void)
{
    struct uthread_tcb *prev = current;
    struct uthread_tcb *next;

	prev->state = EXITED;

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
    queue_enqueue(ready_q, tcb);
    return 0;
}


int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    (void)preempt;

    // initialize ready queue 
    ready_q = queue_create();
    if (!ready_q)
        return -1;

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

    // create initial user thread 
    if (uthread_create(func, arg) < 0)
        return -1;

    // go until READY threads remain 
    while (queue_length(ready_q) > 0) {
        uthread_yield();
    }

    queue_destroy(ready_q);
    return 0;
}


void uthread_block(void)
{
	current->state = BLOCKED; //mark thread as blocked
    uthread_yield(); //switch execution to another READY thread
}

void uthread_unblock(struct uthread_tcb *uthread)
{
    if(uthread && uthread->state == BLOCKED){
        uthread->state = READY;
        queue_enqueue(ready_q, uthread); // Re-add to scheduling queue
    }
}

