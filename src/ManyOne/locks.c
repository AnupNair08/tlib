#include "tlib.h"

extern tcb* __curproc;
extern tcb* __scheduler;
extern tcbQueue __allThreads;
extern mut_t globallock;

int spin_init(mut_t* lock){
    printf("Lock init\n");
    *lock = (volatile mut_t)ATOMIC_FLAG_INIT;
}
int spin_acquire(mut_t *lock){
    while(atomic_flag_test_and_set(lock)){
        // log_trace("Waiting for lock");
    };
}

int spin_release(mut_t *lock){
    atomic_flag_clear(lock);
}


int mutex_init(mut_t *lock){
    *lock = (volatile mut_t)ATOMIC_FLAG_INIT;
}

int mutex_acquire(mut_t *lock){
    disabletimer();
    int lockstatus = atomic_flag_test_and_set(lock);
    if(lockstatus != 0){
        // log_trace("Waiting for lock");
        __curproc->mutexWait = lock;
        __curproc->thread_state = WAITING;
        switchToScheduler();
    }
    enabletimer();
}

int mutex_release(mut_t *lock){
    disabletimer();
    unlockMutex(&__allThreads, lock);
    atomic_flag_clear(lock);
    enabletimer();
    // log_trace("Lock released");
}
