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

/**
 * @brief Atomic Lock Object
 * 
 */
typedef volatile int spin_t;
typedef volatile int mutex_t;
// Spin Lock related functions
int spin_init(spin_t*);
int spin_acquire(spin_t *);
int spin_release(spin_t *);
// Mutex related functions
int mutex_init(mutex_t *);
int mutex_acquire(mutex_t *);
int mutex_release(mutex_t *);