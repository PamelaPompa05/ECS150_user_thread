#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */


/*
From <signal.h>

struct sigaction{
	void     (*sa_handler)(int);
	void     (*sa_sigaction)(int, siginfo_t *, void *);
	sigset_t   sa_mask;
	int        sa_flags;
	void     (*sa_restorer)(void);
}

From <sys/time.h>

struct itimerval {
    struct timeval it_interval;  //time between successive signal
    struct timeval it_value;     //time until the next signal
};

struct timeval {
    time_t      tv_sec;          //seconds
    suseconds_t tv_usec;         //microseconds
};
*/

#define HZ 100

void handler(int signum){
	/*signum = which signal triggered the handler*/
	(void) signum; //we don't need signum, this line prevents a warning error of an unused variable

	uthread_yield(); //force a context switch
}


void preempt_enable(void)
{
	/*This function unblocks the SIGVTALRM, allowing it to be delivered*/

	sigset_t mask;
    sigemptyset(&mask);			 //initialize no blocked signals
    sigaddset(&mask, SIGVTALRM); //add SIGVTALRM to the set
    sigprocmask(SIG_UNBLOCK, &mask, NULL); //unblock SIGVTALRM

	return;
}

void preempt_disable(void)
{

	/*This function blocks the SIGVTALRM from being delivered*/

	sigset_t mask;
    sigemptyset(&mask);			 //initialize no blocked signals
    sigaddset(&mask, SIGVTALRM); //add SIGVTALRM to the set
    sigprocmask(SIG_BLOCK, &mask, NULL); //block SIGVTALRM

	return;
}

void preempt_start(bool preempt)
{
	/*This function sets up preemption at the initialization of the uthread library 
	
		-The function handler is triggered whenever a signal SIGVTALRM is received
		-The function handler forces the thread to yield
		-The timer is the one that sends out the SIGVTALRM every 100HZ
	
	*/
	
	if(!preempt){ return; }

	struct sigaction sa;   //the struct is declared in <signal.h>
	sa.sa_handler = handler;  //this is our signal handler function
	sigemptyset(&sa.sa_mask); //dont block any signals when SIGVTALRM is handled
	sa.sa_flags = SA_NODEFER | SA_RESTART; //don't block the signal while the handler is running, and dont block system calls with the signal

	if(sigaction(SIGVTALRM, &sa, NULL) == -1){ //Our sa struct's handler function receive SIGVTALRM alarm signals
		exit(1);
	}

	struct itimerval timer;

	/*When timer starts*/
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100*HZ; //start immediately
	/*Repeat timer*/
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 100*HZ; // 100 HZ between signals

	if(setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1){ //ITIMER_VIRTUAL counts down only when the process is running in user mode. 
		exit(1);								       //and it Sends SIGVTALRM when it expires.
	}

	return;
}

void preempt_stop(void)
{
	/*
		This function disables the timer so that it stops sending the SIGVTALRM signals
		It also resets the signal handler to default.
	*/

	struct itimerval timer;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);

	struct sigaction sa;
	sa.sa_handler = SIG_DFL; //restore the default signal behavior
    sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, NULL);

	return;
}

/*Sources
https://stackoverflow.com/questions/21180857/installing-signal-handler-in-c
https://thelinuxcode.com/c-sigaction-function-usage/
https://stackoverflow.com/questions/231912/what-is-the-difference-between-sigaction-and-signal/
https://www.man7.org/linux/man-pages/man2/sigaction.2.html
https://linux.die.net/man/2/setitimer
https://www.man7.org/linux/man-pages/man2/sigprocmask.2.html
https://www.gnu.org/software/libc/manual/html_node/Blocking-Signals.html
*/

