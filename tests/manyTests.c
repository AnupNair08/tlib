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

void func(){
    printf("In thread");
}
int main(){
    thread t;
    thread_create(&t,NULL,func,NULL,1);
    thread t2;
    thread_create(&t2,NULL,func,NULL,1);
    return 0;
}