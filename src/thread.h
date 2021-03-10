typedef unsigned long int thread;

//threadModes : 0 - one-to-one, 1 - many one, 2 - many many

thread create(thread *t, void *routine,void *attr,void *arg, int threadMode);
thread createOneOne(thread *t, void * routine,void *attr, void *arg);
thread createManyOne(thread *t, void * routine,void *attr, void *arg);
thread createManyMany(thread *t, void * routine,void *attr, void *arg);