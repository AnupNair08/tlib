#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "../thread.h"
#endif
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include "tests.h" 

void func(){
    printf("Dummy routine\n");
    thread_exit(NULL);
    return;
}
int main(){
    thread t;
    printf("Creating threads with invalid arguments\n");
    if( thread_create(NULL,NULL,NULL,NULL) == EINVAL 
        && thread_create(&t,NULL,NULL,NULL) == EINVAL
        && thread_create(NULL,NULL,func,NULL) == EINVAL)
    {
        TESTPASS
    } else {
        TESTFAIL
    }

    printf("Joining thread in invalid cases\n");
    thread_create(&t,NULL,func,NULL);
    // Already joined thread
    thread_join(t,NULL);
    // Rejoin or give random TID
    if(thread_join(t,NULL) == ESRCH && thread_join(231,NULL) == ESRCH){
        TESTPASS
    } else{
        TESTFAIL
    }

    printf("Sending invalid signal\n");
    if(thread_kill(t,0) == -1) TESTPASS
    else TESTFAIL
}