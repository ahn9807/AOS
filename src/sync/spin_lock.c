#include "spin_lock.h"
#include "lib/debug.h"
#include "interrupt.h"

// Lock spinlock
void spin_lock(spinlock_t *spl) {
    while(!__sync_bool_compare_and_swap(&(spl->locked), 0, 1)) asm("pause");
    __sync_synchronize();
}

// Unlock spinlock
void spin_unlock(spinlock_t *spl) {
    ASSERT(spl->locked == true);

    __sync_synchronize();

    // Atomically set spl->locked false
    asm volatile("movl $0, %0" : "+m" (spl->locked) : );
}

// Lock spinlock without acquiring spin lock
void spin_lock_init(spinlock_t *spl) {
    spl->locked = false;
}

// try lock spin lock else return false immediately
bool spin_trylock(spinlock_t *spl) {
    bool ret = __sync_bool_compare_and_swap(&(spl->locked), 0, 1);
    __sync_synchronize();
    return ret;
}

// Lock spin lock with irq disabled
void spin_lock_irq(spinlock_t *spl) {
    intr_disable();
    spin_lock(spl);
}

// Unlock spin lock with irq enabled
void spin_unlock_irq(spinlock_t *spl) {
    spin_unlock(spl);
    intr_enable();
}

// try lock spin lock else return false immediately with intr enabled
bool spin_trylock_irq(spinlock_t *spl) {
    intr_disable();
    bool ret = spin_trylock(spl);
    intr_enable();
    return ret;
}