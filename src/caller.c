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
    int m = 10;
    int b = 20;
    // puts("In thread");
    // sleep(atoi(a));
    // write(2,"Hello\n",6);
    // puts((char *)a);
    //printf does not work probably coz its not thread safe
    if(atoi(a) == 1){
        for(int i = 0 ; i < 3 ;i++){
            arr[i] = i + 1;
        }
    }
    else{
        for(int i = 3 ;i < 6; i++)
            arr[i] = 9;
    }
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
    printf("Stack size %d \n",thread_attr_getStack(&attribute));
    thread_attr_setStack(&attribute,4096);
    printf("Stack size %d \n",thread_attr_getStack(&attribute));

    
    thread t,t1;
    printf("Process ID : %d\n",getpid());
    pid_t tid = create(&t,&attribute,func,"1",0);
    pid_t tid2 = create(&t1,NULL,func,"2",0);
    // getchar();
    printf("Thread 1 ID : %d\n",tid);
    printf("Thread 2 ID : %d\n",tid2);

    #ifdef DEV
        thread_join(tid,NULL); 
        thread_join(tid2,NULL);
    #endif

    thread_attr_destroy(&attribute);
    for(int i = 0 ; i < 6 ;i++) printf("%d ",arr[i]);
    printf("Returned from main");   
    return 0;
}