#include<stdatomic.h>
#include<unistd.h>
typedef unsigned long int thread;
typedef volatile atomic_flag mut_t;



//threadModes : 0 - one-to-one, 1 - many one, 2 - many many

int thread_create(thread *t,void *attr, void *routine,void *arg, int threadMode);
int thread_join(thread t, void **retLocation);
int thread_kill(pid_t t, int signum);
void thread_exit(void *);

int createOneOne(thread *t, void *attr,void * routine, void *arg);
int createManyOne(thread *t, void *attr,void * routine, void *arg);
int createManyMany(thread *t, void *attr,void * routine, void *arg);

int spin_init(mut_t*);
int spin_acquire(mut_t*);
int spin_release(mut_t*);

int mutex_init(mut_t*);
int mutex_acquire(mut_t*);
int mutex_release(mut_t*);

