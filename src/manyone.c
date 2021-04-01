#define _GNU_SOURCE
#define DEV
#include <sched.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
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
#include <limits.h>
#include <ucontext.h>
#include "thread.h"
#include "tlibtypes.h"
#include "attributetypes.h"
#include "dataStructTypes.h"
#include "log.h"

tcb* __curproc = NULL;
tcb* __mainproc = NULL;
unsigned long int __nextpid;
//required for join - a thread in any state can be joined
tcbQueue __allThreads;
mut_t globallock;

static void* allocStack(size_t size, size_t guard){    
    void *stack = NULL;  
    //Align the memory to a 64 bit compatible page size and associate a guard area for the stack 
    if(posix_memalign(&stack,GUARD_SZ,size + guard) || mprotect(stack,guard, PROT_NONE)){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

void disabletimer(){
    signal(SIGALRM,SIG_IGN);
}

void starttimer(){
    // ualarm(2000000,2000000);
    struct itimerval it_val;
    it_val.it_interval.tv_sec = 2;
    it_val.it_interval.tv_usec = 0;
    it_val.it_value.tv_sec = 2;
    it_val.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
}

void scheduler(){
    disabletimer();
    // printAllmo(&__allThreads);
    // sleep(1);
    log_info("Call to scheduler (interrupted tid %d)", __curproc->tid);
    // Get the next thread in a RR fashion 
    spin_acquire(&globallock);
    queueRunning(&__allThreads);
    tcb *next = getNextThread(&__allThreads);
    spin_release(&globallock);
    if(!next) return;
    // Set new thread as running and current thread as runnable
    next->thread_state = RUNNING;
    tcb* __prev = __curproc;
    __curproc = next;
    if(__prev->thread_state != WAITING){
        __prev->thread_state = RUNNABLE;
    }
    log_trace("Prev TID was %d, Next Thread ID %ld", __prev->tid, next->tid);

    log_trace("Prev Context was %x, Next Context is %x", __prev->context, next->context);

    // log_info("Call to scheduler after get (interrupted tid %d)", __curproc->tid);
    // printAllmo(&__allThreads);
    // Copy the current thread TCB and add it to runnable list
    // tcb temp = *(__curproc);
    // if(temp.tid == getpid()){
    //     log_fatal("Main thread to be inserted");
    // }
    // Current process is now the new thread
    // __curproc = *next;
    
    // Swap contexts of the old and new threads
    // setcontext(&(next->context));}
    signal(SIGALRM, scheduler);
    swapcontext(__curproc->context,next->context);
}

void enabletimer(){
    signal(SIGALRM, scheduler);
}


void initManyOne(){
    log_info("Library initialized");
    spin_init(&globallock);
    ucontext_t* thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    __mainproc = (tcb*)malloc(sizeof(tcb));
    __mainproc->thread_state = RUNNING;
    __mainproc->tid = getpid();
    __mainproc->exited = 0;
    __mainproc->waiters = NULL;
    __mainproc->numWaiters = 0;
    getcontext(thread_context);
    __mainproc->context = thread_context;
    __nextpid = getpid()+1;
    addThread(&__allThreads,__mainproc);
    __curproc = __mainproc;
    starttimer();
}

void getQueue(){
    qnode *t = __allThreads.front;
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
    __curproc->exited = 1;
    for(int i = 0; i < __curproc->numWaiters; i++){
        printf("freeing waiters");
        tcb* temp = getThread(&__allThreads, __curproc->waiters[i]);
        temp->thread_state = RUNNABLE;
    }
    printf("Exited thread %d\n", __curproc->tid);
    // setcontext(__mainproc->context);
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
    ucontext_t te;
    getcontext(&te);
    log_error("Context of main %x", &te.uc_stack);
    ucontext_t *thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    // Allocate a new TCB and set all fields
    tcb *temp = (tcb *)malloc(sizeof(tcb));
    temp->thread_state = RUNNABLE;
    temp->tid = __nextpid++;
    //for handling join
    temp->exited = 0;
    temp->waiters = NULL;
    temp->numWaiters = 0;
    funcargs fa;
    fa.arg = arg;
    fa.f = routine;
    // (*(fa.f))(arg);
    // Context of thread is modified to accepts timer interrupts
    getcontext(thread_context);
    thread_context->uc_stack.ss_sp = allocStack(STACK_SZ,0);
    thread_context->uc_stack.ss_size = STACK_SZ;
    thread_context->uc_link = __mainproc->context;
    makecontext(thread_context,(void*)&wrapRoutine,1,(void *)&(fa));
    temp->context = thread_context;
    // Add the thread to list of runnable threads
    spin_acquire(&globallock);
    //required for join
    addThread(&__allThreads,temp);
    *t = temp->tid;
    spin_release(&globallock);
    log_trace("Thread created with tid %ld and curproc is %ld",temp->tid,__curproc->tid);
    //getQueue(__runnableList);
    enabletimer();
    // Start the timer for the main thread
    return 0;
}

int mo_thread_join(thread t, void **retLocation){
    disabletimer();
    spin_acquire(&globallock);
    tcb* waitedThread = getThread(&__allThreads, t);
    spin_release(&globallock);
    if(waitedThread == NULL){
        return ESRCH;
    }
    //check if not joinable, check if another thread is waiting (or not?)
    // if(waitedThread->exited){
    //     return EINVAL;
    // }
    if(waitedThread->exited){
        return 0;
    }
    //Add thread to the list of waiters
    waitedThread->waiters = (int*)realloc(waitedThread->waiters, (++(waitedThread->numWaiters))*sizeof(int));
    waitedThread->waiters[waitedThread->numWaiters-1] = __curproc->tid;
    __curproc->thread_state = WAITING;
    scheduler();
    return 0;
}