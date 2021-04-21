#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <errno.h>
#include "locks.h"
#define gettid() syscall(SYS_gettid)

/**
 * @brief Initialize the spinlock object
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_init(spin_t *lock)
{
    volatile int outval;
    volatile int *lockvar = &(lock->lock);
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    lock->locker = 0;
    return 0;
}

/**
 * @brief Acquire a lock and wait atomically for the lock object
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_acquire(spin_t *lock)
{
    // Atomically busy wait until the lock becomes available
    int outval;
    volatile int *lockvar = &(lock->lock);
    asm(
        "whileloop:"
        "xchg   %%al, (%1);"
        "test   %%al,%%al;"
        "jne whileloop;"
        : "=r"(outval)
        : "r"(lockvar));
    // lock->locker = gettid();
    return 0;
}

/**
 * @brief Release lock atomically
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_release(spin_t *lock)
{
    int outval;
    // if (lock->locker != gettid())
    // {
    //     return ENOTRECOVERABLE;
    // }
    volatile int *lockvar = &(lock->lock);
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    lock->locker = 0;
    return 0;
}

int spin_trylock(spin_t *lock)
{
    return lock->locker == 0 ? 0 : EBUSY;
}

/**
 * @brief Initialize the mutex lock object 
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_init(mutex_t *lock)
{
    volatile int *lockvar = &(lock->lock);
    int outval;
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    lock->locker = 0;
    return 0;
}

/**
 * @brief Atomically acquire the lock and wait by sleeping if not available
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_acquire(mutex_t *lock)
{
    volatile int outval;
    volatile int *lockvar = &(lock->lock);
    asm(
        "mutexloop:"
        "mov    $1, %%eax;"
        "xchg   %%al, (%%rdi);"
        "test %%al,%%al;"
        "je endlabel"
        : "=r"(outval)
        : "r"(lockvar));
    syscall(SYS_futex, lock, FUTEX_WAIT, 1, NULL, NULL, 0);
    asm(
        "jmp mutexloop");
    asm(
        "endlabel:");
    // lock->locker = gettid();
    return 0;
}

/**
 * @brief Release the lock object atomically and wake up waiting threads
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_release(mutex_t *lock)
{
    volatile int outval;
    // if (lock->locker != gettid())
    // {
    //     return ENOTRECOVERABLE;
    // }
    volatile int *lockvar = &(lock->lock);
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    lock->locker = 0;
    syscall(SYS_futex, lock, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}

int mutex_trylock(mutex_t *lock)
{
    return lock->locker == 0 ? 0 : EBUSY;
}
