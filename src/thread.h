typedef unsigned long int thread;

//threadModes : 0 - one-to-one, 1 - many one, 2 - many many

int create(thread *t,void *attr, void *routine,void *arg, int threadMode);
int thread_join(thread t, void **retLocation);


int createOneOne(thread *t, void *attr,void * routine, void *arg);
int createManyOne(thread *t, void *attr,void * routine, void *arg);
int createManyMany(thread *t, void *attr,void * routine, void *arg);


