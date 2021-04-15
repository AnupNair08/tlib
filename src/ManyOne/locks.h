#define _GNU_SOURCE
#include <stdatomic.h>
#include "log.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/futex.h>

typedef volatile int spin_t;
typedef volatile int mut_t;

int spin_init(spin_t *);

int spin_acquire(spin_t *);

int spin_release(spin_t *);

int mutex_init(mut_t *);

int mutex_acquire(mut_t *);

int mutex_release(mut_t *);