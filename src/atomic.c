#include <stdatomic.h>
typedef volatile int spin_t;
spin_t *lock;
int spin_acquire(spin_t *lock)
{
    while (atomic_flag_test_and_set(lock))
    {
        // log_trace("Waiting for lock");
    };
    return 0;
}

int spin_release(spin_t *lock)
{
    atomic_flag_clear(lock);
    return 0;
}
int main()
{
    *lock = (volatile spin_t)ATOMIC_FLAG_INIT;
    spin_acquire(lock);
    return 0;
}
