/**
 * @file tlib.h
 * @author Anup Nair & Hrishikesh Athalye
 * @brief Internal Library APIs
 * @version 0.1
 * @date 2021-04-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include<unistd.h>
#include<stdlib.h>

/**
 * @brief Default stack size for a thread 
 */
#define STACK_SZ 65536
/**
 * @brief Default guard page size for a thread 
 */
#define GUARD_SZ getpagesize()
/**
 * @brief Flags passed to clone system call in one-one implementation 
 */
#define CLONE_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD |CLONE_SYSVSEM|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID
#define TGKILL 234


/**
 * @brief Thread Object
 * 
 */
typedef unsigned long thread;

/**
 * @brief Attribute types for thread object
 * 
 */
typedef struct thread_attr {
    void *stack;
    size_t guardSize;
    size_t stackSize;
} thread_attr;

// Thread APIs
int thread_create(thread *,void *, void *,void *);
int thread_join(thread , void **);
int thread_kill(pid_t t, int);
void thread_exit(void *);

// Thread attribute APIs
int thread_attr_init(thread_attr *);
int thread_attr_destroy(thread_attr *);

size_t thread_attr_getStack(thread_attr *);
int thread_attr_setStack(thread_attr *,size_t);

size_t thread_attr_getGuard(thread_attr *);
int thread_attr_setGuard(thread_attr *,size_t);

int thread_attr_setStackAddr(thread_attr *, void *,size_t);

