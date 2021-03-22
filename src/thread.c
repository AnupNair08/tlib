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
#include "thread.h"
#include "tlibtypes.h"
#include "attributetypes.h"
#include "dataStructTypes.h"
#include<limits.h>
singlyLL tidList;

static void init(){
    printf("Library initialised\n");
    //Initialise necessay data structures
    singlyLLInit(&tidList);
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


/**
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
        thread_stack = allocStack(((thread_attr *)attr)->stackSize, ((thread_attr *)attr)->guardSize);
        if(!thread_stack) {
            perror("tlib create");
            return errno;
        }
        void * addr = returnTailTidAddress(&tidList);
        tid = clone(routine,
                    thread_stack + ((thread_attr *)attr)->stackSize + ((thread_attr *)attr)->guardSize, 
                    CLONE_FLAGS,
                    arg,
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
        tid = clone(routine,
                    thread_stack + STACK_SZ + GUARD_SZ,
                    CLONE_FLAGS,
                    arg,
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
    thread_t->tid = tid;
    thread_t->stack = thread_stack;
    thread_t->stack_sz = attr == NULL ? STACK_SZ : ((thread_attr *)attr)->stackSize;
    return 0;
}

int thread_join(thread t, void **retLocation){
    int status;
    #ifndef DEV
        printf("Futex waiting for thread %ld\n", t);
        fflush(stdout);
    #endif
    void *addr = returnCustomTidAddress(&tidList, t);
    int ret = syscall(SYS_futex , addr, FUTEX_WAIT, t, NULL, NULL, 0);
    // printf("%d\n",ret);
    // perror("");
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
