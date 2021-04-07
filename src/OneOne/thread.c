#define _GNU_SOURCE
#define DEV
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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <limits.h>
#include <ucontext.h>
#include "tlib.h"
#include "utils.h"
#include "log.h"
#include "locks.h"
#include "sighandler.h"

spin_t globalLock;
singlyLL __tidList;

/**
 * @brief Library initialzer
 * 
 */
static void init(){
    log_info("Library initialised\n");
    fflush(stdout);
    spin_init(&globalLock);

    sigset_t signalMask;
    sigfillset(&signalMask);
    sigdelset(&signalMask,SIGINT);
    sigdelset(&signalMask,SIGSTOP);
    sigdelset(&signalMask,SIGCONT);
    sigprocmask(SIG_BLOCK,&signalMask,NULL);
    spin_acquire(&globalLock);
    singlyLLInit(&__tidList);
    node * insertedNode = singlyLLInsert(&__tidList, getpid());
    spin_release(&globalLock);
    if(insertedNode == NULL){
        log_error("Thread address not found");
        // spin_release(&globalLock);
        return;
    }
    insertedNode->tidCpy = getpid();
}

/**
 * @brief Function to allocate a stack to One One threads
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
 * @brief Wrapper for the routine passed to the thread
 * 
 * @param fa Function pointer of the routine passed to the thread
 * @return int 
 */
static int wrap(void *fa){
    funcargs *temp;
    temp = (funcargs *)fa;
    sigset_t base_mask;
    sigset_t maskArr[5];
    int sigArr[5] = {SIGTERM, SIGFPE, SIGSYS, SIGABRT, SIGPIPE};
    for(int i = 0 ;i < 5;i++){
        base_mask = maskArr[i];
        WRAP_SIGNALS(sigArr[i]);
    }
    temp->f(temp->arg);
    // free(temp->stack);
    register int i asm("eax");
    int regval = i;
    temp->insertedNode->retVal = (void *)&regval;
    // log_trace("Thread exited with return value %d", regval);
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
int thread_create(thread *t,void *attr,void * routine, void *arg){
    static int initState = 0;
    fflush(stdout);
    thread tid;
    void *thread_stack;
    int status;
    if(initState == 0){
        initState = 1;
        init();
    }
    spin_acquire(&globalLock);
    node * insertedNode = singlyLLInsert(&__tidList, 0);
    spin_release(&globalLock);
    if(insertedNode == NULL){
        log_error("Thread address not found");
        return -1;
    }
    funcargs *fa = (funcargs *)malloc(sizeof(funcargs));
    fa->f = routine;
    fa->arg = arg;
    fa->insertedNode = insertedNode;
    if(attr){
        thread_attr *attr_t = (thread_attr *)attr;
        thread_stack = attr_t->stack == NULL ? allocStack(attr_t->stackSize, attr_t->guardSize) : attr_t->stack ;
        if(!thread_stack) {
            perror("tlib create");
            return errno;
        }
        fa->stack = thread_stack;
        tid = clone(wrap,
                    thread_stack + ((thread_attr *)attr)->stackSize + ((thread_attr *)attr)->guardSize, 
                    CLONE_FLAGS,
                    (void *)fa,
                    &(insertedNode->tid),
                    NULL, 
                    &(insertedNode->tid));
        spin_acquire(&globalLock);
        insertedNode->tidCpy = tid;
        spin_release(&globalLock);
    }
    else{
        thread_stack = allocStack(STACK_SZ,GUARD_SZ);
        if(!thread_stack) {
            perror("tlib create");
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
        spin_acquire(&globalLock);
        insertedNode->tidCpy = tid;
        spin_release(&globalLock);
    }
    if(tid == -1){
        perror("tlib create");
        free(thread_stack);
        return errno;
    }
    *t = tid;
    return 0;
}

/**
 * @brief Function to send signals to a specific thread
 * 
 * @param tid TID of the thread to which the signal has to be sent
 * @param signum Signal number of the signal to be sent to the thread
 * @return int
 */
int thread_kill(pid_t tid, int signum){
    int ret;
    if(signum == SIGINT || signum == SIGCONT || signum == SIGSTOP){
       killAllThreads(&__tidList, signum);
       pid_t pid = getpid();
       ret = syscall(TGKILL, pid, gettid(), signum);
       if(ret == -1){
        perror("tgkill");
        return ret;
        }
        return ret;
    }
    pid_t pid = getpid();
    ret = syscall(TGKILL, pid, tid, signum);
    if(ret == -1){
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
int thread_join(thread t, void **retLocation){
    // spin_acquire(&globalLock);
    int status;
    void *addr = returnCustomTidAddress(&__tidList, t);
    if(addr == NULL){
        // spin_release(&globalLock);
        return ESRCH;
    }
    if(*((pid_t*)addr) == 0){
        singlyLLDelete(&__tidList, t);
        // spin_release(&globalLock);
        return EINVAL;
    }
    int ret;
    while(*((pid_t*)addr) == t){
        ret = syscall(SYS_futex , addr, FUTEX_WAIT, t, NULL, NULL, 0);
    }
    syscall(SYS_futex , addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    if(retLocation){
        log_trace("Return value %x %x",*retLocation, retLocation);
        *retLocation = getReturnValue(&__tidList, t);
    } 
    singlyLLDelete(&__tidList, t);
    // spin_release(&globalLock);
    return ret;
}


/**
 * @brief Function to make a thread terminate itself
 * 
 * @param ret return value of the thread to be available to thread_join()
 * @return void
 */
void thread_exit(void *ret){
    void *addr = returnCustomTidAddress(&__tidList, gettid());
    if(addr == NULL){
        log_info("Thread already exited");
        return;
    }
    syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    singlyLLDelete(&__tidList, gettid());
    kill(SIGINT,gettid());
    return;
}
