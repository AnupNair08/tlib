#include<stdlib.h>
#include<unistd.h>
#include<ucontext.h>
#define STACK_SZ 65536
#define GUARD_SZ getpagesize()
#define TGKILL 234
#define CLONE_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD |CLONE_SYSVSEM|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID
#define SCHED_INTERVAL 2000

typedef unsigned long thread;
typedef struct funcargs{
    void (*f)(void *);
    void *arg;
} funcargs;

int createManyOne(thread *, void *, void *, void *);

enum thread_state {RUNNING, WAITING, EXITED, RUNNABLE};

typedef struct tcb {
    thread tid;
    void *stack;
    size_t stack_sz;
    int thread_state;
    ucontext_t* context;
    //indicate if the process has exited or not
    int exited;
    //list of all waiters on this process
    int* waiters;
    int numWaiters;
} tcb;