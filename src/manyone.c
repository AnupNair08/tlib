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
mut_t globallock;

void disabletimer(){
    signal(SIGALRM,SIG_IGN);
}

void scheduler(){
    disabletimer();
    // sleep(1);
    log_info("Call to scheduler");
    // Get the next thread in a RR fashion 
    spin_acquire(&globallock);
    tcb *next = getNextThread(&__runnableList);
    spin_release(&globallock);
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
    else{
        spin_acquire(&globallock);
        addThread(&__runnableList,&temp);
        spin_release(&globallock);
    }
    // Current process is now the new thread
    // __curproc = *next;
    
    // Swap contexts of the old and new threads
    // setcontext(&(next->context));
    swapcontext(&(__curproc.context),&(next->context));
}


void enabletimer(){
    signal(SIGALRM, scheduler);
}

void starttimer(){
    ualarm(2000,0);
}

void initManyOne(){
    log_info("Library initialized");
    spin_init(&globallock);
    signal(SIGALRM,scheduler);
    pid_t p = getpid();
    __nextpid = p+1;
    getcontext(&__curproc.context);
    __curproc.tid = getpid();
    __curproc.thread_state = RUNNING;
}

static void* allocStack(size_t size, size_t guard){    
    void *stack = NULL;  
    //Align the memory to a 64 bit compatible page size and associate a guard area for the stack 
    if(posix_memalign(&stack,GUARD_SZ,size + guard) || mprotect(stack,guard, PROT_NONE)){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
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
    // sleep(4);
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
    getcontext(&thread_context);
    thread_context.uc_stack.ss_sp = allocStack(STACK_SZ,0);
    thread_context.uc_stack.ss_size = STACK_SZ;
    thread_context.uc_link = &(__curproc.context);
    makecontext(&thread_context,wrapRoutine,1,(void *)&(fa));
    temp->context = thread_context;
    // Add the thread to list of runnable threads
    spin_acquire(&globallock);
    addThread(&__runnableList,temp);
    *t = temp->tid;
    spin_release(&globallock);
    log_trace("Thread created with tid %ld and curproc is %ld",temp->tid,__curproc.tid);
    //getQueue(__runnableList);
    enabletimer();
    // Start the timer for the main thread
    starttimer();
    sleep(10);
    return 0;
}
