#include <stdio.h>
#include <unistd.h>
#include "tests.h"
#include "../attributetypes.h"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "../log.h"
#include "../thread.h"

void func(void *i){
    printf("In thread %d\n", *(int*)i);
    fflush(stdout);
    // sleep(*(int *)i);
    return;
}

int main(){
    int j[4] = {1,2,3,4};
    thread t[4];
    for(int i = 0; i < 4; i++){
        thread_create(&t[i],NULL,func,(void *)&j[i]);
    }
    for(int i = 0; i < 4; i++){
        thread_join(t[i], NULL);
    }
    log_info("Main exiting");
    return 0;
}