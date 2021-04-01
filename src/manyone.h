#include<stdatomic.h>
#include<unistd.h>
typedef unsigned long int thread;
typedef volatile atomic_flag mut_t;
int mo_thread_join(thread , void **);