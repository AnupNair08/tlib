/**
 * @file tlib.h
 * @author Hrishikesh Athalye & Anup Nair
 * @brief API Interface for internal library implementations
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <setjmp.h>
#include "locks.h"

// Default parameters for thread creation and scheduling
#define STACK_SZ 65536
#define GUARD_SZ getpagesize()
#define SCHED_SC 1
#define SCHED_MS 0
#define TGKILL 234

/**
 * @brief States of a thread 
 */
enum thread_state
{
    RUNNING,
    WAITING,
    EXITED,
    RUNNABLE
};

/**
 * @brief Thread object
 */
typedef unsigned long thread;

/**
 * @brief Structure to store thread routine and arguments
 */
typedef struct funcargs
{
    void (*f)(void *);
    void *arg;
} funcargs;


// Thread APIs 
int createManyOne(thread *, void *, void *, void *);
static void initManyOne();
static void *allocStack(size_t, size_t);
void wrapRoutine();
static void starttimer();
void enabletimer();
void disabletimer();
void switchToScheduler();
static void scheduler();
