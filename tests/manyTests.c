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
#include "manyone.h"

void func1(void *i){
    printf("In thread 1\n");
    fflush(stdout);
    // sleep(*(int *)i);
    return;
}

void func2(void *i){
    printf("In thread 2\n");
    fflush(stdout);
    // sleep(*(int *)i);
    return;
}

void func3(void *i){
    printf("In thread 3\n");
    fflush(stdout);
    // sleep(*(int *)i);
    return;
}

void func4(void *i){
    printf("In thread 4\n");
    fflush(stdout);
    // sleep(*(int *)i);
    return;
}
int main(){
    int j1 = 1;
    thread t;
    thread_create(&t,NULL,func1,(void *)&j1,1);
    int j2 = 2;
    thread t2;
    thread_create(&t2,NULL,func2,(void *)&j2,1);
    int j3 = 3;
    thread t3;
    thread_create(&t3,NULL,func3,(void *)&j3,1);
    int j4 = 4;
    thread t4;
    thread_create(&t4,NULL,func4,(void *)&j4,1);
    while(1){}
    // mo_thread_join(t, NULL);
    // mo_thread_join(t2, NULL);
    // mo_thread_join(t3, NULL);
    // mo_thread_join(t4, NULL);
    log_info("Main exiting");
    return 0;
}