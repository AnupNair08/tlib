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
#include <linux/futex.h>
#include <linux/unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <limits.h>
#include "tlib.h"
#include "utils.h"
#include "locks.h"
#include "sighandler.h"
spin_t globalLock;
singlyLL __tidList;
thread currTid;

void cleanup()
{
    deleteAllThreads(&__tidList);
    free(__tidList.head);
}

/**
 * @brief Library initialzer
 * 
 */
static void init()
{
    printf("Library initialised\n");
    spin_init(&globalLock);
    sigset_t signalMask;
    sigfillset(&signalMask);
    sigdelset(&signalMask, SIGINT);
    sigdelset(&signalMask, SIGSTOP);
    sigdelset(&signalMask, SIGCONT);
    sigprocmask(SIG_BLOCK, &signalMask, NULL);
    singlyLLInit(&__tidList);
    node *insertedNode = singlyLLInsert(&__tidList, getpid());
    insertedNode->tidCpy = insertedNode->tid;
    insertedNode->fa = NULL;
    atexit(cleanup);
    return;
}

/**
 * @brief Function to allocate a stack to One One threads
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
 * @brief Wrapper for the routine passed to the thread
 * 
 * @param fa Function pointer of the routine passed to the thread
 * @return int 
 */
static int wrap(void *fa)
{
    funcargs *temp;
    temp = (funcargs *)fa;
    sigset_t base_mask;
    sigset_t maskArr[5];
    int sigArr[5] = {SIGTERM, SIGFPE, SIGSYS, SIGABRT, SIGPIPE};
    for (int i = 0; i < 5; i++)
    {
        base_mask = maskArr[i];
        WRAP_SIGNALS(sigArr[i]);
    }
    temp->f(temp->arg);
    thread_exit(NULL);
}

/**
 * 
 * @brief Create a One One mapped thread  
 * 
 * @param t Reference to the thread
 * @param routine Function associated with the thread
 * @param attr Thread attributes
 * @param arg Arguments to the routine
 * @return thread 
 */
int thread_create(thread *t, void *attr, void *routine, void *arg)
{
    spin_acquire(&globalLock);
    static int initState = 0;
    if (t == NULL || routine == NULL)
    {
        spin_release(&globalLock);
        return EINVAL;
    }
    thread tid;
    void *thread_stack;
    if (initState == 0)
    {
        initState = 1;
        init();
    }
    node *insertedNode = singlyLLInsert(&__tidList, 0);
    if (insertedNode == NULL)
    {
        printf("Thread address not found\n");
        spin_release(&globalLock);
        return -1;
    }
    funcargs *fa;
    fa = (funcargs *)malloc(sizeof(funcargs));
    if (!fa)
    {
        printf("Malloc failed\n");
        spin_release(&globalLock);
        return -1;
    }
    fa->f = routine;
    fa->arg = arg;
    if (attr)
    {
        thread_attr *attr_t = (thread_attr *)attr;
        thread_stack = attr_t->stack == NULL ? allocStack(attr_t->stackSize, attr_t->guardSize) : attr_t->stack;
        if (!thread_stack)
        {
            perror("tlib create");
            spin_release(&globalLock);
            return errno;
        }
        fa->stack = attr_t->stack == NULL ? thread_stack : attr_t->stack;
        tid = clone(wrap,
                    thread_stack + ((thread_attr *)attr)->stackSize + ((thread_attr *)attr)->guardSize,
                    CLONE_FLAGS,
                    (void *)fa,
                    &(insertedNode->tid),
                    NULL,
                    &(insertedNode->tid));
        insertedNode->tidCpy = tid;
        insertedNode->fa = fa;
    }
    else
    {
        thread_stack = allocStack(STACK_SZ, GUARD_SZ);
        if (!thread_stack)
        {
            perror("tlib create");
            spin_release(&globalLock);
            return errno;
        }
        fa->stack = thread_stack;
        tid = clone(wrap,
                    thread_stack + STACK_SZ + GUARD_SZ,
                    CLONE_FLAGS,
                    (void *)fa,
                    &(insertedNode->tid),
                    NULL,
                    &(insertedNode->tid));
        insertedNode->tidCpy = tid;
        insertedNode->fa = fa;
    }
    if (tid == -1)
    {
        perror("tlib create");
        free(thread_stack);
        spin_release(&globalLock);
        return errno;
    }
    *t = tid;
    spin_release(&globalLock);
    return 0;
}

/**
 * @brief Function to send signals to a specific thread
 * 
 * @param tid TID of the thread to which the signal has to be sent
 * @param signum Signal number of the signal to be sent to the thread
 * @return int
 */
int thread_kill(pid_t tid, int signum)
{
    if (signum == 0)
    {
        return -1;
    }
    int ret;
    node *insertedNode = returnCustomNode(&__tidList, tid);
    if (signum == SIGINT || signum == SIGCONT || signum == SIGSTOP)
    {
        killAllThreads(&__tidList, signum);
        pid_t pid = getpid();
        ret = syscall(TGKILL, pid, gettid(), signum);
        if (ret == -1)
        {
            perror("tgkill");
            return ret;
        }
        return ret;
    }
    if (insertedNode->tid == 0)
    {
        return -1;
    }
    pid_t pid = getpid();
    ret = syscall(TGKILL, pid, tid, signum);
    if (ret == -1)
    {
        perror("tgkill");
        return ret;
    }
    return ret;
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
    spin_acquire(&globalLock);
    void *addr = returnCustomTidAddress(&__tidList, t);
    if (addr == NULL)
    {
        spin_release(&globalLock);
        return ESRCH;
    }
    if (*((pid_t *)addr) == 0)
    {
        spin_release(&globalLock);
        return EINVAL;
    }
    int ret;
    while (*((pid_t *)addr) == t)
    {
        spin_release(&globalLock);
        ret = syscall(SYS_futex, addr, FUTEX_WAIT, t, NULL, NULL, 0);
        spin_acquire(&globalLock);
    }
    syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    if (retLocation)
    {
        *retLocation = getReturnValue(&__tidList, t);
    }
    spin_release(&globalLock);
    return ret;
}

/**
 * @brief Function to make a thread terminate itself
 * 
 * @param ret return value of the thread to be available to thread_join()
 * @return void
 */
void thread_exit(void *ret)
{
    spin_acquire(&globalLock);
    void *addr = returnCustomTidAddress(&__tidList, gettid());
    if (addr == NULL)
    {
        spin_release(&globalLock);
        return;
    }
    if (ret)
    {
        ret = getReturnValue(&__tidList, gettid());
    }
    node *insertedNode = returnCustomNode(&__tidList, gettid());
    syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    spin_release(&globalLock);
    return;
}
