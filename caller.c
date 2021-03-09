#include "thread.h"
#include<unistd.h>
#include<stdio.h>
#include <sys/types.h>
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
    thread t,t1;
    printf("Process ID : %d\n",getpid());
    pid_t tid = create(&t,func,NULL,"argument");
    pid_t tid2 = create(&t1,func,NULL,"argument");

    getchar();
    printf("Thread ID : %d\n",tid);
    printf("Thread ID : %d\n",tid2);

    
    return 0;
}