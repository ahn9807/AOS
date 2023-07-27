#include "semaphore.h"
#include "interrupt.h"
#include "lib/debug.h"
#include "lib/list.h"
#include "thread.h"

// Initialize the semaphore
// You should not use sempahore at interrupt context
void sema_init(struct semaphore *sema, unsigned int value)
{
	ASSERT(sema != NULL);

	sema->value = value;
	INIT_LIST_HEAD(&sema->waiters);
}

// Down semaphore
// If value is 0, then wait until sema_up
void sema_down(struct semaphore *sema)
{
	ASSERT(sema != NULL);
	ASSERT(!intr_context());

	enum intr_level prev_level = intr_disable();

	while (sema->value == 0) {
		list_add_tail(&thread_current()->list, &sema->waiters);
		thread_block();
	}
	sema->value--;
	intr_set_level(prev_level);
}

// Try down semaphore.
// If value is 0, then return false
bool sema_try_down(struct semaphore *sema)
{
	ASSERT(sema != NULL);
	ASSERT(!intr_context());

	enum intr_level prev_level = intr_disable();
	bool success = false;

	if (sema->value > 0) {
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
void sema_up(struct semaphore *sema)
{
	ASSERT(sema != NULL);
	ASSERT(!intr_context());

	enum intr_level prev_level = intr_disable();
	bool success = false;

	if (!list_empty(&sema->waiters)) {
		thread_unblock(list_entry(&sema->waiters, struct thread_info, list));
	}

	sema->value++;
	intr_set_level(prev_level);
}