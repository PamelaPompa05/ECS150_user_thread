#include <stdlib.h>
#include <ucontext.h>

#include "scheduler.h"

#define STACK_SIZE 4096
#define TASK_COUNT_MAX 8
#define TASK_COUNT_MIN 1

static ucontext_t main_ctx; //context for main
static ucontext_t ctx[TASK_COUNT_MAX]; //contexts for the tasks

int scheduler_run(void (*task_func)(int), int task_cnt, const int *task_sched)
{
	if(task_cnt < TASK_COUNT_MIN || task_cnt > TASK_COUNT_MAX ){ return -1; } //invalid number of tasks
	if(task_func == NULL){ return -1; }//Invalid function pointer

	char *stacks[task_cnt];

	/*Initialize the contexts and stack for every task*/
	for(int i = 0; i < task_cnt; i++){
		stacks[i] = malloc(STACK_SIZE);
		if(!stacks[i]){ return -1; } //memory allocation issue
		
		getcontext(&ctx[i]); //initialize context
		ctx[i].uc_stack.ss_sp = stacks[i];
		ctx[i].uc_stack.ss_size = STACK_SIZE;
		makecontext(&ctx[i], (void (*)(void))task_func, 1, i);
	}

	/*Run the scheduler/Execute Tasks*/
	for(int i = 0; task_sched[i] != -1; i++){
		int task_id = task_sched[i];
		if(task_id < 0 || task_id >= task_cnt){ return -1; } //invalid ID (not in range)

		swapcontext(&main_ctx, &ctx[task_id]);
	}

	for(int i = 0; i < task_cnt; i++){ free(stacks[i]); }

	return 0;
}

void scheduler_yield(int task_id)
{
	swapcontext(&ctx[task_id], &main_ctx); //swap from current context to main context
}

/*Sources*/
//https://stackoverflow.com/questions/6145091/the-term-context-in-programming --> used this to learn more about what a context is
//https://people.computing.clemson.edu/~yfeaste/modules1/mallocAndSizeof.pdf --> I forgot how to malloc space and wanted to check
//https://pdos.csail.mit.edu/6.828/2007/lec/l-threads.html --> I wanted to understand why every context/thread needs its own stack (for local variables)
//https://stackoverflow.com/questions/34106827/threads-in-c-cant-pass-function-properly-to-makecontext --> needed examples on how to use makecontext()
