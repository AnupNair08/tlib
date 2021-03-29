#include<stdlib.h>

typedef struct sched_params {
    // Will be needed for storing scheduling info in many one model;
} sched_params;

typedef struct thread_attr {
    void *stack;
    size_t guardSize;
    size_t stackSize;
    // int detachState;
    // int inheritSched;
    // sched_params *schedParams;
    // int schedPolicy;
    // int scope;
} thread_attr;


int thread_attr_init(thread_attr *);
int thread_attr_destroy(thread_attr *);

size_t thread_attr_getStack(thread_attr *);
int thread_attr_setStack(thread_attr *,size_t);

size_t thread_attr_getGuard(thread_attr *);
int thread_attr_setGuard(thread_attr *,size_t);

int thread_attr_setStackAddr(thread_attr *, void *,size_t);
