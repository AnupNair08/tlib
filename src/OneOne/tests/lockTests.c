#include<stdio.h>
#include<unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "../thread.h"
#endif
#include "tests.h"
#include "../log.h"




spin_t lock;
long c1,c2,c;
long volatile run = 1;

mutex_t lk; 

void *f1(void *lock){
    while(run){
        c1++;
        mutex_acquire((mutex_t *)lock);
        c++;
        mutex_release((mutex_t *)lock);
    }
}
void *f2(void *lock){
    while(run){
        c2++;
        mutex_acquire((mutex_t *)lock);
        c++;
        mutex_release((mutex_t *)lock);
    }
}

int testMutex(){
    printf("Starting test with Mutex\n");
    mutex_init(&lk);
    thread t1,t2;
    thread_create(&t1,NULL,f1,(void *)&lk);
    thread_create(&t2,NULL,f2,(void *)&lk);
    sleep(2);
    run = 0;
    thread_join(t1,NULL);
    thread_join(t2,NULL);
    log_trace("\nValues after test are (c1 + c2)=%ld c=%ld",c1+c2,c);
    if(c1 + c2 != c) printf(RED"Test failed\n"RESET);
    else printf(GREEN"Test passed\n"RESET);
    return 0;
}
void *f1spin(void *lock){
    while(run){
        c1++;
        spin_acquire((spin_t *)lock);
        c++;
        spin_release((spin_t *)lock);
    }
}
void *f2spin(void *lock){
    while(run){
        c2++;
        spin_acquire((spin_t *)lock);
        c++;
        spin_release((spin_t *)lock);
    }
}

int testSpin(){
    c1 = 0;
    c2 = 0;
    run = 1;
    c = 0;
    printf("Starting test with Spinlocks\n");
    spin_init(&lock);
    thread t1,t2;
    thread_create(&t1,NULL,f1spin,(void *)&lock);
    thread_create(&t2,NULL,f2spin,(void *)&lock);
    sleep(2);
    run = 0;
    thread_join(t1,NULL);
    thread_join(t2,NULL);
    log_trace("\nValues after test are (c1 + c2)=%ld c=%ld",c1+c2,c);
    if(c1 + c2 != c) printf(RED"Test failed\n"RESET);
    else printf(GREEN"Test passed\n"RESET);
    return 0;
}

int main(){
    setbuf(stdout, NULL);
    printf(BLUE"Testing Synchronization primitives\n\n"RESET);
    testMutex();
    testSpin();
    return 0;
}

