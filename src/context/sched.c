#include "sched.h"
#include "interrupt.h"
#include "lib/debug.h"
#include "lib/list.h"
#include "spin_lock.h"
#include "thread.h"
#include "timer.h"

LIST_HEAD(ready_list);
LIST_HEAD(sleep_list);
LIST_HEAD(blocked_list);
LIST_HEAD(dying_list);

static size_t ready_list_size;

static spinlock_t sched_lock;
static int load_average = 0;
static struct thread_info *idle_thread;

static void sleep_ticks(time_t ticks);
static void sleep_busy(time_t ticks);
static void mlfqs_priority(struct thread_info *t, void *);
static void mlfqs_recent_cpu(struct thread_info *t, void *);

// Initialize the scheduler
void sched_init()
{
	spin_lock_init(&sched_lock);
	ready_list_size = 0;
}

// Pick next thread to run
struct thread_info *sched_next()
{
	spin_lock(&sched_lock);
	if (list_empty(&ready_list)) {
		spin_unlock(&sched_lock);
		return thread_current_s();
	}
	struct thread_info *th_next = list_last_entry(&ready_list, struct thread_info, list);
	ready_list_size--;
	list_del_init(&th_next->list);
	spin_unlock(&sched_lock);
	// Have to change this to handle idle thread
	return th_next;
}

// Push thread into the sceduler list
void sched_push_ready(struct thread_info *th)
{
	struct thread_info *cur;

	spin_lock(&sched_lock);

	if (list_empty(&ready_list))
		list_add(&th->list, &ready_list);
	else {
		list_for_each_entry (cur, &ready_list, list) {
			if (th->priority >= cur->priority) {
				list_add(&th->list, &ready_list);
				ready_list_size++;
				break;
			}
		}
	}
	spin_unlock(&sched_lock);
}

// Reschedule the current thread
// Priority schedulering with round robbin
void sched_do()
{
	if (intr_get_level() == INTR_ON) {
		PANIC("Scheduling while atomic");
		debug_backtrace();
	}

	struct thread_info *cur_thread = thread_current_s();
	if (cur_thread->status == THREAD_RUNNUNG) {
		cur_thread->status = THREAD_READY;
		sched_push_ready(cur_thread);
	}

	struct thread_info *next_thread = sched_next();
	if (next_thread == idle_thread) {
		sched_push_ready(idle_thread);
		next_thread = sched_next();
	}
	next_thread->status = THREAD_RUNNUNG;

	intr_enable();
	thread_launch(next_thread);
}

// Periodic event on kernel timer tick
enum irq_handler_result sched_tick()
{
	static time_t ticks = 0;
	struct thread_info *cur;
	struct thread_info *temp;
	ticks++;

	enum intr_level old_level;

	old_level = intr_disable();

	list_for_each_entry_safe (cur, temp, &sleep_list, list) {
		if (cur->sleep_ticks <= timer_ticks()) {
			list_del(&cur->list);
			thread_unblock(cur);
			break;
		}
	}

	struct thread_info *cur_thread = thread_current_s();

	// MLFQ
	// Increase the current thread's cpu usage by 1
	if (idle_thread != thread_current_s()) {
		cur_thread->recent_cpu = ADD_REAL_INT(cur_thread->recent_cpu, 1);
	}

	// Re-calculate the load average and current cpu's priority
	if (timer_ticks() % SCHED_TIMER_MLFQ_INTVAL == 0) {
		load_average = ADD_REAL(MUL_REAL(DIV_REAL(59, 60), load_average),
					MUL_REAL_INT(DIV_REAL(1, 60), ready_list_size));

		// Recalculate recent cpu
		thread_foreach(mlfqs_recent_cpu, NULL);
		// Recalculate priority
		thread_foreach(mlfqs_priority, NULL);
	}

	if (timer_ticks() % SCHED_TIMER_TICK_INTVAL == 0) {
		mlfqs_priority(cur_thread, NULL);
	}

	intr_set_level(old_level);

	if (!list_empty(&ready_list) &&
	    cur_thread->priority < list_entry(&ready_list, struct thread_info, list)->priority)
		return YIELD_ON_RETURN;
	else if (ticks % SCHED_TIMER_TICK_INTVAL == 0)
		return YIELD_ON_RETURN;

	return OK;
}

// Sleep the current thread with sec and ns
void sched_sleep(timespec_t tm)
{
	time_t ticks = (tm.tv_sec * 1000000000 + tm.tv_nsec) * TIMER_FREQ / 1000000000;

	ASSERT(intr_get_level() == INTR_ON);
	if (ticks >= 0) {
		sleep_ticks(ticks);
	} else {
		sleep_busy(ticks);
	}
}

// Sleep the current thread with ticks
static void sleep_ticks(time_t ticks)
{
	struct thread_info *cur;
	struct thread_info *th = thread_current();
	time_t start = timer_ticks();
	enum intr_level old_level;

	ASSERT(!intr_context());
	old_level = intr_disable();

	// for idle thread blocking must be ignored
	if (th != idle_thread) {
		th->sleep_ticks = ticks + start;
		if (list_empty(&sleep_list))
			list_add(&th->list, &sleep_list);
		else {
			list_for_each_entry (cur, &sleep_list, list) {
				if (th->sleep_ticks < cur->sleep_ticks) {
					list_add(&th->list, &cur->list);
					ready_list_size++;
					thread_block();
					break;
				}
			}
		}
	}

	intr_set_level(old_level);
}

static void sleep_busy(time_t ticks)
{
	// Have to implement
	return;
}

void sched_set_idle(struct thread_info *idle)
{
	idle_thread = idle;
}

void sched_set_nice(int nice)
{
	intr_disable();

	struct thread_info *cur_thread = thread_current_s();
	cur_thread->nice = nice;
	mlfqs_priority(cur_thread, NULL);

	if (!list_empty(&ready_list) &&
	    cur_thread->priority < list_entry(&ready_list, struct thread_info, list)->priority) {
		thread_yield();
	}

	intr_enable();
}

int sched_get_nice()
{
	return thread_current_s()->nice;
}

static void mlfqs_recent_cpu(struct thread_info *t, void *aux UNUSED)
{
	if (idle_thread != t) {
		t->recent_cpu = ADD_REAL_INT(MUL_REAL(DIV_REAL(MUL_REAL_INT(load_average, 2),
							       ADD_REAL_INT(MUL_REAL_INT(load_average, 2), 1)),
						      t->recent_cpu),
					     t->nice);
	}
}

static void mlfqs_priority(struct thread_info *t, void *aux UNUSED)
{
	if (idle_thread != t)
		t->priority = PRIORITY_MAX - REAL_TO_INT_ROUND((t->recent_cpu / 4)) - t->nice * 2;

	if (t->priority > PRIORITY_MAX) {
		t->priority = PRIORITY_MAX;
	} else if (t->priority < PRIORITY_MIN) {
		t->priority = PRIORITY_MIN;
	}
}