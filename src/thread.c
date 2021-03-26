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

#define TGKILL 234

singlyLL tidList;
int handledSignal = 0;

static void init(){
    printf("Library initialised\n");
    //Initialise necessay data structures
    singlyLLInit(&tidList);
    singlyLLInsert(&tidList, getpid());
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
int create(thread *t,void *attr,void * routine,void *arg, int threadMode){
    switch(threadMode){
        case 0:
            return createOneOne(t, attr,routine, arg);
        case 1:
            return createManyOne(t, attr,routine, arg);
        case 2:
            return createManyMany(t, attr,routine, arg);
    }
}



void* allocStack(size_t size, size_t guard){
    
    void *stack = NULL;  
    //Align the memory to a 64 bit compatible page size and associate a guard area for the stack 
    if(posix_memalign(&stack,GUARD_SZ,size + guard) || mprotect(stack,guard, PROT_NONE)){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

typedef struct funcargs{
    void (*f)(void *);
    void *arg;
} funcargs;

void handlesegf(){
    printf("Thread Seg faulted\n");
}
int wrap(void *fa){
    printf("Signals handled\n");
    signal(SIGSEGV,handlesegf);
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
//
int createOneOne(thread *t,void *attr,void * routine, void *arg){
    mut_t lock;
    spin_init(&lock);
    spin_acquire(&lock);
    static int initState = 0;
    tcb *thread_t = (tcb *)malloc(sizeof(tcb));
    if(!thread_t){
        perror("");
        return errno;
    }
    thread tid;
    void *thread_stack;
    int status;
    if(initState == 0){
        initState = 1;
        init();
    }
    singlyLLInsert(&tidList, 0);
    if(attr){
        thread_attr *attr_t = (thread_attr *)attr;
        thread_stack = attr_t->stack == NULL ? allocStack(attr_t->stackSize, attr_t->guardSize) : attr_t->stack ;
        if(!thread_stack) {
            perror("tlib create");
            return errno;
        }
        funcargs fa;
        fa.f = routine;
        fa.arg = arg;
        void * addr = returnTailTidAddress(&tidList);
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
        funcargs fa;
        fa.f = (void (*)(void *))routine;
        fa.arg = arg;
        void * addr = returnTailTidAddress(&tidList);
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
    if(signum == SIGINT || signum == SIGCONT || signum == SIGTSTP){
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
    printf("Kill called %d %d\n",pid,tid);
    // int ret = 0;
    ret = syscall(TGKILL, pid, tid, signum);
    if(ret == -1){
        perror("tgkill");
        return errno;
    }
    return ret;
}

int thread_join(thread t, void **retLocation){
    int status;
    #ifndef DEV
        printf("Futex waiting for thread %ld\n", t);
        fflush(stdout);
    #endif
    void *addr = returnCustomTidAddress(&tidList, t);
    if(addr == NULL){
        return ESRCH;
    }
    if(*((pid_t*)addr) == 0){
        return EINVAL;
    }
    while(*((pid_t*)addr) == t){
        int ret = syscall(SYS_futex , addr, FUTEX_WAIT, t, NULL, NULL, 0);
    }
    //By default, clone wakes up only one futex, so need a way to wake up multiple threads
    syscall(SYS_futex , addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    #ifndef DEV
        printf("Futex done with thread %ld\n", t);
        fflush(stdout);
    #endif
    singlyLLDelete(&tidList, t);
    return 0;
}


void thread_exit(void *ret){
    void *addr = returnCustomTidAddress(&tidList, gettid());
    syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    singlyLLDelete(&tidList, gettid());
    // _exit(0);
    kill(SIGINT,gettid());
    return;
}

//Handles ManyOne thread creation
int createManyOne(thread *t, void *attr,void * routine, void *arg){
    /*ManyOne Code*/
    return 0;
}

//Handles ManyMany thread creation
int createManyMany(thread *t, void *attr,void * routine, void *arg){
    /*ManyMany Code*/
    return 0;
}
