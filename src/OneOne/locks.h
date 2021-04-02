#include<stdatomic.h>
#include"thread.h"
#include"log.h"
#include <sys/syscall.h>
#include<stdio.h>
#include<linux/futex.h>

int spin_init(mut_t*);

int spin_acquire(mut_t *);

int spin_release(mut_t *);

int mutex_init(mut_t *);

int mutex_acquire(mut_t *);

int mutex_release(mut_t *);