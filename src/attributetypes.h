#include<stdlib.h>

typedef struct sched_params {
    // Will be needed for storing scheduling info in many one model;
} sched_params;

typedef struct thread_attr {
    int detachState;
    size_t guardSize;
    int inheritSched;
    sched_params *schedParams;
    int schedPolicy;
    int scope;
    void *stack;
    size_t stackSize;
} thread_attr;


int thread_attr_init(thread_attr *);
int thread_attr_destroy(thread_attr *);

size_t thread_attr_getStack(thread_attr *);
int thread_attr_setStack(thread_attr *,size_t);
