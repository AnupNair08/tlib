/**
 * @file unitTests.c
 * @author Hrishikesh Athalye & Anup Nair
 * @brief Unit Tests for tlib to test creation, join, exit, attribute, locks and signals
 * @version 0.1
 * @date 2021-04-07
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
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#define TEST_STSZ 8192
#define TEST_GDSZ 2048
#define gettid() syscall(SYS_gettid)

int TotalTests = 0;
int success = 0;
int failure = 0;
spin_t printlock;

void globalhandle()
{
    printf("----------Process wide signal caught by handler----------\n");
    printf(GREEN "Test Passed\n" RESET);
    success += 1;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Tests creation of multiple threads with the one one model.
 * 
 */

void routine(void *i)
{
    int a = 10;
    return;
}
void testCreate()
{
    TotalTests += 1;
    int s = 0, f = 0;
    printf(BLUE "Testing thread_create()\n\n" RESET);
    thread t[10];
    for (int i = 0; i < 10; i++)
    {
        if (thread_create(&t[i], NULL, routine, (void *)&i) == 0)
        {
            printf("Thread %d created successfully with id %ld\n", i, t[i]);
            s++;
        }
        else
        {
            printf("Thread creation failed\n");
            f++;
        }
    }
    for (int i = 0; i < 10; i++)
        thread_join(t[i], NULL);

    printf(RESET "Create test completed with the following statistics:\n");
    printf("Success: %d\n", s);
    printf("Failures: %d\n", f);
    if (f)
        failure += 1;
    else
        success += 1;
    return;
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Tests the joining on threads in a pipelined and sequential order
 * 
 */
void routineJoin()
{
    sleep(1);
}
void testJoin()
{
    TotalTests += 1;
    int s = 0, f = 0;
    printf(BLUE "Testing thread_join()\n" RESET);
    thread t[10];
    puts("");
    printf("Joining threads upon creation in a sequential order\n");
    for (int i = 0; i < 5; i++)
    {
        if (thread_create(&t[i], NULL, routineJoin, (void *)&i) == 0)
        {
            printf("Thread %d created successfully with id %ld\n", i, t[i]);
            thread_join(t[i], NULL);
            s++;
        }
        else
        {
            printf("Thread creation failed\n");
            f++;
        }
    }
    puts("");
    printf("Joining on threads after creation\n");
    for (int i = 0; i < 5; i++)
    {
        if (thread_create(&t[i], NULL, routineJoin, (void *)&i) == 0)
        {
            printf("Thread %d created successfully with id %ld\n", i, t[i]);
            s++;
        }
        else
        {
            printf("Thread creation failed\n");
            f++;
        }
    }
    for (int i = 0; i < 5; i++)
        thread_join(t[i], NULL);
    printf(RESET "Join test completed with the following statistics:\n");
    printf("Success: %d\n", s);
    printf("Failures: %d\n", f);
    if (f)
        failure += 1;
    else
        success += 1;
    return;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Tests the exit of threads
 * 
 */

void exitroutine1(void *retVal)
{
    *(int *)retVal = gettid();
    safeprintf(&printlock, "Exiting thread 1 with return value %d\n", *(int *)retVal);
    thread_exit(retVal);
}

void exitroutine2(void *retVal)
{
    *(int *)retVal = gettid();
    safeprintf(&printlock, "Exiting thread 2 with return value %d\n", *(int *)retVal);
    thread_exit(retVal);
}
void testExit()
{
    int *retVal1 = (int *)malloc(sizeof(int));
    int *retVal2 = (int *)malloc(sizeof(int));
    int arg1, arg2;
    TotalTests += 1;
    printf(BLUE "Testing thread_exit()\n\n" RESET);
    thread t1, t2;
    int *retVal = (int *)malloc(sizeof(int));
    spin_t lock;
    spin_init(&lock);
    thread_create(&t1, NULL, exitroutine1, (void *)&arg1);
    thread_create(&t2, NULL, exitroutine2, (void *)&arg2);
    thread_join(t1, (void **)&retVal1);
    thread_join(t2, (void **)&retVal2);
    printf("Joined thread 1 with return value %d\n", *(retVal1));
    printf("Joined thread 2 with return value %d\n", *(retVal2));
    printf(GREEN "Test Passed\n" RESET);
    success += 1;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Testing Signal Handling for process and thread wide signals
 * 
 */

int sigroutine()
{
    int i = 0;
    while (i < 3)
    {
        puts("Running Thread..");
        sleep(1);
        i++;
    }
    return 0;
}
void testSig()
{
    TotalTests += 3;
    printf(BLUE "Testing thread_kill()\n\n" RESET);
    spin_t lock;
    spin_init(&lock);
    thread t1, t2, t3;
    printf("Sending a signal to a running thread\n");
    thread_create(&t1, NULL, sigroutine, NULL);
    sleep(1);
    int ret = thread_kill(t1, SIGTERM);
    thread_join(t1, NULL);
    if (ret != -1)
    {
        printf(GREEN "Test passed\n" RESET);
        success += 1;
    }
    else
    {
        printf(RED "Test failed\n" RESET);
        failure += 1;
    }

    printf("\nSending a signal to an already exited thread\n");
    thread_create(&t2, NULL, sigroutine, NULL);
    thread_join(t2, NULL);
    ret = thread_kill(t2, SIGTERM);
    if (ret == -1)
    {
        printf(GREEN "Test passed\n" RESET);
        success += 1;
    }
    else
    {
        printf(RED "Test failed\n" RESET);
        failure += 1;
    }

    printf("\nSending a process wide signal\n");
    thread_create(&t3, NULL, sigroutine, NULL);
    thread_join(t3, NULL);
    ret = thread_kill(t3, SIGINT);
    if (ret == -1)
        failure += 1;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Testing thread attributes creation and updation
 * 
 */

void attrroutine()
{
    puts("Thread spawned with all attributes");
}
void testAttr()
{
    TotalTests += 1;
    printf(BLUE "Testing thread_attr_*()\n\n" RESET);
    short err = 0;
    thread t1;
    thread_attr *a = (thread_attr *)malloc(sizeof(thread_attr));
    thread_attr_init(a);
    thread_attr_setStack(a, TEST_STSZ) == -1 ? printf("Failed to set new stack size\n"), err = 1 : printf("Stack size changed\n");
    thread_attr_setGuard(a, TEST_GDSZ) == -1 ? printf("Failed to set new guard page size\n"), err = 1 : printf("Guard page size changed\n");
    thread_attr_getStack(a) != TEST_STSZ ? printf("Stack size does not match\n"), err = 1 : printf("Set stack size to %d\n", TEST_STSZ);

    thread_attr_getGuard(a) != TEST_GDSZ ? printf("Guard page size does not match\n"), err = 1 : printf("Set guard page size to %d\n", TEST_GDSZ);
    if (err)
    {
        printf(RED "Test failed\n" RESET);
        failure += 1;
        return;
    }
    thread_create(&t1, a, attrroutine, NULL);
    thread_join(t1, NULL);
    thread_attr_destroy(a);
    printf(GREEN "Test Passed\n" RESET);
    success += 1;
    return;
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief Testing Locks with a simple crictical section
 * 
 */

int i = 0;
int refstring[2] = {0, 0};
void lockroutine(void *lock)
{
    mutex_acquire((mutex_t *)lock);
    printf("Critical Section\n");
    refstring[i++] = i;
    printf("%d\n", i);
    mutex_release((mutex_t *)lock);
}
void testLock()
{
    TotalTests += 1;
    printf(BLUE "Testing mutex_*()\n\n" RESET);
    thread t, g;
    mutex_t lock;
    mutex_init(&lock);
    thread_create(&t, NULL, lockroutine, (void *)&lock);
    thread_create(&g, NULL, lockroutine, (void *)&lock);
    thread_join(t, NULL);
    thread_join(g, NULL);
    if (refstring[0] == 1 && refstring[1] == 2)
    {
        printf(GREEN "Test Passed\n" RESET);
        success += 1;
    }
    else
    {
        printf(RED "Test failed\n" RESET);
        failure += 1;
    }
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Caller function
 * 
 */
int main(int argc, char *argv[])
{
    signal(SIGINT, globalhandle);
    printf("\nRunning Unit Tests\n");
    LINE;
    testCreate();
    LINE;
    testJoin();
    LINE;
    testAttr();
    LINE;
    testLock();
    LINE;
    testSig();
    LINE;
    testExit();
    LINE;

    printf("Test completed with the following statistics:\n");
    printf("Total Tests: %d\n", TotalTests);
    printf(GREEN "Success: %d\n" RESET, success);
    printf(RED "Failed: %d\n" RESET, failure);

    return 0;
}