#include "locks.h"

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
    int lockstatus = atomic_flag_test_and_set(lock);
    if(lockstatus != 0){
        log_trace("Waiting for lock");
        while(lock!=0){
            syscall(SYS_futex , lock, FUTEX_WAIT, 1, NULL, NULL, 0);
        }
    }
    else{
        log_trace("Lock acquired");
    }
}

int mutex_release(mut_t *lock){
    atomic_flag_clear(lock);
    syscall(SYS_futex , lock, FUTEX_WAKE, 1, NULL, NULL, 0);
    log_trace("Lock released");
}
