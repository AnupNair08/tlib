typedef unsigned long int thread;

//threadModes : 0 - one-to-one, 1 - many one, 2 - many many

int create(thread *t, void *routine,void *attr,void *arg, int threadMode);
int createOneOne(thread *t, void * routine,void *attr, void *arg);
int createManyOne(thread *t, void * routine,void *attr, void *arg);
int createManyMany(thread *t, void * routine,void *attr, void *arg);