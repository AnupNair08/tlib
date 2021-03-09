#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<wait.h>
#include "thread.h"
#include"tlibtypes.h"


int create(thread *t, void * routine,void *attr,void *arg){
    char *stack = (char *)malloc(STACK_SZ);
    pid_t tid = clone(routine,stack + 1024, CLONE_FLAGS,arg,NULL);
    if(tid == -1){
        perror("Error:");
        free(stack);
        return errno;
    }
    wait(NULL);
    free(stack);
    return tid;
}