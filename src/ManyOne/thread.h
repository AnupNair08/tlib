#define _GNU_SOURCE
#include<stdatomic.h>
#include<unistd.h>
#include "locks.h"
typedef unsigned long int thread;
int thread_create(thread *, void *, void *, void *);
int thread_join(thread , void **);
int thread_kill(pid_t, int);
int thread_exit(void *);