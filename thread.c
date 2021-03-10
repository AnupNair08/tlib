#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include "thread.h"

//Umbrella function which calls specific thread function
int create(thread *t, void * routine,void *attr,void *arg, int threadMode){
    switch(threadMode){
        case 0:
            createOneOne(t, routine, attr, arg);
        case 1:
            createManyOne(t, routine, attr, arg);
        case 2:
            createManyMany(t, routine, attr, arg);
    }
}

//Handles OneOne thread creation
int createOneOne(thread *t, void * routine,void *attr, void *arg){
    char *stack = (char *)malloc(1024);
    pid_t tid = clone(routine,stack + 1024,CLONE_FILES | CLONE_FS,arg,NULL);
    if(tid == -1){
        perror("");
        return errno;
    }
    return 0;
}

//Handles ManyOne thread creation
int createManyOne(thread *t, void * routine,void *attr, void *arg){
    /*ManyOne Code*/
    return 0;
}

//Handles ManyMany thread creation
int createManyMany(thread *t, void * routine,void *attr, void *arg){
    /*ManyMany Code*/
    return 0;
}
