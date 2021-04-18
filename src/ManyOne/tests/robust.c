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

void routine1()
{
    mutex_acquire(&lock);
}

void routine2()
{
    int ret = mutex_release(&lock);
    if (ret == ENOTRECOVERABLE)
    {
        printf("Test passed");
    }
}

int main()
{
    thread t1, t2;
    mutex_init(&lock);
    thread_create(&t1, NULL, routine1, NULL);
    thread_create(&t2, NULL, routine2, NULL);
    thread_join(t1, NULL);
    thread_join(t2, NULL);
    return 0;
}