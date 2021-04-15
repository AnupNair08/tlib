#include "tlib.h"
#include "tattr.h"

static thread_attr __default = {NULL,STACK_SZ};

int thread_attr_init(thread_attr *t){
    t->stackSize = __default.stackSize;
    t->stack = __default.stack;
    t->schedInterval = (schedParams){ .sc = SCHED_SC, .ms = SCHED_MS};
    return 0;
}


int thread_attr_destroy(thread_attr *t){
    if(!t) return -1;
    t = NULL;
    return 0;
}

int thread_attr_setStack(thread_attr *t, size_t size) {
    if(!t) return -1;
    t->stackSize = size;
    return 0;
}
size_t thread_attr_getStack(thread_attr *t){
    if(!t) return -1;
    return t->stackSize;
}

int thread_attr_setStackAddr(thread_attr *t,void *stack,size_t size){
    if(!t) return -1;
    t->stack = stack;
    t->stackSize = size;
    return 0;
}

int thread_attr_setSchedInterval(thread_attr *t, schedParams interval){
    if(!t) return -1;
    t->schedInterval.sc = interval.sc;
    t->schedInterval.ms = interval.ms;
    return 0;
}

schedParams* thread_attr_getSchedInterval(thread_attr *t){
    if(!t) return NULL;
    return &(t->schedInterval);
}