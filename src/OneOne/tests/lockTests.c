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
    return (refstring[0] == 0 && refstring[1] == 1 && refstring[2] == 1 && refstring[3] == 2) || (refstring[0] == 1 && refstring[1] == 2 && refstring[2] == 0 && refstring[3] == 1);
}

int main(){
    setbuf(stdout, NULL);
    printf("tlib Synchronization Tests\n\n");
    int i = 0;
    printf(GREEN"Testing Synchronization without locks.\n"RESET);
    while(1){
        testLocks(WITHOUT_LOCKS);
        if(!isConsistent()) {
            log_error("Inconsistent values of global");
            break;
        }
        sleep(1);
        log_trace("Consistent values of global in run %d",i);
        i++;
    }
    printf(GREEN"\nTesting with Locks\n"RESET);
    i = 0;
    while(1){
        testLocks(WITH_LOCKS);
        if(!isConsistent()) {
            log_error("Inconsistent values of global\nTest Failed\n");
            break;
        }
        sleep(1);
        log_trace("Consistent values of global in run %d",i);
        i++;
        if(i > 5) break;
    }
    return 0;
}