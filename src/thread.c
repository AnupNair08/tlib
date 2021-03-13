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
#include "thread.h"
#include "tlibtypes.h"
#include "attributetypes.h"

static void init(){
    puts("Library initialised");
    //Initialise necessay data structures
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
thread create(thread *t,void *attr,void * routine,void *arg, int threadMode){
    switch(threadMode){
        case 0:
            return createOneOne(t, attr,routine, arg);
        case 1:
            return createManyOne(t, attr,routine, arg);
        case 2:
            return createManyMany(t, attr,routine, arg);
    }
}

#ifndef DEV
typedef struct threadStack {
    thread tid;
    char *stack;
    long size;
} threadStack;

threadStack ts[2];
#endif


static void* allocStack(size_t size){
    void *stack = malloc(size);  
    if(posix_memalign(&stack,8,size) != 0){
        perror("");
        return NULL;
    }
    return stack + size;
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
thread createOneOne(thread *t,void *attr,void * routine, void *arg){
    static int initState = 0;
    thread tid;
    void *thread_stack;
    
    if(attr){
        thread_attr *t = (thread_attr *)attr;
        thread_stack = allocStack(t->stackSize);
        tid = clone(routine,thread_stack, CLONE_FLAGS,arg,NULL);
    }
    else{
        thread_stack = allocStack(STACK_SZ);
        tid = clone(routine,thread_stack, CLONE_FLAGS,arg,NULL);
    }

    
    if(tid == -1){
        perror("tlib create");
        free(thread_stack);
        return errno;
    }
    int status;
    // free(stack);
    if(initState == 0){
        initState = 1;
        init();
    }
    #ifndef DEV
        ts[ts->size++].tid = tid;
        ts[ts->size++].stack = stack;
    #endif // !DEV
    return tid;
}

int thread_join(thread t, void **retLocation){
    int status;
    pid_t tid = waitpid(t,&status,WUNTRACED);
    if(tid == -1){
        perror("tlib join");
        return errno;
    }
    #ifndef DEV
    for(int i = 0 ; i < 2;i++){
        if(ts[i].tid == t){
            free(ts[i].stack);
            break;
        }
    }
    #endif // !DEV
    return 0;
}


//Handles ManyOne thread creation
thread createManyOne(thread *t, void *attr,void * routine, void *arg){
    /*ManyOne Code*/
    return 0;
}

//Handles ManyMany thread creation
thread createManyMany(thread *t, void *attr,void * routine, void *arg){
    /*ManyMany Code*/
    return 0;
}
