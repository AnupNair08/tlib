/**
 * @file lockTests.c
 * @author Hrishikesh Athalye & Anup Nair 
 * @brief Tests for synchronization primitives
 * @version 0.1
 * @date 2021-05-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
#include <tlib.h>
#else
#include "../thread.h"
#endif
#include "tests.h"
#define WITH_LOCKS 1
#define WITHOUT_LOCKS 0

int spinTest = 0;
short refstring[4];

mutex_t sleeplock;
spin_t spinlock;
long c1, c2, c, run = 1;

void *f1(void *lock)
{
    while (run)
    {
        c1++;
        mutex_acquire((mutex_t *)lock);
        c++;
        mutex_release((mutex_t *)lock);
    }
}
void *f2(void *lock)
{
    while (run)
    {
        c2++;
        mutex_acquire((mutex_t *)lock);
        c++;
        mutex_release((mutex_t *)lock);
    }
}

int testMutex()
{
    printf("Starting test with Mutex\n");
    mutex_init(&sleeplock);
    thread t1, t2;
    thread_create(&t1, NULL, f1, (void *)&sleeplock);
    thread_create(&t2, NULL, f2, (void *)&sleeplock);
    while (c < 10)
    {
    }
    run = 0;
    thread_join(t1, NULL);
    thread_join(t2, NULL);
    printf("\nValues after test are (c1 + c2)=%ld c=%ld\n", c1 + c2, c);
    if (c1 + c2 - c != 0)
        printf(RED "Test failed\n" RESET);
    else
        printf(GREEN "Test passed\n" RESET);
    return 0;
}
void *f1spin(void *lock)
{
    while (run)
    {
        c1++;
        spin_acquire((spin_t *)lock);
        c++;
        spin_release((spin_t *)lock);
    }
}
void *f2spin(void *lock)
{
    while (run)
    {
        c2++;
        spin_acquire((spin_t *)lock);
        c++;
        spin_release((spin_t *)lock);
    }
}

int testSpin()
{
    c1 = 0;
    c2 = 0;
    run = 1;
    c = 0;
    printf("Starting test with Spinlocks\n");
    spin_init(&spinlock);
    thread t1, t2;
    thread_create(&t1, NULL, f1spin, (void *)&spinlock);
    thread_create(&t2, NULL, f2spin, (void *)&spinlock);
    while (c < 10)
    {
    }
    run = 0;
    thread_join(t1, NULL);
    thread_join(t2, NULL);
    printf("\nValues after test are (c1 + c2)=%ld c=%ld\n", c1 + c2, c);
    if (c1 + c2 - c != 0)
        printf(RED "Test failed\n" RESET);
    else
        printf(GREEN "Test passed\n" RESET);
    return 0;
}

int main()
{
    printf("tlib Synchronization Tests\n");
    testMutex();
    testSpin();
    return 0;
}
