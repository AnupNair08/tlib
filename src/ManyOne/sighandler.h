#include<signal.h>
#include<stdio.h>

#define WRAP_SIGNALS \
    signal(SIGTERM, TLIB_SIG_HANDLER); \
    signal(SIGFPE, TLIB_SIG_HANDLER); \
    signal(SIGSYS, TLIB_SIG_HANDLER); \
    signal(SIGPIPE, TLIB_SIG_HANDLER); \
    signal(SIGABRT, TLIB_SIG_HANDLER); \

 
void TLIB_SIG_HANDLER(int signum){
    printf("Thread specific signal handled\n");
}

