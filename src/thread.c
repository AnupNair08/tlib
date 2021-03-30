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

singlyLL tidList;
int handledSignal = 0;

/**
 * @brief Library initialzer
 * 
 */
static void init(){
    log_info("Library initialised\n");
    //Initialise necessay data structures
    singlyLLInit(&tidList);
    // singlyLLInsert(&tidList, getpid());
}

/**
 * @brief Umbrella function which calls specific thread function
 * 
 * @param t Reference to the thread 
 * @param routine Function associated with the thread
 * @param attr Thread attributes
 * @param arg Arguments to the routine
 * @param threadMode Mapping model to be used (0 = One One , 1 = Many One, 2 = Many Many)
 * @return thread 
 */
int thread_create(thread *t,void *attr,void * routine,void *arg, int threadMode){
    switch(threadMode){
        case 0:
            return createOneOne(t, attr,routine, arg);
        case 1:
            return createManyOne(t, attr,routine, arg);
        case 2:
            return createManyMany(t, attr,routine, arg);
    }
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


/**
 * @brief Wrapper for the routine passed to the thread
 * 
 * @param fa Function pointer of the routine passed to the thread
 * @return int 
 */
static int wrap(void *fa){
    // printf("Signals handled\n");
    signal(SIGINT,SIG_DFL);
    signal(SIGSTOP,SIG_DFL);
    signal(SIGCONT,SIG_DFL);
    handledSignal = 1;
    funcargs *temp;
    temp = (funcargs *)fa;
    temp->f(temp->arg);
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
int createOneOne(thread *t,void *attr,void * routine, void *arg){
    static int initState = 0;

    thread tid;
    void *thread_stack;
    int status;
    if(initState == 0){
        initState = 1;
        init();
    }
    funcargs fa;
    fa.f = routine;
    fa.arg = arg;
    singlyLLInsert(&tidList, 0);
    if(attr){
        thread_attr *attr_t = (thread_attr *)attr;
        thread_stack = attr_t->stack == NULL ? allocStack(attr_t->stackSize, attr_t->guardSize) : attr_t->stack ;
        if(!thread_stack) {
            perror("tlib create");
            return errno;
        }
 
        void * addr = returnTailTidAddress(&tidList);
         if(addr == NULL){
            log_error("Thread address not found");
            return -1;
        }
        tid = clone(wrap,
                    thread_stack + ((thread_attr *)attr)->stackSize + ((thread_attr *)attr)->guardSize, 
                    CLONE_FLAGS,
                    (void*)&fa,
                    addr,
                    NULL, 
                    addr);
        persistTid(&tidList);
    }
    else{
        thread_stack = allocStack(STACK_SZ,GUARD_SZ);
        if(!thread_stack) {
            perror("tlib create");
            return errno;
        }
        void * addr = returnTailTidAddress(&tidList);
        if(addr == NULL){
            log_error("Thread address not found");
            return -1;
        }
        tid = clone(wrap,
                    thread_stack + STACK_SZ + GUARD_SZ,
                    CLONE_FLAGS,
                    (void *)&fa,
                    addr,
                    NULL, 
                    addr);
        persistTid(&tidList);
    }
    if(tid == -1){
        perror("tlib create");
        free(thread_stack);
        return errno;
    }
    *t = tid;
    return 0;
}

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
    while(!handledSignal){};
    pid_t pid = getpid();
    ret = syscall(TGKILL, pid, tid, signum);
    if(ret == -1){
        perror("tgkill");
        return errno;
    }
    return ret;
}

int thread_join(thread t, void **retLocation){
    int status;
    void *addr = (void *)returnCustomTidAddress(&tidList, t);

    if(addr == NULL){
        return ESRCH;
    }
    if(*((pid_t*)addr) == 0){
        // printf("Thread already exited %d\n", EINVAL);
        singlyLLDelete(&tidList, t);
        return EINVAL;
    }
    
    int ret;
    // log_error("%d %d",*(pid_t *)(addr),t);
    while(*((pid_t*)addr) == t){
        ret = syscall(SYS_futex ,addr, FUTEX_WAIT, t, NULL, NULL, 0);
        // printf("Futex retval 1 %d\n", ret);
    }
    syscall(SYS_futex , addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    singlyLLDelete(&tidList, t);
    
    // printf("Futex retval %d\n", ret);
    if(retLocation) *retLocation = (void *)&ret;
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

//Handles ManyMany thread creation
int createManyMany(thread *t, void *attr,void * routine, void *arg){
    /*ManyMany Code*/
    return 0;
}
