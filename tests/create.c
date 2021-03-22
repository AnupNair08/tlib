#include<stdio.h>
#include<unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "thread.h"
#endif
#include "tests.h"
#include "attributetypes.h"
#include<signal.h>
#include<setjmp.h>

jmp_buf buffer;

void syncprintf(){
    
}

void routine(void *i){
    int a = 10;
    long b = a * 100;
    // sleep(2 * *(int *)i);
    // sleep(2);
    flockfile(stdout);
    printf("Yes %d\n",*(int *)(i));
    funlockfile(stdout);
    return;
}

/**
 * @brief Tests creation of multiple threads with the one one model.
 * 
 */
void testCreate(){
    int s = 0,f = 0;
    printf("tlib creation test started...\n");
    thread t[10];
    for(int i = 0 ;i < 10; i++){
        if(create(&t[i],NULL,routine,(void *)&i,0) == 0){
            flockfile(stdout);
            printf("Thread %d created successfully with id %ld\n",i,t[i]);
            funlockfile(stdout);
            thread_join(t[i],NULL);
            s++;
        }
        else{
            printf(RED"Thread creation failed\n");
            f++;
        }
    }
    for(int i = 9 ; i > 0 ;i--)
    flockfile(stdout);
    printf(RESET"Test completed with the following statistics:\n");
    printf(GREEN"Success: %d\n",s);
    printf(RED"Failures: %d\n"RESET,f);
    funlockfile(stdout);
    return;
}

void testJoin(){
    //Sleep in different intervals (Pipelined test)
    //Order of join 
    int s = 0,f = 0;
    printf("tlib creation test started...\n");
    thread t[10];
    for(int i = 0 ;i < 10; i++){
        if(create(&t[i],NULL,routine,(void *)&i,0) == 0){
            printf("Thread %d created successfully with id %ld\n",i,t[i]);
            s++;
        }
        else{
            printf(RED"Thread creation failed\n");
            f++;
        }
    }
    for(int i = 9 ; i > 0 ;i--){
        thread_join(t[i],NULL);
    }
    printf(RESET"Test completed with the following statistics:\n");
    printf(GREEN"Success: %d\n",s);
    printf(RED"Failures: %d\n"RESET,f);
    return;
}

/**
 * @brief Allocate a stack of 8 bytes which will be overflown and then check if guard page generates
 *        seg fault to provide stack protection.
 * 
 */
void handleseg(){
    printf("Guard page handled stack overflow\nReturning control to main\n");
    longjmp(buffer,1);
}
void testStack(){
    signal(SIGSEGV,handleseg);
    thread t;
    thread_attr attr;
    if(thread_attr_init(&attr)){
        printf(RED"Attribute initialisation failed\n");
    }
    thread_attr_setStack(&attr,8);
    create(&t,&attr,routine,NULL,0);
    return;
}

void routine1(){
    sleep(10);
}

void routine2(){
    thread_exit(NULL);
    sleep(10);
}

void testExit(){
    thread t,t2;
    create(&t,NULL,routine1,NULL,0);
    create(&t2,NULL,routine2,NULL,0);
    thread_join(t,NULL);
    thread_join(t2,NULL);
    printf("Joining Complete\n");
}

/**
 * @brief Caller function
 * 
 */
int main(int arc,char *argv[]){
    // testCreate();
    LINE;
    if(setjmp(buffer) == 0)
        testStack();
    else{
        printf(GREEN"Test Passed\n"RESET);
    }
    // testExit();
    return 0;
}