/**
 * @file thread.c
 * @author Hrishikesh Athalye & Anup Nair
 * @brief Implementation of thread APIs in many many mapping
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#define DEV
#include <stdio.h>
#include <sched.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <limits.h>
#include "tattr.h"
#include "utils.h"
#include "sighandler.h"
#include "mangle.h"
tcb *__curproc = NULL;
tcb *__scheduler = NULL;
tcb *__mainproc = NULL;
tcbQueue __allThreads;
sigset_t __signalList;
unsigned long int __nextpid;
schedParams __def = {sc : 0, ms : 100};

/**
 * @brief Function to allocate a stack to Many One threads
 * 
 * @param size Size of stack excluding the guard size
 * @param guard Size of guard page
 * @return void
 */
static void *allocStack(size_t size, size_t guard)
{
    void *stack = NULL;
    //Align the memory to a 64 bit compatible page size and associate a guard area for the stack
    stack = mmap(NULL, size + guard, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
    {
        perror("Stack Allocation");
        return NULL;
    }
    if (mprotect(stack, guard, PROT_NONE))
    {
        munmap(stack, size + guard);
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
static void setSignals()
{
    sigfillset(&__signalList);
    sigdelset(&__signalList, SIGINT);
    sigdelset(&__signalList, SIGSTOP);
    sigdelset(&__signalList, SIGCONT);
    sigdelset(&__signalList, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &__signalList, NULL);
}

/**
 * @brief Function to start an interval timer
 * 
 * @return void
 */
static void starttimer(schedParams *interval)
{
    struct itimerval it_val;
    it_val.it_interval.tv_sec = interval->sc;
    it_val.it_interval.tv_usec = interval->ms;
    it_val.it_value.tv_sec = interval->sc;
    it_val.it_value.tv_usec = interval->ms;
    if (setitimer(ITIMER_VIRTUAL, &it_val, NULL) == -1)
    {
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
void enabletimer()
{
    if (signal(SIGVTALRM, switchToScheduler) == SIG_ERR)
    {
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
void disabletimer()
{
    if (signal(SIGVTALRM, SIG_IGN) == SIG_ERR)
    {
        perror("Timer ");
        exit(EXIT_FAILURE);
    };
    return;
}

void createContext(sigjmp_buf *context, void *routine, void *stack)
{
    sigsetjmp(*context, 1);
    context[0]->__jmpbuf[JB_BP] = translate_address((address_t)(stack - sizeof(int)));
    context[0]->__jmpbuf[JB_SP] = context[0]->__jmpbuf[JB_BP];
    if (routine)
    {
        context[0]->__jmpbuf[JB_PC] = translate_address((address_t)routine);
    }
}

void setContext(sigjmp_buf *context)
{
    siglongjmp(*context, 1);
}

void raiseSignals()
{
    if (!__curproc)
    {
        return;
    }
    int k = __curproc->numPendingSig;
    sigset_t *mask = (sigset_t *)malloc(sizeof(sigset_t));
    for (int i = 0; i < k; i++)
    {
        sigaddset(mask, __curproc->pendingSig[i]);
        sigprocmask(SIG_UNBLOCK, mask, NULL);
        __curproc->numPendingSig -= 1;
        kill(getpid(), SIGTERM);
    }
    setSignals();
    free(mask);
}

void switchContext(sigjmp_buf *old, sigjmp_buf *new)
{
    int ret = sigsetjmp(*old, 1);
    if (ret == 0)
    {
        siglongjmp(*new, 1);
    }
    return;
}

/**
 * @brief Function to switch to the scheduler's context, acts as a coroutine to scheduler()
 * 
 * @return void
 */
void switchToScheduler()
{
    raiseSignals();
    switchContext(__curproc->ctx, __scheduler->ctx);
    enabletimer();
}

/**
 * @brief Function to run the scheduler, acts as a coroutine to switchToScheduler()
 * 
 * @return void
 */
static void scheduler()
{
    disabletimer();
    setSignals();
    queueRunning(&__allThreads);
    int curprocexited = __curproc->exited;
    removeExitedThreads(&__allThreads);
    tcb *next = getNextThread(&__allThreads);
    if (!next)
        return;

    if (curprocexited == 0)
    {
        tcb *__prev = __curproc;
        __curproc = next;
        next->thread_state = RUNNING;
        setContext(next->ctx);
    }
    else
    {
        __curproc = next;
        next->thread_state = RUNNING;
        setContext(next->ctx);
    }
}

/**
 * @brief Library initialzer
 * 
 */
static void initManyOne(schedParams interval)
{
    printf("Library initialized\n");

    setSignals();

    __allThreads.back = NULL;
    __allThreads.front = NULL;
    __allThreads.len = 0;

    __mainproc = (tcb *)malloc(sizeof(tcb));
    sigjmp_buf *ctx = (sigjmp_buf *)malloc(sizeof(sigjmp_buf));
    // Main's context (has default stack and PC)
    createContext(ctx, NULL, NULL);
    initTcb(__mainproc, RUNNING, getpid(), ctx);

    __curproc = __mainproc;

    __nextpid = getpid() + 1;
    addThread(&__allThreads, __mainproc);

    __scheduler = (tcb *)malloc(sizeof(tcb));
    ctx = (sigjmp_buf *)malloc(sizeof(sigjmp_buf));
    // Schedulers context (has a new stack and PC)
    void *schedStack = allocStack(STACK_SZ, 0) + STACK_SZ;
    createContext(ctx, scheduler, schedStack);

    // makecontext(thread_context, scheduler, 0);
    initTcb(__scheduler, RUNNING, 0, ctx);

    starttimer(&interval);
}

/**
 * @brief Wrapper function that internally invokes the thread routine
 * 
 * @return void
 */
void wrapRoutine()
{
    WRAP_SIGNALS;
    funcargs *temp = __curproc->args;
    enabletimer();
    (temp->f)(temp->arg);
    disabletimer();
    __curproc->exited = 1;
    free(temp);
    switchToScheduler();
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
int thread_create(thread *t, void *attr, void *routine, void *arg)
{
    // Handle errors
    if (t == NULL || routine == NULL)
    {
        return EINVAL;
    }
    // Disable any switches during creation
    disabletimer();
    // Initalise the state of the main thread
    static int isInit = 0;
    if (!isInit)
    {
        if (attr)
        {
            int ms = ((thread_attr *)attr)->schedInterval.ms;
            int sc = ((thread_attr *)attr)->schedInterval.sc;
            if (ms != 0 || sc != 0)
            {
                initManyOne(((thread_attr *)attr)->schedInterval);
            }
        }
        else
        {
            initManyOne(__def);
        }
        isInit = 1;
    }
    sigjmp_buf *ctx = (sigjmp_buf *)malloc(sizeof(sigjmp_buf));

    // Allocate a new TCB and set all fields
    tcb *temp = (tcb *)malloc(sizeof(tcb));

    funcargs *fa = (funcargs *)malloc(sizeof(funcargs));
    fa->f = routine;
    fa->arg = arg;

    temp->args = fa;
    if (attr)
    {
        if (((thread_attr *)attr)->stack)
        {
            ctx[0]->__jmpbuf[JB_SP] = translate_address((address_t)((thread_attr *)attr)->stack + ((thread_attr *)attr)->stackSize);
            createContext(ctx, wrapRoutine, ((thread_attr *)attr)->stack + ((thread_attr *)attr)->stackSize);
        }
        else if (((thread_attr *)attr)->stackSize)
        {
            temp->stack = allocStack(((thread_attr *)attr)->stackSize, 0);
            createContext(ctx, wrapRoutine, temp->stack + ((thread_attr *)attr)->stackSize);
        }
    }
    else
    {
        temp->stack = allocStack(STACK_SZ, 0) + STACK_SZ;
        createContext(ctx, wrapRoutine, temp->stack);
    }

    initTcb(temp, RUNNABLE, __nextpid++, ctx);
    // Add the thread to list of runnable threads
    addThread(&__allThreads, temp);
    *t = temp->tid;
    sigsetjmp(*__mainproc->ctx, 1);
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
int thread_join(thread t, void **retLocation)
{
    disabletimer();
    tcb *waitedThread = getThread(&__allThreads, t);
    if (waitedThread == NULL)
    {
        if (retLocation)
            *retLocation = (void *)ESRCH;
        enabletimer();
        return ESRCH;
    }
    if (waitedThread->exited)
    {
        if (retLocation)
            *retLocation = (void *)EINVAL;
        enabletimer();
        return EINVAL;
    }
    //Add thread to the list of waiters
    waitedThread->waiters = (int *)realloc(waitedThread->waiters, (++(waitedThread->numWaiters)) * sizeof(int));
    waitedThread->waiters[waitedThread->numWaiters - 1] = __curproc->tid;
    __curproc->thread_state = WAITING;
    switchToScheduler();
    return 0;
}

/**
 * @brief Function to send signals to a specific thread
 * 
 * @param t TID of the thread to which the signal has to be sent
 * @param signum Signal number of the signal to be sent to the thread
 * @return int
 */
int thread_kill(pid_t t, int signum)
{
    if (signum <= 0)
    {
        return -1;
    }
    int ret = 0;
    disabletimer();
    if (signum == SIGINT || signum == SIGCONT || signum == SIGSTOP)
    {
        kill(getpid(), signum);
    }
    else
    {
        if (__curproc->tid == t)
        {
            raise(signum);
            enabletimer();
            return ret;
        }
        tcb *temp = getThread(&__allThreads, t);
        if (temp)
        {
            temp->pendingSig = (int *)realloc(temp->pendingSig, (++(temp->numPendingSig) * sizeof(int)));
            temp->pendingSig[temp->numPendingSig - 1] = signum;
        }
        else
        {
            ret = -1;
        }
    }
    enabletimer();
    return ret;
}

int thread_exit(void *retVal)
{
    disabletimer();
    __curproc->exited = 1;
    switchToScheduler();
}
