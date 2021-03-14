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
    
    printf("Process ID : %d\n",getpid());
    
    thread t1, t2, t3;
    int a = 5;
    create(&t1, NULL, func, (void*) &a, 0);
    int b = 10;
    create(&t2, NULL, func, (void*) &b, 0);
    int c = 15;
    create(&t3, NULL, func, (void*) &c, 0);
    
    printf("Thread 1 ID : %ld\n", t1);
    printf("Thread 2 ID : %ld\n", t2);
    printf("Thread 3 ID : %ld\n", t3);
    
    thread_join(t1, NULL);    
    thread_join(t2, NULL);
    thread_join(t3, NULL);
    
    thread_attr_destroy(&attribute);
    return 0;
}
