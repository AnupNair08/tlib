#include "tlib.h"

extern tcb* __curproc;
extern tcb* __scheduler;
extern tcbQueue __allThreads;
extern mut_t globallock;

int spin_init(spin_t* lock){
    printf("Lock init\n");
    *lock = (volatile spin_t)ATOMIC_FLAG_INIT;
    return 0;
}
int spin_acquire(spin_t *lock){
    while(atomic_flag_test_and_set(lock)){
        // log_trace("Waiting for lock");
    };
    return 0;
}

int spin_release(spin_t *lock){
    atomic_flag_clear(lock);
    return 0;
}

atomic_int unlocked;
atomic_int locked;

int mutex_init(mut_t *lock){
    atomic_init(lock,0);
    atomic_init(&unlocked,0);
    atomic_init(&locked,1);
    return 0;
}

int mutex_acquire(mut_t *lock){
    disabletimer();
    if(!atomic_compare_exchange_strong(lock,&unlocked,1)){
        // log_trace("Waiting for lock");
        __curproc->mutexWait = lock;
        __curproc->thread_state = WAITING;
        switchToScheduler();
    }
    enabletimer();
}

int mutex_release(mut_t *lock){
    disabletimer();
    atomic_compare_exchange_strong(lock,&locked,0);
    unlockMutex(&__allThreads, lock);
    enabletimer();
    // log_trace("Lock released");
}
