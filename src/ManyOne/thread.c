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
#include "locks.h"
#include "tlibtypes.h"
#include "attributetypes.h"
#include "dataStructTypes.h"
#include "log.h"
#include "thread.h"

tcb* __curproc = NULL;
tcb* __mainproc = NULL;
tcb* __scheduler = NULL;
unsigned long int __nextpid;
tcbQueue __allThreads;
int* exited = NULL;
int numExited = 0;
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

void starttimer(){
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

void enabletimer(){
    signal(SIGALRM, switchToScheduler);
}

void disabletimer(){
    signal(SIGALRM,SIG_IGN);
}

void switchToScheduler(){
    swapcontext(__curproc->context, __scheduler->context);
    enabletimer();
}

void scheduler(){
    disabletimer();
    int flag = 0;
    // log_info("Call to scheduler (interrupted tid %d)", __curproc->tid);
    for(int i = 0; i < numExited; i++){
        flag = 1;
        tcb* temp = getThread(&__allThreads, exited[i]);
        for(int j = 0; j < temp->numWaiters; j++){
            tcb* setRunnable = getThread(&__allThreads, temp->waiters[i]);
            // log_info("%d was waiting for %d, numwaiters:%d\n", temp->waiters[i], exited[i], temp->numWaiters);
            fflush(stdout);
            //issue here, the if is a hack
            if(setRunnable){
                setRunnable->thread_state = RUNNABLE;
            }
        }
        removeThread(&__allThreads, temp->tid);
    }
    if(flag){
        free(exited);
        numExited = 0;
        exited = NULL;
    }
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
    setcontext(next->context);
}


void initManyOne(){
    
    log_info("Library initialized");
    spin_init(&globallock);
    
    __mainproc = (tcb*)malloc(sizeof(tcb));
    __mainproc->thread_state = RUNNING;
    __mainproc->tid = getpid();
    __mainproc->exited = 0;
    __mainproc->waiters = NULL;
    __mainproc->numWaiters = 0;
    ucontext_t* thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(thread_context);
    __mainproc->context = thread_context;
    __nextpid = getpid()+1;
    addThread(&__allThreads,__mainproc);
    __curproc = __mainproc;
    
    __scheduler = (tcb*)malloc(sizeof(tcb));
    __scheduler->thread_state = RUNNING;
    __scheduler->tid = 0;
    __scheduler->exited = 0;
    __scheduler->waiters = NULL;
    __scheduler->numWaiters = 0;
    thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(thread_context);
    thread_context->uc_stack.ss_sp = allocStack(STACK_SZ,0);
    thread_context->uc_stack.ss_size = STACK_SZ;
    thread_context->uc_link = __mainproc->context;
    makecontext(thread_context, scheduler,0);
    __scheduler->context = thread_context;

    starttimer();
}

void wrapRoutine(void *fa){
    funcargs *temp = (funcargs *)fa;
    enabletimer();
    (temp->f)(temp->arg);
    disabletimer();
    exited = (int*)realloc(exited, (++numExited)*sizeof(int));
    exited[numExited-1] = __curproc->tid; 
    // __curproc = __mainproc;
    // __mainproc->thread_state = RUNNING;
    enabletimer();
    // while(1){}
    while(__mainproc->thread_state != RUNNABLE){}
    __mainproc->thread_state = RUNNING;
}

int thread_create(thread *t, void *attr, void *routine, void *arg){
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
    ucontext_t *thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    // Allocate a new TCB and set all fields
    tcb *temp = (tcb *)malloc(sizeof(tcb));
    temp->thread_state = RUNNABLE;
    temp->tid = __nextpid++;
    //for handling join
    temp->exited = 0;
    temp->waiters = NULL;
    temp->numWaiters = 0;
    funcargs* fa = (funcargs*)malloc(sizeof(funcargs));
    fa->f = routine;
    fa->arg = arg;
    // Context of thread is modified to accepts timer interrupts
    getcontext(thread_context);
    thread_context->uc_stack.ss_sp = allocStack(STACK_SZ,0);
    thread_context->uc_stack.ss_size = STACK_SZ;
    thread_context->uc_link = __mainproc->context;
    makecontext(thread_context,(void*)&wrapRoutine,1,(void *)(fa));
    temp->context = thread_context;
    // Add the thread to list of runnable threads
    spin_acquire(&globallock);
    addThread(&__allThreads,temp);
    *t = temp->tid;
    spin_release(&globallock);
    enabletimer();
    // Start the timer for the main thread
    return 0;
}

int thread_join(thread t, void **retLocation){
    // log_info("trying to join thread %d\n", t);
    fflush(stdout);
    disabletimer();
    spin_acquire(&globallock);
    tcb* waitedThread = getThread(&__allThreads, t);
    if(waitedThread == NULL){
        spin_release(&globallock);
        return ESRCH;
    }
    // check if not joinable, check if another thread is waiting (or not?)
    if(waitedThread->exited){
        spin_release(&globallock);
        return 0;
    }
    //Add thread to the list of waiters
    waitedThread->waiters = (int*)realloc(waitedThread->waiters, (++(waitedThread->numWaiters))*sizeof(int));
    waitedThread->waiters[waitedThread->numWaiters-1] = __curproc->tid;
    __curproc->thread_state = WAITING;
    spin_release(&globallock);
    switchToScheduler();
    return 0;
}