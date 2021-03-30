#define _GNU_SOURCE
#define DEV
#include <sched.h>
#include <unistd.h>
#include<sys/stat.h>
#include<stdio.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "thread.h"
#include "tlibtypes.h"
#include "attributetypes.h"
#include "dataStructTypes.h"
#include<limits.h>
#include "log.h"
#include <ucontext.h>

tcb __curproc;
unsigned long int __nextpid;
tcbQueue __runnableList;

void disabletimer(){
    signal(SIGALRM,SIG_IGN);
}

void scheduler(){
    disabletimer();
    log_info("Call to scheduler");
    // Get the next thread in a RR fashion 
    tcb *next = getNextThread(&__runnableList);
    if(!next) return;
    log_trace("Next Thread ID %ld",next->tid);
    // Set new thread as running and current thread as runnable
    next->thread_state = RUNNING;
    __curproc.thread_state = RUNNABLE;
    // Copy the current thread TCB and add it to runnable list
    tcb temp = __curproc;
    if(temp.tid == getpid()){
        log_fatal("Main thread to be inserted");
    }
    addThread(&__runnableList,&temp);
    // Current process is now the new thread
    __curproc = *next;
    // Swap contexts of the old and new threads

    // swapcontext(&(temp.context),&(next->context));
}


void enabletimer(){
    signal(SIGALRM, scheduler);
}

void starttimer(){
    ualarm(10000,0);
}

void initManyOne(){
    signal(SIGALRM,scheduler);
    __nextpid = getpid()+1;
    getcontext(&__curproc.context);
    __curproc.tid = getpid();
    __curproc.thread_state = RUNNING;
}

void getQueue(){
    qnode *t = __runnableList.front;
    while(t){
        log_trace("%d",t->tcbnode->tid);
        t = t->next;
    }
    return;
}
void wrapRoutine(void *fa){
    signal(SIGALRM,scheduler);
    funcargs *temp = (funcargs *)fa;
    starttimer();
    sleep(2);
    temp->f(temp->arg);
    //cleanup();
}
int createManyOne(thread *t, void *attr, void *routine, void *arg){
    // Disable any switches during creation
    disabletimer();
    // Initalise the state of the main thread
    static int isInit = 0;
    if(!isInit){
        initManyOne();
        isInit = 1;
    }
    ucontext_t thread_context;
    // Allocate a new TCB and set all fields
    tcb *temp = (tcb *)malloc(sizeof(tcb));
    temp->thread_state = RUNNABLE;
    temp->tid = __nextpid++;
    funcargs fa;
    fa.arg = arg;
    fa.f = routine;
    // Context of thread is modified to accepts timer interrupts
    // makecontext(&thread_context,wrapRoutine,1,(void *)&fa);
    temp->context = thread_context;
    // Add the thread to list of runnable threads
    addThread(&__runnableList,temp);
    *t = temp->tid;
    log_trace("Thread created with tid %ld",temp->tid);
    enabletimer();
    // Start the timer for the main thread
    starttimer();
    sleep(1);
    return 0;
}