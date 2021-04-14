#include "tlib.h"

extern tcb *__curproc;
extern tcb *__scheduler;
extern tcbQueue __allThreads;
extern mut_t globallock;

/**
 * @brief Initialize the spinlock object
 * 
 * @param lock Spinlock object
 * @return int 
 */
int spin_init(spin_t *lock)
{
    // log_debug("Lock initialzed");
    asm(
        "movl $0x0,8(%rdi);");
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
    asm(
        "whileloop:"
        "mov    $1, %eax;"
        "xchg   %al, (%rdi);"
        "test %al,%al;"
        "jne whileloop;");
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
    asm(
        "movl $0x0,(%rdi);");
    return 0;
}

int mutex_init(mut_t *lock)
{
    atomic_init(lock, 0);
    asm(
        "movl $0x0,(%rdi);");
    return 0;
}

int mutex_acquire(mut_t *lock)
{
    asm(
        "mov    $1, %eax;"
        "xchg   %al, (%rdi);"
        "test %al,%al;"
        "je endlabel");
    disabletimer();
    __curproc->mutexWait = lock;
    __curproc->thread_state = WAITING;
    switchToScheduler();
    asm(
        "endlabel:");
    enabletimer();
}

int mutex_release(mut_t *lock)
{
    asm(
        "movl $0x0,(%rdi);");
    disabletimer();
    unlockMutex(&__allThreads, lock);
    enabletimer();
    // log_trace("Lock released");
}