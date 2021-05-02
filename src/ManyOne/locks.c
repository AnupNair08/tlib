/**
 * @file locks.c
 * @author Hrishikesh Athalye & Anup Nair
 * @brief Implementation of synchronization primitives
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "utils.h"
#include <errno.h>

/**
 * @brief TCB of current running process 
 */
extern tcb *__curproc;
/**
 * @brief TCB of scheduler 
 */
extern tcb *__scheduler;
/**
 * @brief Thread Queue 
 */
extern tcbQueue __allThreads;

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
    disabletimer();
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    lock->locker = 0;
    enabletimer();
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
    int outval;
    volatile int *lockvar = &(lock->lock);
    asm(
        "whileloop:"
        "xchg   %%al, (%1);"
        "test   %%al,%%al;"
        "jne whileloop;"
        : "=r"(outval)
        : "r"(lockvar));
    disabletimer();
    lock->locker = __curproc->tid;
    enabletimer();
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
    disabletimer();
    int outval;
    if (lock->locker != __curproc->tid)
    {
        enabletimer();
        return ENOTRECOVERABLE;
    }
    volatile int *lockvar = &(lock->lock);
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    lock->locker = 0;
    enabletimer();
    return 0;
}

int spin_trylock(spin_t *lock)
{
    disabletimer();
    if (lock->locker)
    {
        enabletimer();
        return EBUSY;
    }
    else
    {
        enabletimer();
        return 0;
    }
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
    disabletimer();
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    lock->locker = 0;
    enabletimer();
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
    disabletimer();
    volatile int *lockvar = &(lock->lock);
    asm(
        "xchg   %%al,(%1);"
        "test   %%al, %%al;"
        "je endlabel;"
        : "=r"(outval)
        : "r"(lockvar)
        : "%ebx");
    __curproc->mutexWait = lock;
    __curproc->thread_state = WAITING;
    switchToScheduler();
    asm(
        "endlabel:");
    lock->locker = __curproc->tid;
    enabletimer();
}

/**
 * @brief Release the lock object atomically and wake up waiting threads
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_release(mutex_t *lock)
{
    disabletimer();
    if (lock->locker != __curproc->tid)
    {
        enabletimer();
        return ENOTRECOVERABLE;
    }
    volatile int *lockvar = &(lock->lock);
    int outval;
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lockvar));
    unlockMutex(&__allThreads, lock);
    lock->locker = 0;
    enabletimer();
}

int mutex_trylock(mutex_t *lock)
{
    disabletimer();
    if (lock->locker)
    {
        enabletimer();
        return EBUSY;
    }
    else
    {
        enabletimer();
        return 0;
    }
}