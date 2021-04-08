#include<signal.h>
#include<stdio.h>
#define RED "\033[1;31m"
#define RESET "\033[0m"
#define GREEN "\e[0;32m"
#define BLUE "\033[1;34m"

void TLIB_SIG_HANDLER(int signum){
    printf(RED"Dispatched Signal\n"RESET);
    printf("Thread specific signal handled\n");
}

#define WRAP_SIGNALS \
    signal(SIGTERM, TLIB_SIG_HANDLER); \
    signal(SIGFPE, TLIB_SIG_HANDLER); \
    signal(SIGSYS, TLIB_SIG_HANDLER); \
    signal(SIGPIPE, TLIB_SIG_HANDLER); \
    signal(SIGABRT, TLIB_SIG_HANDLER); \

 

