/**
 * @file tattr.c
 * @author Hrishikesh Athalaye & Anup Nair
 * @brief Attribute handling for threads
 * @version 0.1
 * @date 2021-04-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "tlib.h"
#include "tattr.h"

static thread_attr __default = {NULL, STACK_SZ};

/**
 * @brief Initialize the attribute object
 * 
 * @param t Pointer to a thread_attr object
 * @return int 
 */
int thread_attr_init(thread_attr *t)
{
    t->stackSize = __default.stackSize;
    t->stack = __default.stack;
    t->schedInterval = (schedParams){.sc = SCHED_SC, .ms = SCHED_MS};
    return 0;
}

/**
 * @brief Destroy the attribute object
 * 
 * @param t Pointer to a thread_attr object
 * @return int 
 */
int thread_attr_destroy(thread_attr *t)
{
    if (!t)
        return -1;
    t = NULL;
    return 0;
}
/**
 * @brief Set a user defined stack size for the thread
 * 
 * @param t Pointer to thread_attr object
 * @param size Size of the new stack
 * @return int 
 */
int thread_attr_setStack(thread_attr *t, size_t size)
{
    if (!t)
        return -1;
    t->stackSize = size;
    return 0;
}

size_t thread_attr_getStack(thread_attr *t)
{
    if (!t)
        return -1;
    return t->stackSize;
}

/**
 * @brief Initialize the attribute obj
 * 
 * @param t Pointer to a thread_attr object
 * @return int 
 */
int thread_attr_setStackAddr(thread_attr *t, void *stack, size_t size)
{
    if (!t)
        return -1;
    t->stack = stack;
    t->stackSize = size;
    return 0;
}

/**
 * @brief Set the scheduling interval
 * @note Works only when invoking the first thread
 * 
 * @param t Pointer to a thread_attr object
 * @return int
 */
int thread_attr_setSchedInterval(thread_attr *t, schedParams interval)
{
    if (!t)
        return -1;
    t->schedInterval.sc = interval.sc;
    t->schedInterval.ms = interval.ms;
    return 0;
}

/**
 * @brief Get the scheduling interval
 * 
 * @param t Pointer to a thread_attr object
 * @return schedParams*
 */
schedParams *thread_attr_getSchedInterval(thread_attr *t)
{
    if (!t)
        return NULL;
    return &(t->schedInterval);
}

/**
 * @brief Set a user defined guard page size for the thread
 * 
 * @param t Pointer to thread_attr object 
 * @param size Size of the guard page
 * @return int 
 */
int thread_attr_setGuard(thread_attr *t, size_t size)
{
    if (!t)
        return -1;
    t->guardSize = size;
    return 0;
}

/**
 * @brief Returns the size of the guard page used by the thread
 * 
 * @param t Pointer to thread_attr object
 * @return size_t 
 */
size_t thread_attr_getGuard(thread_attr *t)
{
    if (!t)
        return -1;
    return t->guardSize;
}