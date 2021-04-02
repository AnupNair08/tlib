#define _GNU_SOURCE
#include<stdatomic.h>
#include<unistd.h>
typedef unsigned long int thread;
typedef volatile atomic_flag mut_t;
int thread_create(thread *, void *, void *, void *);
int thread_join(thread , void **);