/**
 * @file sighandler.h
 * @author Anup Nair & Hrishikesh Athalye
 * @brief Custom signal hanlder wrappers for threads
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include<signal.h>
#include<stdio.h>
#define RED "\033[1;31m"
#define RESET "\033[0m"
#define GREEN "\e[0;32m"
#define BLUE "\033[1;34m"

/**
 * @brief Default signal handlers for signals sent via thread_kill()
 * 
 * @param signum Signal number to be sent
 */
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

 

