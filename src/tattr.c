#include "thread.h"
#include "tlibtypes.h"
#include "attributetypes.h"
#include <stdlib.h>

static thread_attr __default = {0,0,0,NULL,0,0,NULL,STACK_SZ};

int thread_attr_init(thread_attr *t){
    t->stackSize = __default.stackSize;
    t->detachState = __default.detachState;
    t->guardSize = GUARD_SZ;
    t->inheritSched = __default.inheritSched;
    t->schedParams = __default.schedParams;
    t->scope = __default.scope;
    t->stack = __default.stack;
    t->schedPolicy = __default.schedPolicy;
    return 0;
}


int thread_attr_destroy(thread_attr *t){
    t = NULL;
    return 0;
}

int thread_attr_setStack(thread_attr *t, size_t size) {
    t->stackSize = size;
    return 0;
}
size_t thread_attr_getStack(thread_attr *t){
    return t->stackSize;
}

int thread_attr_setGuard(thread_attr *t, size_t size) {
    t->guardSize = size;
    return 0;
}
size_t thread_attr_getGuard(thread_attr *t){
    return t->guardSize;
}