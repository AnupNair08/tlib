#include<stdatomic.h>
#include"thread.h"
#include<stdio.h>
int mutex_init(mut_t* lock){
    *lock = (volatile mut_t)ATOMIC_FLAG_INIT;
}

int mutex_acquire(mut_t *lock){
    while(atomic_flag_test_and_set(lock));
}

int mutex_release(mut_t *lock){
    atomic_flag_clear(lock);
}

// int main(){
//     mut_t lock;
//     mutex_init(&lock);
//     mutex_acquire(&lock);
//     puts("Critcial Section");
//     mutex_release(&lock);
//     return 1;
// }