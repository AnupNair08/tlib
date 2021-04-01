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
#include <pthread.h>
#include "thread.h"
#include "tlibtypes.h"
#include "attributetypes.h"
#include "dataStructTypes.h"
#include "log.h"

mut_t globalLock;
singlyLL tidList;
int handledSignal = 0;

/**
 * @brief Library initialzer
 * 
 */
static void init(){
    log_info("Library initialised\n");
    fflush(stdout);
    singlyLLInit(&tidList);
    node * insertedNode = singlyLLInsert(&tidList, getpid());
    if(insertedNode == NULL){
        log_error("Thread address not found");
        // spin_release(&globalLock);
        return;
    }
    insertedNode->tidCpy = getpid();
    spin_init(&globalLock);
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
// static int wrap(void *fa){
//     spin_acquire(&globalLock);
//     spin_release(&globalLock);
//     funcargs *temp;
//     temp = (funcargs *)fa;
//     signal(SIGINT,SIG_DFL);
//     signal(SIGSTOP,SIG_DFL);
//     signal(SIGCONT,SIG_DFL);
//     handledSignal = 1;
//     temp->f(temp->arg);
//     return 0;
// }

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
    // spin_acquire(&globalLock);
    fflush(stdout);
    thread tid;
    void *thread_stack;
    int status;
    if(initState == 0){
        initState = 1;
        init();
    }
    node * insertedNode = singlyLLInsert(&tidList, 0);
    if(insertedNode == NULL){
        log_error("Thread address not found");
        // spin_release(&globalLock);
        return -1;
    }
    if(attr){
        thread_attr *attr_t = (thread_attr *)attr;
        thread_stack = attr_t->stack == NULL ? allocStack(attr_t->stackSize, attr_t->guardSize) : attr_t->stack ;
        if(!thread_stack) {
            perror("tlib create");
            // spin_release(&globalLock);
            return errno;
        }
        tid = clone(routine,
                    thread_stack + ((thread_attr *)attr)->stackSize + ((thread_attr *)attr)->guardSize, 
                    CLONE_FLAGS,
                    arg,
                    &(insertedNode->tid),
                    NULL, 
                    &(insertedNode->tid));
        insertedNode->tidCpy = tid;
    }
    else{
        thread_stack = allocStack(STACK_SZ,GUARD_SZ);
        if(!thread_stack) {
            perror("tlib create");
            // spin_release(&globalLock);
            return errno;
        }
        tid = clone(routine,
                    thread_stack + STACK_SZ + GUARD_SZ,
                    CLONE_FLAGS,
                    arg,
                    &(insertedNode->tid),
                    NULL, 
                    &(insertedNode->tid));
        insertedNode->tidCpy = tid;
    }
    if(tid == -1){
        perror("tlib create");
        free(thread_stack);
        // spin_release(&globalLock);
        return errno;
    }
    *t = tid;
    // spin_release(&globalLock);
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
       killAllThreads(&tidList, signum);
       pid_t pid = getpid();
       ret = syscall(TGKILL, pid, gettid(), signum);
       if(ret == -1){
        perror("tgkill");
        return errno;
        }
        return ret;
    }
    pid_t pid = getpid();
    ret = syscall(TGKILL, pid, tid, signum);
    if(ret == -1){
        perror("tgkill");
        return errno;
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
    void *addr = returnCustomTidAddress(&tidList, t);
    if(addr == NULL){
        // spin_release(&globalLock);
        return ESRCH;
    }
    if(*((pid_t*)addr) == 0){
        singlyLLDelete(&tidList, t);
        // spin_release(&globalLock);
        return EINVAL;
    }
    int ret;
    while(*((pid_t*)addr) == t){
        ret = syscall(SYS_futex , addr, FUTEX_WAIT, t, NULL, NULL, 0);
    }
    syscall(SYS_futex , addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    singlyLLDelete(&tidList, t);
    if(retLocation) *retLocation = (void *)&ret;
    // spin_release(&globalLock);
    return ret;
}


void thread_exit(void *ret){
    void *addr = returnCustomTidAddress(&tidList, gettid());
    if(addr == NULL){
        log_info("Thread already exited");
        return;
    }
    syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    singlyLLDelete(&tidList, gettid());
    kill(SIGINT,gettid());
    return;
}
