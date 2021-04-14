#include "tlib.h"

extern tcb* __curproc;
extern tcb* __scheduler;
extern tcbQueue __allThreads;
extern mut_t globallock;

int spin_init(spin_t* lock){
    volatile int outval; 
    asm (
       "movl $0x0,(%1);"
       :"=r" (outval)
       :"r"( lock)
    );
    return 0;
}
int spin_acquire(spin_t *lock){
    int outval;
    asm(
        "whileloop:"
        "xchg   %%al, (%1);"
        "test   %%al,%%al;"
        "jne whileloop;"
        :"=r" (outval)
        :"r"(lock)
    );
    return 0;
}

int spin_release(spin_t *lock){
    int outval;
    asm(
        "movl $0x0,(%1);"
        : "=r" (outval)
        : "r" (lock)
        );
    return 0;
}

int mutex_init(mut_t *lock)
{
    int outval;
    asm(
        "movl $0x0,(%1);"
        : "=r" (outval)
        : "r" (lock)
        );
    return 0;
}

int mutex_acquire(mut_t *lock)
{
    volatile int outval ;
    asm (
        "xchg   %%al,(%1);"
        "test   %%al, %%al;"
        "je endlabel;"
         : "=r" ( outval )        
         : "r" ( lock )         
         : "%ebx"         
     );
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
    disabletimer();
    int outval;
    asm(
        "movl $0x0,(%1);"
        : "=r" (outval)
        : "r" (lock)
        );
    unlockMutex(&__allThreads, lock);
    enabletimer();
}