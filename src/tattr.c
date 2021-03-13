#include "thread.h"
#include "tlibtypes.h"
#include "attributetypes.h"

static thread_attr _default = {0,0,0,NULL,0,0,NULL,65536};


int thread_attr_init(thread_attr *t){
    t->stackSize = _default.stackSize;
    t->detachState = _default.detachState;
    t->guardSize = _default.guardSize;
    t->inheritSched = _default.inheritSched;
    t->schedParams = _default.schedParams;
    t->scope = _default.scope;
    t->stack = _default.stack;
    t->schedPolicy = _default.schedPolicy;
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