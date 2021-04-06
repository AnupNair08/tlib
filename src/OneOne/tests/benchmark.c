#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
    #include<tlib.h>
#else
    #include "../thread.h"
#endif
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "tests.h"
#include "../log.h"
#define SZ 1000000
#define printarr \
    for(short it = 0 ; it < SZ; it++) \
        printf("%d\n",arr[it]); \

typedef struct argumets {
    long *arr; 
    long low;
    long high;
} arguments;

long sum = 0;
spin_t lock;
void compute(void *arg){
    arguments *t = (arguments *)arg;
    for(long i = t->low ; i < t->high;i++){
        spin_acquire(&lock);
        sum += t->arr[i];
        spin_release(&lock);
    }
}


void sum_list(long *arr){
    spin_init(&lock);
    
    arguments a1,a2;
    a1.arr = arr, a2.arr = arr;
    a1.low = 0, a1.high = SZ/2;
    a2.low = SZ/2, a2.high = SZ;
    // printarr;
    thread t1, t2;
    thread_create(&t1,NULL,compute,(void *)&a1);
    thread_create(&t1,NULL,compute,(void *)&a2);
    thread_join(t1,NULL);
    thread_join(t2,NULL);
    log_info("Value of sum is %ld\n",sum);
}


int main(){
    long arr[SZ];
    for(long i = 0 ; i < SZ; i++){
        arr[i] = rand() % SZ/2;
    }
    clock_t start = clock();
    sum_list(arr);
    clock_t end = clock();
    printf("Elapsed: %f ms\n", 1000 * (double)(end - start) / CLOCKS_PER_SEC);
    clock_t start1 = clock();
    sum = 0;
    for(long i = 0 ; i < SZ; i++) sum += arr[i];
    log_info("Value of sum is %ld\n",sum);
    clock_t end1 = clock();
    printf("Elapsed: %f ms\n", 1000 * (double)(end1 - start1) / CLOCKS_PER_SEC);

    return 0;
}