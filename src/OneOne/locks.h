/**
 * @file locks.h
 * @author Hrishikesh Athalye & Anup Nair
 * @brief Header files for synchronization primitives
 * @version 0.1
 * @date 2021-04-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdatomic.h>

/**
 * @brief Atomic Lock Object
 * 
 */
typedef volatile atomic_flag mut_t;
 
// Spin Lock related functions
int spin_init(mut_t*);
int spin_acquire(mut_t *);
int spin_release(mut_t *);

// Mutex related functions
int mutex_init(mut_t *);
int mutex_acquire(mut_t *);
int mutex_release(mut_t *);