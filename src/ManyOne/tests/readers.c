#include <stdio.h>
#include "../thread.h"
spin_t lock;
spin_t rwlock;
int readers = 0;
void f1()
{
    spin_acquire(&lock);
    readers += 1;
    if (readers == 1)
    {
        printf("Acquired writers lock\n");
        spin_acquire(&rwlock);
    }
    spin_release(&lock);
    printf("Reader process in\n");
    spin_acquire(&lock);
    readers -= 1;
    if (readers == 0)
    {
        spin_release(&rwlock);
    }
    spin_release(&lock);
}

void f2()
{
    spin_acquire(&rwlock);
    printf("Writer process in\n");
    spin_release(&rwlock);
}
int main()
{
    spin_init(&lock);
    spin_init(&rwlock);
    thread t1, t2;
    thread_create(&t1, NULL, f1, NULL);
    thread_create(&t2, NULL, f2, NULL);
    thread_join(t2, NULL);
    thread_join(t1, NULL);
    return 0;
}