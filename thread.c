#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include "thread.h"


int create(thread *t, void * routine,void *attr,void *arg){
    char *stack = (char *)malloc(1024);
    pid_t tid = clone(routine,stack + 1024,CLONE_FILES | CLONE_FS,arg,NULL);
    if(tid == -1){
        perror("");
        return errno;
    }
    return 0;
}