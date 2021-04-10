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
#include "attributetypes.h"
#include "log.h"
#include "tlib.h"
#include "sighandler.h"
#include <setjmp.h>
tcb* __curproc = NULL;
tcb* __scheduler = NULL;
tcb* __mainproc = NULL;
tcbQueue __allThreads;
spin_t globallock;
sigset_t __signalList;
unsigned long int __nextpid;
int* exited = NULL;
int numExited = 0;


typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%fs:0x30,%0\n"
			"rol    $0x11,%0\n"
			: "=g" (ret)
			: "0" (addr));
	return ret;
}


/**
 * @brief Function to allocate a stack to Many One threads
 * 
 * @param size Size of stack excluding the guard size
 * @param guard Size of guard page
 * @return void
 */
static void* allocStack(size_t size, size_t guard){    
    void *stack = NULL;  
    //Align the memory to a 64 bit compatible page size and associate a guard area for the stack 
    if(posix_memalign(&stack,GUARD_SZ,size + guard) || mprotect(stack,guard, PROT_NONE)){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

/**
 * @brief Function to create signal sets for threads
 * 
 * @return void
 */
static void setSignals(){
    sigfillset(&__signalList);
    sigdelset(&__signalList,SIGINT);
    sigdelset(&__signalList,SIGSTOP);
    sigdelset(&__signalList,SIGCONT);
    sigdelset(&__signalList,SIGVTALRM);
    sigprocmask(SIG_BLOCK,&__signalList,NULL);
}

/**
 * @brief Function to start an interval timer
 * 
 * @return void
 */
static void starttimer(){
    struct itimerval it_val;
    it_val.it_interval.tv_sec = 1;
    it_val.it_interval.tv_usec = 0;
    it_val.it_value.tv_sec = 1;
    it_val.it_value.tv_usec = 0;
    if (setitimer(ITIMER_VIRTUAL, &it_val, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
    return;
}

/**
 * @brief Function to make a thread handle timer interrupts
 * 
 * @return void
 */
void enabletimer(){
    if(signal(SIGVTALRM, switchToScheduler) == SIG_ERR){
        perror("Timer ");
        exit(EXIT_FAILURE); 
    };
    return;
}

/**
 * @brief Function to make a thread ignore timer interrupts
 * 
 * @return void
 */
void disabletimer(){
    if(signal(SIGVTALRM,SIG_IGN) == SIG_ERR){
        perror("Timer ");
        exit(EXIT_FAILURE);
    };
    return;
}

void createContext(sigjmp_buf *context, void *routine){
    sigsetjmp(*context,1);
    if(routine) context[0]->__jmpbuf[JB_SP] = translate_address((address_t)(allocStack(STACK_SZ,0) + STACK_SZ - sizeof(int)));
    if(routine) context[0]->__jmpbuf[JB_PC] = translate_address((address_t)routine);
}

void setContext(sigjmp_buf *context){
    siglongjmp(*context,1);
}

void raiseSignals(){
    if(!__curproc){
        return;
    }
    int k = __curproc->numPendingSig;
    sigset_t mask;
    for(int i = 0 ;i < k;i++) {
        // log_trace("Signal %d pending for %ld",__curproc->pendingSig[i],__curproc->tid);
        sigaddset(&mask,__curproc->pendingSig[i]);
        sigprocmask(SIG_UNBLOCK,&mask,NULL);
        // Remove from the pending list
        __curproc->numPendingSig -= 1;
        raise(__curproc->pendingSig[i]);
    }
}

void switchContext(sigjmp_buf *old, sigjmp_buf *new){
    int ret = sigsetjmp(*old,1);
    if(ret == 0){
        siglongjmp(*new,1);
    }
    return;
}


/**
 * @brief Function to switch to the scheduler's context, acts as a coroutine to scheduler()
 * 
 * @return void
 */
void switchToScheduler(){
    switchContext(__curproc->ctx,__scheduler->ctx);
    // if(swapcontext(__curproc->context, __scheduler->context) == -1){
    //     perror("Context switch");
    //     exit(EXIT_FAILURE); 
    // }
    
    log_trace("%d Returned here", __curproc->tid);
    raiseSignals();
    enabletimer();
}

/**
 * @brief Function to run the scheduler, acts as a coroutine to switchToScheduler()
 * 
 * @return void
 */
static void scheduler(){
    disabletimer();
    queueRunning(&__allThreads);
    int curprocexited = 0;
    for(int i = 0; i < numExited; i++){
        tcb* temp = getThread(&__allThreads, exited[i]);
        if(temp->tid == __curproc->tid){
            curprocexited = 1;
        }
        for(int j = 0; j < temp->numWaiters; j++){
            tcb* setRunnable = getThread(&__allThreads, temp->waiters[i]);
            setRunnable->thread_state = RUNNABLE;
            reQueue(&__allThreads, setRunnable);
        }
        removeThread(&__allThreads, temp->tid);
    }
    if(numExited){
        free(exited);
        exited = NULL;
    }
    numExited = 0;
    setSignals();
    tcb *next = getNextThread(&__allThreads);
    if(!next) return;
    
    // Set new thread as running and current thread as runnable
    if(curprocexited == 0){
        tcb* __prev = __curproc;
        __curproc = next;
        next->thread_state = RUNNING;
        // log_trace("Scheduler: switching from %d to %d", __prev->tid, next->tid);
        // if(setcontext(next->context) == -1){
        //     perror("Context Switch: ");
        //     exit(EXIT_FAILURE);
        // }
        setContext(next->ctx);
    }
    else{
        __curproc = next;
        next->thread_state = RUNNING;
        // log_trace("Scheduler: switching from deleted to %d", next->tid);
        // if(setcontext(next->context) == -1){
        //     perror("Context Switch: ");
        //     exit(EXIT_FAILURE);
        // }
        setContext(next->ctx);
    }
    
}

/**
 * @brief Library initialzer
 * 
 */
static void initManyOne(){
    log_info("Library initialized");
    spin_init(&globallock);
    setSignals();    
    __allThreads.back = NULL;
    __allThreads.front = NULL;
    __allThreads.len = 0;
    
    __mainproc = (tcb*)malloc(sizeof(tcb));
    ucontext_t* thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    sigjmp_buf *ctx = (sigjmp_buf *)malloc(sizeof(sigjmp_buf));
    // Main's context (has default stack and PC)
    createContext(ctx,NULL);
    //    
    // getcontext(thread_context);
    initTcb(__mainproc, RUNNING, getpid(), thread_context,ctx);
    
    __curproc = __mainproc;
    
    __nextpid = getpid()+1;
    addThread(&__allThreads,__mainproc);
    
    __scheduler = (tcb*)malloc(sizeof(tcb));
    thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    ctx = (sigjmp_buf *)malloc(sizeof(sigjmp_buf));
    // Schedulers context (has a new stack and PC)
    createContext(ctx,scheduler);
    //
    // getcontext(thread_context);
    

    thread_context->uc_stack.ss_sp = allocStack(STACK_SZ,0);
    thread_context->uc_stack.ss_size = STACK_SZ;
    thread_context->uc_link = __mainproc->context;
    
    // makecontext(thread_context, scheduler, 0);
    initTcb(__scheduler, RUNNING, 0, thread_context,ctx);

    starttimer();
}


/**
 * @brief Wrapper function that internally invokes the thread routine
 * 
 * @return void
 */
void wrapRoutine(){
    WRAP_SIGNALS;
    // raiseSignals();
    funcargs *temp = __curproc->args;
    enabletimer();
    (temp->f)(temp->arg);
    disabletimer();
    exited = (int*)realloc(exited, (++numExited)*sizeof(int));
    exited[numExited-1] = __curproc->tid; 
    free(temp);
    enabletimer();
    switchToScheduler();
    // while(1){}
}

/**
 * 
 * @brief Create a Many One mapped thread  
 * 
 * @param t Reference to the thread
 * @param routine Function associated with the thread
 * @param attr Thread attributes
 * @param arg Arguments to the routine
 * @return thread 
 */
int thread_create(thread *t, void *attr, void *routine, void *arg){
    // Disable any switches during creation
    disabletimer();
    // Initalise the state of the main thread
    static int isInit = 0;
    if(!isInit){
        initManyOne();
        isInit = 1;
    }
    ucontext_t *thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    sigjmp_buf *ctx = (sigjmp_buf *)malloc(sizeof(sigjmp_buf));

    // Allocate a new TCB and set all fields
    tcb *temp = (tcb *)malloc(sizeof(tcb));
    // Context of new thread (with new stack and PC)
    //
    // getcontext(thread_context);
    
    funcargs* fa = (funcargs*)malloc(sizeof(funcargs));
    fa->f = routine;
    fa->arg = arg;
    createContext(ctx,wrapRoutine);

    temp->args = fa;
    // Context of thread is modified to accepts timer interrupts
    if(attr){
        if(((thread_attr *)attr)->stack){
            thread_context->uc_stack.ss_sp = ((thread_attr *)attr)->stack;
            thread_context->uc_stack.ss_size = ((thread_attr *)attr)->stackSize;
            ctx[0]->__jmpbuf[6] = (long)(((thread_attr *)attr)->stack + ((thread_attr *)attr)->stackSize);
        }
        else if(((thread_attr *)attr)->stackSize){
            temp->stack = allocStack(((thread_attr *)attr)->stackSize,0);
            thread_context->uc_stack.ss_sp = temp->stack;
            thread_context->uc_stack.ss_size = ((thread_attr *)attr)->stackSize;
            ctx[0]->__jmpbuf[6] = (long)(temp->stack + ((thread_attr *)attr)->stackSize);
        }
    }
    else{
        // thread_context->uc_stack.ss_sp = attr != NULL ? allocStack(STACK_SZ,0) : allocStack(((thread_attr *)attr)->stackSize,0);
        temp->stack = allocStack(STACK_SZ,0);
        thread_context->uc_stack.ss_sp = temp->stack;
        thread_context->uc_stack.ss_size = STACK_SZ;
    }
    thread_context->uc_link = __mainproc->context;

    // makecontext(thread_context,(void*)&wrapRoutine,1,(void *)(fa));
    initTcb(temp, RUNNABLE, __nextpid++, thread_context,ctx);
    // Add the thread to list of runnable threads
    addThread(&__allThreads,temp);
    *t = temp->tid;
    sigsetjmp(*__mainproc->ctx,1);
    // Start the timer for the main thread
    enabletimer();
    return 0;
}

/**
 * @brief Function to wait for a specific thread to terminate
 * 
 * @param t TID of the thread to wait for
 * @param guard Size of guard pag
 * @return int
 */
int thread_join(thread t, void **retLocation){
    // log_info("trying to join thread %d\n", t);
    disabletimer();
    tcb* waitedThread = getThread(&__allThreads, t);
    if(waitedThread == NULL){
        if(retLocation) *retLocation = (void *)ESRCH;
        enabletimer();
        return ESRCH;
    }
    // check if not joinable, check if another thread is waiting (or not?)
    if(waitedThread->exited){
        if(retLocation) *retLocation = (void *)0;
        enabletimer();
        return 0;
    }
    //Add thread to the list of waiters
    waitedThread->waiters = (int*)realloc(waitedThread->waiters, (++(waitedThread->numWaiters))*sizeof(int));
    waitedThread->waiters[waitedThread->numWaiters-1] = __curproc->tid;
    __curproc->thread_state = WAITING;
    switchToScheduler();
    if(retLocation) *retLocation = (void *)0;
    return 0;
}

/**
 * @brief Function to send signals to a specific thread
 * 
 * @param t TID of the thread to which the signal has to be sent
 * @param signum Signal number of the signal to be sent to the thread
 * @return int
 */
int thread_kill(pid_t t, int signum){
    disabletimer();
    if(signum == SIGINT || signum == SIGCONT || signum == SIGSTOP){
        kill(getpid(),signum);
    }
    else{
        if(__curproc->tid == t){
            raise(signum);
            enabletimer();
            return 0;
        }
        tcb *temp = getThread(&__allThreads,t);
        // A memory leak here
        temp->pendingSig = (int *)realloc(temp->pendingSig, (++(temp->numPendingSig) * sizeof(int)));
        temp->pendingSig[temp->numPendingSig - 1] = signum;
    }
    enabletimer();
}

int thread_exit(void *retVal){
    disabletimer();
    log_trace("Exiting thread %d",__curproc->tid);
    exited = (int*)realloc(exited, (++numExited)*sizeof(int));
    exited[numExited-1] = __curproc->tid;
    switchToScheduler(); 
    // enabletimer();
}