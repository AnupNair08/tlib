/**
 * @file readers.c
 * @author Hrishikesh Athalaye & Anup Nair
 * @brief Readers writers synchronization problem
 * @version 0.1
 * @date 2021-04-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdatomic.h>
#include "tests.h"
#include "../thread.h"

mutex_t lock;
mutex_t rwlock;
spin_t printlock;

int readers = 0;
int numReadersIn = 0;
int numWritersIn = 0;

void f1()
{
    mutex_acquire(&lock);
    readers += 1;
    if (readers == 1)
    {
        mutex_acquire(&rwlock);
    }
    mutex_release(&lock);

    safeprintf(&printlock, "Reader process in\n");
    atomic_fetch_add(&numReadersIn, 1);
    mutex_acquire(&lock);
    readers -= 1;
    if (readers == 0)
    {
        mutex_release(&rwlock);
    }
    mutex_release(&lock);
    atomic_fetch_sub(&numReadersIn, 1);
    safeprintf(&printlock, "Reader process out\n");
}

void f2()
{
    mutex_acquire(&rwlock);
    atomic_fetch_add(&numWritersIn, 1);
    safeprintf(&printlock, "Writer process in\n");
    mutex_release(&rwlock);
    atomic_fetch_sub(&numWritersIn, 1);
    safeprintf(&printlock, "Writers process out\n");
}
int main()
{
    mutex_init(&lock);
    mutex_init(&rwlock);
    spin_init(&printlock);
    atomic_init(&numReadersIn, 0);
    atomic_init(&numWritersIn, 0);
    thread t1, t2;
    thread readers[5];
    thread writers[5];
    for (int i = 0; i < 5; i++)
    {
        thread_create(&readers[i], NULL, f1, NULL);
        thread_create(&writers[i], NULL, f2, NULL);
    }
    for (int i = 0; i < 5; i++)
    {
        thread_join(writers[i], NULL);
        thread_join(readers[i], NULL);
    }
    // printf("%d %d\n\n", numReadersIn, numWritersIn);
    return 0;
}