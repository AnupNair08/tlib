#include <stdatomic.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include "thread.h"
#include "log.h"

/**
 * @brief Initialize the spinlock object
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_init(mut_t* lock){
    // log_debug("Lock initialzed");
    *lock = (volatile mut_t)ATOMIC_FLAG_INIT;
    return 0;
}

/**
 * @brief Acquire a lock and wait atomically for the lock object
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_acquire(mut_t *lock){
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
int spin_release(mut_t *lock){
    atomic_flag_clear(lock);
    return 0;
}


/**
 * @brief Initialize the mutex lock object 
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_init(mut_t *lock){
    *lock = (volatile mut_t)ATOMIC_FLAG_INIT;
    return 0;
}

/**
 * @brief Atomically acquire the lock and wait by sleeping if not available
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_acquire(mut_t *lock){
    // Try to acquire a lock
    int lockstatus = atomic_flag_test_and_set(lock);
    if(lockstatus != 0){
        // If lock is not available, the thread goes to sleep state
        while(lock!=0){
            syscall(SYS_futex , lock, FUTEX_WAIT, 1, NULL, NULL, 0);
        }
    }
    return 0;
}


/**
 * @brief Release the lock object atomically and wake up waiting threads
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_release(mut_t *lock){
    atomic_flag_clear(lock);
    syscall(SYS_futex , lock, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}
