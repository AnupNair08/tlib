#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<wait.h>
#include "thread.h"
#include"tlibtypes.h"

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
thread create(thread *t, void * routine,void *attr,void *arg, int threadMode){
    switch(threadMode){
        case 0:
            return createOneOne(t, routine, attr, arg);
        case 1:
            return createManyOne(t, routine, attr, arg);
        case 2:
            return createManyMany(t, routine, attr, arg);
    }
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
thread createOneOne(thread *t, void * routine,void *attr, void *arg){
    char *stack = (char *)malloc(STACK_SZ);
    thread tid = clone(routine,stack + 1024, CLONE_FLAGS,arg,NULL);
    if(tid == -1){
        perror("Error:");
        free(stack);
        return errno;
    }
    wait(NULL);
    free(stack);
    return tid;
}

//Handles ManyOne thread creation
thread createManyOne(thread *t, void * routine,void *attr, void *arg){
    /*ManyOne Code*/
    return 0;
}

//Handles ManyMany thread creation
thread createManyMany(thread *t, void * routine,void *attr, void *arg){
    /*ManyMany Code*/
    return 0;
}
