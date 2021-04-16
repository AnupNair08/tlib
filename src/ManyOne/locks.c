#include "utils.h"

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
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lock));
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
    asm(
        "whileloop:"
        "xchg   %%al, (%1);"
        "test   %%al,%%al;"
        "jne whileloop;"
        : "=r"(outval)
        : "r"(lock));
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
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lock));
    return 0;
}

/**
 * @brief Initialize the mutex lock object 
 * 
 * @param lock Mutex Lock object
 * @return int 
 */
int mutex_init(mutex_t *lock)
{
    int outval;
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lock));
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
    asm(
        "xchg   %%al,(%1);"
        "test   %%al, %%al;"
        "je endlabel;"
        : "=r"(outval)
        : "r"(lock)
        : "%ebx");
    disabletimer();
    __curproc->mutexWait = lock;
    __curproc->thread_state = WAITING;
    switchToScheduler();
    asm(
        "endlabel:");
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
    int outval;
    asm(
        "movl $0x0,(%1);"
        : "=r"(outval)
        : "r"(lock));
    unlockMutex(&__allThreads, lock);
    enabletimer();
}