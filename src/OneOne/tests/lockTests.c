#include<stdio.h>
#include<unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "../thread.h"
#endif
#include "tests.h"
#include "../attributetypes.h"
#include "../log.h"
#define WITH_LOCKS 1
#define WITHOUT_LOCKS 0


int spinTest = 0;
short refstring[4];

void routine1(void *l){
    if(l) spin_acquire((mut_t *)l);
    printf("Routine 1 Before: %d\n",spinTest);
    refstring[0] = spinTest;
    spinTest++;
    printf("Routine 1 After: %d\n",spinTest);
    refstring[1] = spinTest;
    if(l) spin_release((mut_t *)l);
}

void routine2(void *l){
    if(l) spin_acquire((mut_t *)l);
    printf("Routine 2 Before: %d\n",spinTest);
    refstring[2] = spinTest;
    spinTest++;
    printf("Routine 2 After: %d\n",spinTest);
    refstring[3] = spinTest;
    if(l) spin_release((mut_t *)l);
}

void testLocks(int type){
    thread t1,t2;
    mut_t lock;
    mut_t *temp = NULL;
    if(type){
        spin_init(&lock);
        temp = &lock;
    } 
    thread_create(&t1,NULL,routine1, (void *)temp);
    thread_create(&t2,NULL,routine2, (void *)temp);
    thread_join(t1,NULL);
    thread_join(t2,NULL);

    log_trace("Value of global is %d",spinTest);
    spinTest = 0;
}

int isConsistent(){
    // for(int i = 0;i < 4; i++) printf("%d\n",refstring[i]);
    return (refstring[0] == 0 && refstring[1] == 1 && refstring[2] == 1 && refstring[3] == 2) || 
            (refstring[0] == 1 && refstring[1] == 2 && refstring[2] == 0 && refstring[3] == 1);
}

mut_t lock;
long c1,c2,c,run = 1;

void *f1(void *lock){
    while(run){
        c1++;
        mutex_acquire((mut_t *)lock);
        c++;
        mutex_release((mut_t *)lock);
    }
}
void *f2(void *lock){
    while(run){
        c2++;
        mutex_acquire((mut_t *)lock);
        c++;
        mutex_release((mut_t *)lock);
    }
}

int testMutex(){
    log_info("Starting test with Mutex");
    mutex_init(&lock);
    thread t1,t2;
    thread_create(&t1,NULL,f1,(void *)&lock);
    thread_create(&t2,NULL,f2,(void *)&lock);
    sleep(2);
    run = 0;
    log_trace("\nValues after test are (c1 + c2)=%ld c=%ld\n",c1+c2,c);
    if(c1 + c2 - c > 2) printf(RED"Test failed\n"RESET);
    else printf(GREEN"Test passed\n"RESET);
    return 0;
}
void *f1spin(void *lock){
    while(run){
        c1++;
        spin_acquire((mut_t *)lock);
        c++;
        spin_release((mut_t *)lock);
    }
}
void *f2spin(void *lock){
    while(run){
        c2++;
        spin_acquire((mut_t *)lock);
        c++;
        spin_release((mut_t *)lock);
    }
}

int testSpin(){
    c1 = 0;
    c2 = 0;
    run = 1;
    c = 0;
    log_info("Starting test with Spinlocks");
    spin_init(&lock);
    thread t1,t2;
    thread_create(&t1,NULL,f1,(void *)&lock);
    thread_create(&t2,NULL,f2,(void *)&lock);
    sleep(2);
    run = 0;
    log_trace("\nValues after test are (c1 + c2)=%ld c=%ld\n",c1+c2,c);
    if(c1 + c2 - c > 2) printf(RED"Test failed\n"RESET);
    else printf(GREEN"Test passed\n"RESET);
    return 0;
}

int main(){
    setbuf(stdout, NULL);
    printf("tlib Synchronization Tests\n");
    int i = 1;
    // printf(GREEN"\nTesting with Locks\n"RESET);
    // while(1){
    //     testLocks(WITH_LOCKS);
    //     if(!isConsistent()) {
    //         log_error("Inconsistent values of global\nTest Failed\n");
    //         break;
    //     }
    //     sleep(1);
    //     log_trace("Consistent values of global in run %d",i);
    //     i++;
    //     if(i > 5) break;
    // }
    testMutex();
    testSpin();
    return 0;
}

