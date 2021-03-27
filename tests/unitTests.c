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
#define gettid() syscall(SYS_gettid)

jmp_buf buffer;

void routine(void *i){
    int a = 10;
    long b = a * 100;
    // sleep(2 * *(int *)i);
    // sleep(2);
    flockfile(stdout);
    // printf("Yes %d\n",*(int *)(i));
    funlockfile(stdout);
    return;
}

/**
 * @brief Tests creation of multiple threads with the one one model.
 * 
 */
void testCreate(){
    mut_t lock;
    spin_init(&lock);
    int s = 0,f = 0;
    printf("tlib creation test started...\n");
    thread t[10];
    for(int i = 0 ;i < 10; i++){
        if(create(&t[i],NULL,routine,(void *)&i,0) == 0){
            spin_acquire(&lock);
            log_info("Thread %d created successfully with id %ld",i,t[i]);
            spin_release(&lock);
            s++;
        }
        else{
            log_error("Thread creation failed\n");
            f++;
        }
    }
    for(int i = 9 ; i > 0 ;i--)
            thread_join(t[i],NULL);

    spin_acquire(&lock);
    printf(RESET"Test completed with the following statistics:\n");
    printf(GREEN"Success: %d\n",s);
    printf(RED"Failures: %d\n"RESET,f);
    spin_release(&lock);
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
            log_info("Thread %d created successfully with id %ld\n",i,t[i]);
            s++;
        }
        else{
            log_error("Thread creation failed\n");
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
        log_error("Attribute initialisation failed\n");
    }
    // void *stack = malloc(8);
    // thread_attr_setStackAddr(&attr,stack);
    thread_attr_setStack(&attr,8);
    create(&t,&attr,routine,NULL,0);
    return;
}

void exitroutine1(){
    log_info("Exiting thread 1");
} 


void exitroutine2(){
    log_info("Exiting thread 2");
} 

void testExit(){
    thread t,t2;
    create(&t,NULL,exitroutine1,NULL,0);
    create(&t2,NULL,exitroutine2,NULL,0);
    thread_join(t,NULL);
    thread_join(t2,NULL);
    printf("Joining Complete\n");
    printf(GREEN"Test Passed\n"RESET);
}

// void handleCont(int signo){
//     printf("Thread Continued\n");
//     kill(,signo)
// }

void handleStop(int signo){
    printf("Thread Stopped\n");
}

int test = 1;

void sigroutine(){
    // signal(SIGSEGV,SIG_IGN);
    // sleep(2);
    printf("Stopped and resumed\n");
    return;
}
// problem singnals are being sent to make program instead of the thread routine

void testSig(){
    thread t1;
    // printf("Thread id of the main thread %d %d\n",gettid(), getpid());
    create(&t1,NULL,sigroutine,NULL,0);
    int ret;
    // ret = thread_kill(t1, SIGSTOP);
    // SIGINT SIGSTOP and SIGCONT should affect the entire process
    ret = thread_kill(t1, SIGTSTP);
    // thread_join(t1,NULL);
    printf(GREEN"Test Passed\n"RESET);
}

/**
 * @brief Caller function
 * 
 */
int main(int argc,char *argv[]){
    setbuf(stdout, NULL);
    testCreate();
    LINE;
    if(setjmp(buffer) == 0)
        testStack();
    else{
        printf(GREEN"Test Passed\n"RESET);
    }
    LINE;
    testExit();
    LINE;
    // testSig();
    return 0;
}