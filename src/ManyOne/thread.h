/**
 * @file thread.h
 * @author Anup Nair & Hrishikeh Athalye
 * @brief API Interface of tlib for programs
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>

/**
 * @brief Thread object
 */
typedef unsigned long int thread;

/**
 * @brief Spinlock object
 */
typedef volatile int spin_t;

/**
 * @brief Mutex object
 */
typedef volatile int mut_t;

/**
 * @brief Attribute parameter for controlling scheduling time
 */
typedef struct schedParams {
    unsigned int sc;
    unsigned int ms;
} schedParams;

/**
 * @brief Attribute object for each thread
 */
typedef struct thread_attr {
    void *stack;
    size_t stackSize;
    schedParams schedInterval;
} thread_attr;

// Thread related APIs
int thread_create(thread *, void *, void *, void *);

int thread_join(thread , void **);

int thread_kill(pid_t, int);

int thread_exit(void *);

// Spin Lock related APIs
int spin_init(spin_t *);

int spin_acquire(spin_t *);

int spin_release(spin_t *);

// Mutex related APIs
int mutex_init(mut_t *);

int mutex_acquire(mut_t *);

int mutex_release(mut_t *);


// Thread attribute handler APIs
int thread_attr_init(thread_attr *);
int thread_attr_destroy(thread_attr *);

size_t thread_attr_getStack(thread_attr *);
int thread_attr_setStack(thread_attr *,size_t);

int thread_attr_setSchedInterval(thread_attr *, schedParams);
schedParams* thread_attr_getSchedInterval(thread_attr *);

int thread_attr_setStackAddr(thread_attr *, void *,size_t);

