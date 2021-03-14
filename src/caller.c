#define DEV
#include "thread.h"
#include<unistd.h>
#include<stdio.h>
#include <sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include"attributetypes.h"
int arr[] = {0,0,0,0,0,0};
/**
 * @brief Routine run by the thread
 * 
 * @param a Argument passed to the local context
 * @return int 
 */
int func(void *a){
    sleep(*((int*)a));
    return 0;
}

/**
 * @brief Caller function 
 * 
 * @return int 
 */
int main(){
    thread_attr attribute;
    thread_attr_init(&attribute);
    thread_attr_setStack(&attribute,4096);
    printf("Stack size %ld \n",thread_attr_getStack(&attribute));
    thread t1, t2, t3;
    printf("Process ID : %d\n",getpid());
    int a = 5;
    pid_t tid1 = create(&t1, NULL, func, (void*) &a, 0);
    int b = 10;
    pid_t tid2 = create(&t2, NULL, func, (void*) &b, 0);
    int c = 15;
    pid_t tid3 = create(&t3, NULL, func, (void*) &c, 0);
    // getchar();
    printf("Thread 1 ID : %d\n", tid1);
    printf("Thread 2 ID : %d\n", tid2);
    printf("Thread 3 ID : %d\n", tid3);
    thread_join(tid1, NULL);    
    thread_join(tid2, NULL);
    thread_join(tid3, NULL);
    #ifdef DEV
    #endif
    thread_attr_destroy(&attribute);
    return 0;
}
