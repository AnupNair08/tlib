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
    puts("In thread");
    //sleep(100);
    write(2,"Hello\n",6);
    puts((char *)a);
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
    pid_t tid = create(&t,func,NULL,"thread 1",0);
    pid_t tid2 = create(&t1,func,NULL,"thread 2",0);

    getchar();
    printf("Thread 1 ID : %d\n",tid);
    printf("Thread 2 ID : %d\n",tid2);

    
    return 0;
}