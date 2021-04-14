#include <stdio.h>
#include <unistd.h>
#include "tests.h"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "../attributetypes.h"
#include "../log.h"
#include "../thread.h"

void func(void *i)
{
    printf("In thread %d\n", *(int *)i);
    fflush(stdout);
    // sleep(*(int *)i);
    while (1)
    {
    }
    return;
}
void handleGlobal(int signum)
{
    printf("Global signal caught in main thread\n");
    return;
}

void routine()
{
    // sleep(4);
    thread_exit(NULL);
}

int main()
{
    // signal(SIGINT, handleGlobal);
    int j[4] = {1, 2, 3, 4};
    thread t[4];
    for (int i = 0; i < 4; i++)
    {
        thread_create(&t[i], NULL, func, (void *)&j[i]);
        log_trace("Thread %ld created", t[i]);
    }
    log_trace("Sending signal to thread %ld", t[2]);
    thread_kill(t[2], SIGTERM);
    log_trace("Sending signal to thread %ld", t[3]);
    thread_kill(t[3], SIGTERM);
    for (int i = 0; i < 4; i++)
    {
        thread_join(t[i], NULL);
    }
    // log_trace("Sending process wide signal");
    // thread_kill(getpid(),SIGINT);
    // thread test,t;
    // thread_create(&test,NULL,routine,NULL);
    // // thread_join(t,NULL);
    // thread_join(test,NULL);
    // log_info("Main exiting");
    return 0;
}
