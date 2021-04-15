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

typedef volatile int spin_t;
/**
 * @brief Mutex object
 */
typedef volatile int mut_t;

// Spin lock APIs
int spin_init(spin_t *);

int spin_acquire(spin_t *);

int spin_release(spin_t *);

// Mutex lock APIs
int mutex_init(mut_t *);

int mutex_acquire(mut_t *);

int mutex_release(mut_t *);