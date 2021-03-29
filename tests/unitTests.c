#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "thread.h"
#endif
#include "tests.h"
#include "attributetypes.h"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "log.h"
#define TEST_STSZ 8192
#define TEST_GDSZ 2048
#define gettid() syscall(SYS_gettid)

jmp_buf buffer;
jmp_buf buffer1;

void routine(void *i){
    int a = 10;
    long b = a * 100;
    // sleep(2 * *(int *)i);
    // sleep(2);
    // log_trace("thread exited\n");
    return;
}

/**
 * @brief Tests creation of multiple threads with the one one model.
 * 
 */
void testCreate(){
    mut_t lock;
    int s = 0,f = 0;
    printf("tlib creation test started...\n");
    thread t[10];
    for(int i = 0 ;i < 10; i++){
        if(thread_create(&t[i],NULL,routine,(void *)&i,0) == 0){
            log_info("Thread %d created successfully with id %ld",i,t[i]);
            s++;
        }
        else{
            log_error("Thread creation failed\n");
            f++;
        }
    }
    for(int i = 0 ; i < 10 ;i++)
            thread_join(t[i],NULL);


    printf(RESET"Test completed with the following statistics:\n");
    printf(GREEN"Success: %d\n",s);
    printf(RED"Failures: %d\n"RESET,f);
    return;
}

/**
 * @brief Functions to test joining on threads
 * 
 */
void routineJoin(){
    sleep(1);
}
void testJoin(){
    //Sleep in different intervals (Pipelined test)
    //Order of join 
    int s = 0,f = 0;
    printf("tlib join test started...\n");
    thread t[10];
    puts("");
    log_trace("Joining threads upon creation in a sequential order");
    for(int i = 0 ;i < 5; i++){
        if(thread_create(&t[i],NULL,routineJoin,(void *)&i,0) == 0){
            log_info("Thread %d created successfully with id %ld",i,t[i]);
            thread_join(t[i],NULL);
            s++;
        }
        else{
            log_error("Thread creation failed\n");
            f++;
        }
    }
    puts("");
    log_trace("Joining on threads after creation");
    for(int i = 0 ;i < 5; i++){
        if(thread_create(&t[i],NULL,routineJoin,(void *)&i,0) == 0){
            log_info("Thread %d created successfully with id %ld",i,t[i]);
            s++;
        }
        else{
            log_error("Thread creation failed\n");
            f++;
        }
    }
    for(int i = 0 ; i < 5;i++)
        thread_join(t[i],NULL);
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
    printf("tlib stack test started...\n");

    signal(SIGSEGV,handleseg);
    thread t;
    thread_attr attr;
    if(thread_attr_init(&attr)){
        log_error("Attribute initialisation failed\n");
    }
    thread_attr_setStack(&attr,8);
    thread_create(&t,&attr,routine,NULL,0);
    return;
}


void exitroutine1(void *lock){

    log_info("Exiting thread 1");

} 

void exitroutine2(void *lock){

    log_info("Exiting thread 2");

} 
/**
 * @brief Checking exit of threads
 * 
 */
void testExit(){
    printf("tlib exit test started...\n");
    thread t,t2;
    mut_t lock;
    spin_init(&lock);
    thread_create(&t,NULL,exitroutine1,(void *)&lock,0);
    thread_create(&t2,NULL,exitroutine2,(void *)&lock,0);
    thread_join(t,NULL);
    thread_join(t2,NULL);
    printf("Joining Complete\n");
    printf(GREEN"Test Passed\n"RESET);
}

void handlesegfault(int signo){
    log_info("Thread received signal number %d",signo);
    longjmp(buffer1,1);
}


void sigroutine(){
    signal(SIGSEGV,handlesegfault);
    int i = 0;
    while(i < 5){puts("F"); sleep(1); i++;}
    log_info("Exiting");
    return;
}

void sigroutine1(){
    while(1) log_trace("Waiting to be terminated");
}

void handlestop(){
    log_info("Process received signal");
    return;
}

/**
 * @brief Functions to test signal handling  
 * 
 */
void testSig(){
    // Send a thread specific signal
    signal(SIGINT,handlestop);
    int ret;
    if(setjmp(buffer1) == 0){
        thread t1;
        thread_create(&t1,NULL,sigroutine,NULL,0);
        ret = thread_kill(t1, SIGSTOP);
        thread_join(t1,NULL);
    }
    else{
        // Send a process specific signal
        // thread t2;
        // create(&t2,NULL,sigroutine1,NULL,0);
        // ret = thread_kill(t2, SIGALRM);
        // thread_join(t2,NULL);
    } 
    printf(GREEN"Test Passed\n"RESET);
}


/**
 * @brief Functions to check handling of attributes viz. stack, stack and guard page sizes. 
 * 
 */

void attrroutine(){
    puts("Thread spawned with all attributes");
}
void testAttr(){
    printf("tlib attribute test started...\n");
    short err = 0; 
    thread t1;
    void *newstack = malloc(TEST_STSZ/2);
    thread_attr a;
    thread_attr_init(&a);
    thread_attr_setStack(&a,TEST_STSZ) == -1 ? log_error("Failed to set new stack size"),err=1 : log_info("Stack size changed");
    thread_attr_setGuard(&a,TEST_GDSZ)  == -1 ? log_error("Failed to set new guard page size"),err=1 : log_info("Guard page size changed");
    thread_attr_getStack(&a) != TEST_STSZ ? log_error("Stack size does not match"),err=1 : log_info("Set stack size to %d",TEST_STSZ);

    thread_attr_setStackAddr(&a,newstack,TEST_STSZ/2)  == -1 ? log_error("Failed to set new stack"),err=1 : log_info("Stack changed");  
    thread_attr_getGuard(&a) != TEST_GDSZ ? log_error("Guard page size does not match"),err=1 : log_info("Set guard page size to %d",TEST_GDSZ);
    if(err){
        printf(RED"Test failed"RESET);
        return;
    }
    thread_create(&t1,&a,attrroutine,NULL,0);
    thread_join(t1,NULL);
    thread_attr_destroy(&a);
    printf(GREEN"Test Passed\n"RESET);
    return;
}
int i = 0;
void lockroutine(void *lock){
    mutex_acquire((mut_t *)lock);
    printf("Critical Section\n");
    i++;
    printf("%d\n",i);
    mutex_release((mut_t *)lock);
} 
void testLock(){
    thread t,g;
    mut_t lock;
    mutex_init(&lock);
    thread_create(&t,NULL,lockroutine,(void *)&lock,0);
    thread_create(&g,NULL,lockroutine,(void *)&lock,0);
    thread_join(t,NULL);
    thread_join(g,NULL);
}
/**
 * @brief Caller function
 * 
 */
int main(int argc,char *argv[]){
    setbuf(stdout, NULL);
    // testCreate();
    // LINE;
    // if(setjmp(buffer) == 0)
    //     testStack();
    // else{
    //     printf(GREEN"Test Passed\n"RESET);
    // }
    // LINE;
    // testJoin();
    // LINE;
    // testExit();
    // LINE;
    testAttr();
    // testLock();
    return 0;
}