#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "thread.h"
#endif
#include "tests.h"
#include "attributetypes.h"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "log.h"

void func(void *i){
    printf("In thread %d\n",*(int *)i);
    // sleep(*(int *)i);
    return;
}
int main(){
    int j = 1;
    thread t;
    thread_create(&t,NULL,func,(void *)&j,1);
    j++;
    thread t2;
    thread_create(&t2,NULL,func,(void *)&j,1);
    j++;
    thread t3;
    thread_create(&t3,NULL,func,(void *)&j,1);
    j++;
    thread t4;
    thread_create(&t4,NULL,func,(void *)&j,1);
    log_info("Main exiting");
    return 0;
}