#include<stdio.h>
#include<unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "thread.h"
#endif
#include "tests.h"
#include "attributetypes.h"
#define WITH_LOCKS 1
#define WITHOUT_LOCKS 0


int mutexTest = 0;
short refstring[4];

void routine1(void *l){
    if(l) mutex_acquire((mut_t *)l);
    printf("Routine 1 Before: %d\n",mutexTest);
    refstring[0] = mutexTest;
    mutexTest++;
    printf("Routine 1 After: %d\n",mutexTest);
    refstring[1] = mutexTest;
    if(l) mutex_release((mut_t *)l);
}

void routine2(void *l){
    if(l) mutex_acquire((mut_t *)l);
    printf("Routine 2 Before: %d\n",mutexTest);
    refstring[2] = mutexTest;
    mutexTest++;
    printf("Routine 2 After: %d\n",mutexTest);
    refstring[3] = mutexTest;
    if(l) mutex_release((mut_t *)l);
}

void testLocks(int type){
    thread t1,t2;
    mut_t lock;
    mut_t *temp = NULL;
    if(type){
        mutex_init(&lock);
        temp = &lock;
    } 
    create(&t1,NULL,routine1, (void *)temp,0);
    create(&t2,NULL,routine2, (void *)temp,0);
    thread_join(t1,NULL);
    thread_join(t2,NULL);

    printf("\nValue of global is %d\n",mutexTest);
    mutexTest = 0;
}

int isConsistent(){
    return refstring[0] == 0 && refstring[1] == 1 && refstring[2] == 1 && refstring[3] == 2;
}

int main(){
    printf("tlib Synchronization Tests\n\n");
    int i = 0;
    printf(GREEN"Testing Synchronization without locks.\n"RESET);
    while(1){
        testLocks(WITHOUT_LOCKS);
        if(!isConsistent()) {
            printf(RED"Inconsistent values of global\n"RESET);
            break;
        }
        sleep(1);
        printf("Consistent values of global in run %d\n",i);
        i++;
    }
    printf(GREEN"\nTesting with Locks\n"RESET);
    while(1){
        testLocks(WITH_LOCKS);
        if(!isConsistent()) {
            printf(RED"Inconsistent values of global\nTest Failed\n"RESET);
            break;
        }
        sleep(1);
        printf("Consistent values of global in run %d\n",i);
        i++;
        if(i > 5) break;
    }
    return 0;
}