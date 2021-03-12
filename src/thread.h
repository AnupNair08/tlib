typedef unsigned long int thread;

//threadModes : 0 - one-to-one, 1 - many one, 2 - many many

thread create(thread *t,void *attr, void *routine,void *arg, int threadMode);
int thread_join(thread t, void **retLocation);


thread createOneOne(thread *t, void *attr,void * routine, void *arg);
thread createManyOne(thread *t, void *attr,void * routine, void *arg);
thread createManyMany(thread *t, void *attr,void * routine, void *arg);

typedef struct tcb {
    thread tid;
    // struct attr;
    char *stack;
} tcb;