#include<stdlib.h>
#include<unistd.h>
#define STACK_SZ 65536
#define GUARD_SZ getpagesize()

#define TGKILL 234
#define CLONE_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD |CLONE_SYSVSEM|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID

typedef struct funcargs{
    void (*f)(void *);
    void *arg;
} funcargs;


enum thread_state {RUNNING, JOINWAIT, EXITED};

typedef struct tcb {
    thread tid;
    void *stack;
    size_t stack_sz;
    int thread_state;
} tcb;