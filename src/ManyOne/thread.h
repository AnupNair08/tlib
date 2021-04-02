#define _GNU_SOURCE
#include<stdatomic.h>
#include<unistd.h>
typedef unsigned long int thread;
typedef volatile atomic_flag mut_t;
static void* allocStack(size_t , size_t );
void starttimer();
void enabletimer();
void disabletimer();
void switchToScheduler();
void scheduler();
void initManyOne();
void wrapRoutine(void *);
int thread_create(thread *, void *, void *, void *);
int thread_join(thread , void **);