#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include "locks.h"
#include "log.h"

/**
 * @brief Initialize the spinlock object
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_init(spin_t* lock){
    // log_debug("Lock initialzed");
    *lock = (volatile spin_t)ATOMIC_FLAG_INIT;
    return 0;
}

/**
 * @brief Acquire a lock and wait atomically for the lock object
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_acquire(spin_t *lock){
    // Atomically busy wait until the lock becomes available
    while(atomic_flag_test_and_set(lock)){
        // log_trace("Waiting for lock");
    };
    return 0;
}

/**
 * @brief Release lock atomically
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_release(spin_t *lock){
    atomic_flag_clear(lock);
    return 0;
}

atomic_int unlocked;
atomic_int locked;

/**
 * @brief Initialize the mutex lock object 
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_init(mutex_t *lock){
    atomic_init(lock,0);
    atomic_init(&unlocked,0);
    atomic_init(&locked,1);
    return 0;
}


/**
 * @brief Atomically acquire the lock and wait by sleeping if not available
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_acquire(mutex_t *lock){
    while(!atomic_compare_exchange_strong(lock,&unlocked,1)){
        syscall(SYS_futex , lock, FUTEX_WAIT, 1, NULL, NULL, 0);
    }
    return 0;
}


/**
 * @brief Release the lock object atomically and wake up waiting threads
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_release(mutex_t *lock){
    atomic_compare_exchange_strong(lock,&locked,0);
    syscall(SYS_futex , lock, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}
