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
    usleep(500000);
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
int i = 0;
void rout1()
{
    spin_acquire(&splock);
    usleep(500000);
    spin_release(&splock);
}
void rout2()
{
    while (spin_trylock(&splock) == EBUSY)
    {
        i++;
    }
}

void rout3()
{
    while (spin_acquire(&splock) != 0)
    {
        i++;
    };
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
    if (thread_join(t, NULL) == EINVAL && thread_join(231, NULL) == ESRCH)
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
    spin_init(&splock);
    thread t3, t4, t5, t6;
    thread_create(&t3, NULL, rout1, NULL);
    thread_create(&t4, NULL, rout2, NULL);
    thread_join(t3, NULL);
    printf("With trylock value of i: %d\n", i);
    i = 0;
    thread_create(&t5, NULL, rout1, NULL);
    thread_create(&t6, NULL, rout3, NULL);
    thread_join(t5, NULL);
    printf("Without trylock value value of i: %d\n", i);

    // -------------------------------------------------------------------------------------------------------------------------

    return 0;
}