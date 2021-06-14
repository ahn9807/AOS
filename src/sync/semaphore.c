#include "semaphore.h"
#include "debug.h"
#include "list.h"
#include "interrupt.h"
#include "thread.h"

// Initialize the semaphore
// You should not use sempahore at interrupt context
void sema_init(struct semaphore *sema, unsigned int value) {
    ASSERT(sema != NULL);

    sema->value = value;
    list_init(&sema->waiters);
}

// Down semaphore
// If value is 0, then wait until sema_up
void sema_down(struct semaphore *sema) {
    ASSERT(sema != NULL);
    ASSERT(!intr_context());

    enum intr_level prev_level = intr_disable();

    while(sema->value == 0) {
        list_push_back(&sema->waiters, &thread_current()->elem);
        thread_block();
    }
    sema->value--;
    intr_set_level(prev_level);
}

// Try down semaphore.
// If value is 0, then return false
bool sema_try_down(struct semaphore *sema) {
    ASSERT(sema != NULL);
    ASSERT(!intr_context());

    enum intr_level prev_level = intr_disable();
    bool success = false;

    if(sema->value > 0) {
        sema->value--;
        success = true;
    } else {
        success = false;
    }
    intr_set_level(prev_level);

    return success;
}

// Up semaphore.
// This makes other lock holing thread unblocked.
void sema_up(struct semaphore *sema) {
    ASSERT(sema != NULL);
    ASSERT(!intr_context());

    enum intr_level prev_level = intr_disable();
    bool success = false;

    if(!list_empty(&sema->waiters)) {
        thread_unblock(list_entry(list_pop_front(&sema->waiters), struct thread_info, elem));
    }

    sema->value++;
    intr_set_level(prev_level);
}