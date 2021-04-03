#include<signal.h>
#include<stdio.h>
#include<unistd.h>
#define WRAP_SIGNALS(signum) \
    signal(signum,TLIB_SIG_HANDLER); \
    sigemptyset (&base_mask); \
    sigaddset (&base_mask, signum); \
    sigprocmask(SIG_UNBLOCK,&base_mask,NULL); \


void TLIB_SIG_HANDLER(int signum){
    printf("Thread tid %ld handled signal\n",(long)gettid());
}