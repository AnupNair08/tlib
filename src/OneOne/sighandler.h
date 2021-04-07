/**
 * @file sighandler.h
 * @author Anup Nair & Hrishikesh Athalye
 * @brief Custom handler definitions for signals
 * @version 0.1
 * @date 2021-04-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define RED "\033[1;31m"
#define RESET "\033[0m"

/**
 * @brief Macro for installing custom signal handlers for threads
 * 
 */
#define WRAP_SIGNALS(signum) \
    signal(signum,TLIB_SIG_HANDLER); \
    sigemptyset (&base_mask); \
    sigaddset (&base_mask, signum); \
    sigprocmask(SIG_UNBLOCK,&base_mask,NULL); \

/**
 * @brief Custom signal handler function
 * 
 * @param signum Signal Number
 */
void TLIB_SIG_HANDLER(int signum){
    printf(RED"Signal Dispatched\n"RESET);
    printf("Thread tid %ld handled signal\n",(long)gettid());
}