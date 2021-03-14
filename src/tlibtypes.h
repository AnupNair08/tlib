#include<stdlib.h>
#include<unistd.h>
#define STACK_SZ 65536
#define GUARD_SZ getpagesize()

#ifdef OLD_FLAGS
    #define CLONE_FLAGS CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM | SIGCHLD
#endif
#define CLONE_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID

enum thread_state {RUNNING, JOINWAIT, EXITED};

typedef struct tcb {
    thread tid;
    void *stack;
    size_t stack_sz;
    int thread_state;
} tcb;