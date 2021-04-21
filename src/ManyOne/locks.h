/**
 * @file locks.h
 * @author Hrishikesh Athalye & Anup Nair
 * @brief 
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <linux/futex.h>

/**
 * @brief Spin Lock object
 */

typedef struct
{
    volatile int lock;
    unsigned int locker;
} spin_t;
/**
 * @brief Mutex object
 */
typedef struct
{
    volatile int lock;
    unsigned int locker;
} mutex_t;

// Spin lock APIs
int spin_init(spin_t *);

int spin_acquire(spin_t *);

int spin_release(spin_t *);

int spin_trylock(spin_t *);

// Mutex lock APIs
int mutex_init(mutex_t *);

int mutex_acquire(mutex_t *);

int mutex_release(mutex_t *);

int mutex_trylock(mutex_t *);
