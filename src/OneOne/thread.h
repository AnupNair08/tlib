#define _GNU_SOURCE
#include<stdatomic.h>
#include<unistd.h>
typedef unsigned long int thread;
typedef volatile atomic_flag mut_t;



//threadModes : 0 - one-to-one, 1 - many one, 2 - many many

int thread_create(thread *,void *, void *,void *);
int thread_join(thread , void **);
int thread_kill(pid_t t, int);
void thread_exit(void *);

int spin_init(mut_t*);
int spin_acquire(mut_t*);
int spin_release(mut_t*);

int mutex_init(mut_t*);
int mutex_acquire(mut_t*);
int mutex_release(mut_t*);

