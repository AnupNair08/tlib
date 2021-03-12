#define _GNU_SOURCE
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


// int thread_attr_init(thread_attr *t){
//     thread_attr th_init = default_values;
//     t = &th_init;
//     return 0;
// }


// int thread_attr_destroy(thread_attr *t){
//     t = NULL;
//     return 0;
// }


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


typedef struct threadStack {
    thread tid;
    char *stack;
    long size;
} threadStack;

threadStack ts[2];
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
    char *stack;
    
    if(attr){
        thread_attr *t = (thread_attr *)attr;
        stack = (char *)malloc(t->stackSize);
        tid = clone(routine,stack + t->stackSize, CLONE_FLAGS,arg,NULL);
    }
    else{
        stack = (char *)malloc(STACK_SZ);
        tid = clone(routine,stack + STACK_SZ, CLONE_FLAGS,arg,NULL);
    }

    
    if(tid == -1){
        perror("tlib create");
        free(stack);
        return errno;
    }
    int status;
    // free(stack);
    if(initState == 0){
        initState = 1;
        init();
    }
    ts[ts->size++].tid = tid;
    ts[ts->size++].stack = stack;
    return tid;
}

int thread_join(thread t, void **retLocation){
    int status;
    pid_t tid = waitpid(t,&status,WUNTRACED);
    if(tid == -1){
        perror("tlib join");
        return errno;
    }
    for(int i = 0 ; i < 2;i++){
        if(ts[i].tid == t){
            free(ts[i].stack);
            break;
        }
    }
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
