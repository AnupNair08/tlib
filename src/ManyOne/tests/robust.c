#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
#include <tlib.h>
#else
#include "../thread.h"
#endif
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include "tests.h"

mutex_t lock;
spin_t splock;

void func()
{
    printf("Dummy routine\n");
    thread_exit(NULL);
    return;
}
void routine1()
{
    mutex_acquire(&lock);
    printf("[Thread 1] Acquiring a lock and waiting\n");
    sleep(1);
    mutex_release(&lock);
}

void routine2()
{
    printf("[Thread 2] Trying to release lock acquired by thread 1\n");
    int ret = mutex_release(&lock);
    if (ret == ENOTRECOVERABLE)
    {
        TESTPASS
    }
    else
    {
        TESTFAIL
    }
}
void rout1()
{
    printf("In t1\n");
    spin_acquire(&splock);
    if (spin_trylock(&splock) == EBUSY)
    {
        printf("Lock is busy\n");
    }
    spin_release(&splock);
}
void rout2()
{
    printf("In t2\n");
    if (spin_trylock(&splock) == EBUSY)
    {
        printf("Lock is busy\n");
    }
    else
    {
        spin_acquire(&splock);
        printf("Critcal Section\n");
    }
    spin_release(&splock);
}

int main()
{
    // -------------------------------------------------------------------------------------------------------------------------
    thread t;
    printf("Creating threads with invalid arguments\n");
    if (thread_create(NULL, NULL, NULL, NULL) == EINVAL && thread_create(&t, NULL, NULL, NULL) == EINVAL && thread_create(NULL, NULL, func, NULL) == EINVAL)
    {
        TESTPASS
    }
    else
    {
        TESTFAIL
    }

    // -------------------------------------------------------------------------------------------------------------------------
    printf("Joining thread in invalid cases\n");
    thread_create(&t, NULL, func, NULL);
    // Already joined thread
    thread_join(t, NULL);
    // Rejoin or give random TID
    if (thread_join(t, NULL) == ESRCH && thread_join(231, NULL) == ESRCH)
    {
        TESTPASS
    }
    else
    {
        TESTFAIL
    }

    // -------------------------------------------------------------------------------------------------------------------------
    printf("Sending invalid signal\n");
    if (thread_kill(t, 0) == -1)
        TESTPASS
    else
        TESTFAIL

    // -------------------------------------------------------------------------------------------------------------------------
    printf("Releasing lock without acquiring\n");
    thread t1, t2;
    mutex_init(&lock);
    thread_create(&t1, NULL, routine1, NULL);
    thread_create(&t2, NULL, routine2, NULL);
    thread_join(t1, NULL);
    thread_join(t2, NULL);

    // -------------------------------------------------------------------------------------------------------------------------
    printf("Testing trylock\n");
    thread t3, t4;
    spin_init(&splock);
    thread_create(&t3, NULL, rout1, NULL);
    thread_create(&t4, NULL, rout2, NULL);
    thread_join(t3, NULL);
    return 0;
}