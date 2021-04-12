#include "tlib.h"

extern tcb *__curproc;
extern tcb *__scheduler;
extern tcbQueue __allThreads;
extern mut_t globallock;

int spin_init(spin_t *lock)
{
    printf("Lock init\n");
    asm(
        "movl $0x0,(%rdi);");
    return 0;
}
int spin_acquire(spin_t *lock)
{
    asm(
        "whileloop:"
        "mov    $1, %eax;"
        "xchg   %al, (%rdi);"
        "test %al,%al;"
        "jne whileloop;");
    return 0;
}

int spin_release(spin_t *lock)
{
    asm(
        "movl $0x0,(%rdi);");
    return 0;
}

atomic_int unlocked;
atomic_int locked;

int mutex_init(mut_t *lock)
{
    // atomic_init(lock,0);
    asm(
        "movl $0x0,(%rdi);");
    // atomic_init(&unlocked,0);
    // atomic_init(&locked,1);
    return 0;
}

int mutex_acquire(mut_t *lock)
{
    disabletimer();
    asm(
        "movl   $1, %eax;"
        "xchg   %al, (%rdi);"
        "test   %al,%al;"
        "je endlabel");
    log_trace("Waiting for lock");
    __curproc->mutexWait = lock;
    __curproc->thread_state = WAITING;
    switchToScheduler();
    asm(
        "endlabel:");
    enabletimer();
}

int mutex_release(mut_t *lock)
{
    disabletimer();
    // atomic_compare_exchange_strong(lock,&locked,0);
    asm(
        "movl $0x0,(%rdi);");
    unlockMutex(&__allThreads, lock);
    enabletimer();
    // log_trace("Lock released");
}
