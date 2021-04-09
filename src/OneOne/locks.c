#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include <stdlib.h>
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
    asm (
       "movl $0x0,8(%rdi);"
    );
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
    asm(
        "whileloop:"
        "mov    $1, %eax;"
        "xchg   %al, (%rdi);"
        "test %al,%al;"
        "jne whileloop;"
    );
    return 0;
}

/**
 * @brief Release lock atomically
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_release(spin_t *lock){
    asm(
        "movl $0x0,(%rdi);"
    );
    return 0;
}

/**
 * @brief Initialize the mutex lock object 
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_init(mutex_t *lock){
    asm(
        "movl $0x0,(%rdi);"
    );  
    return 0;
}


/**
 * @brief Atomically acquire the lock and wait by sleeping if not available
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_acquire(mutex_t *lock){
    asm(
        "mutexloop:"
        "mov    $1, %eax;"
        "xchg   %al, (%rdi);"
        "test %al,%al;" 
        "je endlabel"
    );  
    syscall(SYS_futex , lock, FUTEX_WAIT, 1, NULL, NULL, 0);
    asm(
        "jmp mutexloop"
    );
    asm(
        "endlabel:"
    );
    return 0;
}


/**
 * @brief Release the lock object atomically and wake up waiting threads
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_release(mutex_t *lock){
    asm(
        "movl $0x0,(%rdi);"
    );
    syscall(SYS_futex , lock, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}
