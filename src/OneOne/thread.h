/**
 * @file thread.h
 * @author Anup Nair & Hrishikesh Athalye
 * @brief API Interface of tlib for programs
 * @version 0.1
 * @date 2021-04-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#define _GNU_SOURCE
#include <unistd.h>
/**
 * @brief Thread object
 */
typedef unsigned long int thread;
/**
 * @brief Mutex Lock Object
 */
typedef struct
{
    volatile int __lock;
    unsigned int __locker;
} mutex_t;

/**
 * @brief Spin Lock Object
 */
typedef struct
{
    volatile int __lock;
    unsigned int __locker;
} spin_t;

/**
 * @brief Attribute Object for threads 
 */
typedef struct thread_attr
{
    void *stack;
    size_t guardSize;
    size_t stackSize;
} thread_attr;

// Thread related APIs
int thread_create(thread *, void *, void *, void *);
int thread_join(thread, void **);
int thread_kill(pid_t t, int);
void thread_exit(void *);

// Spin lock related APIs
int spin_init(spin_t *);
int spin_acquire(spin_t *);
int spin_release(spin_t *);
int spin_trylock(spin_t *);

// Mutex related APIs
int mutex_init(mutex_t *);
int mutex_acquire(mutex_t *);
int mutex_release(mutex_t *);
int mutex_trylock(mutex_t *);

// Thread attribute handler APIs
int thread_attr_init(thread_attr *);
int thread_attr_destroy(thread_attr *);

size_t thread_attr_getStack(thread_attr *);
int thread_attr_setStack(thread_attr *, size_t);

size_t thread_attr_getGuard(thread_attr *);
int thread_attr_setGuard(thread_attr *, size_t);

int thread_attr_setStackAddr(thread_attr *, void *, size_t);
