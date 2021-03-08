#include "thread.h"
#include<unistd.h>
#include<stdio.h>

/**
 * @brief Routine run by the thread
 * 
 * @param a Argument passed to the local context
 * @return int 
 */
int func(void *a){
    int m = 10;
    int b = 20;
    write(2,"Hello",5);
    puts((char *)a);
    puts("In thread");
    //printf does not work probably coz its not thread safe
    return 0;
}

/**
 * @brief Caller function 
 * 
 * @return int 
 */
int main(){
    thread t;
    create(&t,func,NULL,"argument");
    return 0;
}