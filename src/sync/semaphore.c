#include "semaphore.h"
#include "debug.h"
#include "list.h"
#include "interrupt.h"

void sema_init(struct semaphore *sema, unsigned int value) {
    ASSERT(sema != NULL);

    sema->value = value;
    list_init(&sema->waiters);
}
void sema_down(struct semaphore *sema) {
    ASSERT(sema != NULL);
    ASSERT(!intr_context());

    intr_disable();
    // spin lock
}

bool sema_try_down(struct semaphore *);
void sema_up(struct semaphore *);
void sema_self_tes(void);