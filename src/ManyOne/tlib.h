#include<stdlib.h>
#include<unistd.h>
#include<setjmp.h>
#include "locks.h"

typedef unsigned long thread;
typedef struct funcargs{
    void (*f)(void *);
    void* arg;
} funcargs;

int createManyOne(thread *, void *, void *, void *);

enum thread_state {RUNNING, WAITING, EXITED, RUNNABLE};

typedef struct tcb {
    thread tid;
    void *stack;
    size_t stack_sz;
    int thread_state;
    sigjmp_buf *ctx;
    //indicate if the process has exited or not
    int exited;
    //list of all waiters on this process
    int* waiters;
    int numWaiters;
    mut_t* mutexWait;
    // Implement as a queue for easy deletion
    int* pendingSig;
    int numPendingSig;
    funcargs* args;
} tcb;

typedef struct qnode {
    tcb *tcbnode;
    struct qnode *next;
} qnode;

typedef struct tcbQueue {
    qnode *front;
    qnode *back;
    int len;
} tcbQueue;

#define STACK_SZ 65536
#define GUARD_SZ getpagesize()
#define TGKILL 234
#define CLONE_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD |CLONE_SYSVSEM|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID
#define SCHED_INTERVAL 2000

void wrapRoutine();
static void* allocStack(size_t , size_t );
static void starttimer();
void enabletimer();
void disabletimer();
void switchToScheduler();
static void scheduler();
static void initManyOne();

int addThread(tcbQueue *, tcb *);
tcb* getNextThread(tcbQueue *);
tcb* getThread(tcbQueue *, thread);
void printAllmo(tcbQueue *);
void queueRunning(tcbQueue *);
int removeThread(tcbQueue *, unsigned long int);
void reQueue(tcbQueue *, tcb *);
void unlockMutex(tcbQueue *, mut_t*);
void initTcb(tcb *, int, thread,sigjmp_buf *);