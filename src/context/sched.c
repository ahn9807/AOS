#include "sched.h"
#include "list.h"
#include "spin_lock.h"
#include "thread.h"
#include "debug.h"
#include "timer.h"
#include "interrupt.h"

struct list ready_list;
struct list sleep_list;
struct list blocked_list;
struct list dying_list;

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
    list_init(&ready_list);
    list_init(&sleep_list);
    list_init(&blocked_list);
    list_init(&dying_list);
}

static bool thread_order_priority(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED)
{
    return list_entry(a, struct thread_info, elem)->priority <= list_entry(b, struct thread_info, elem)->priority;
}

// Pick next thread to run
struct thread_info *sched_next()
{
    spin_lock(&sched_lock);
    struct thread_info *th_next = list_entry(list_pop_back(&ready_list), struct thread_info, elem);
    spin_unlock(&sched_lock);
    // Have to change this to handle idle thread
    return th_next;
}

// Push thread into the sceduler list
void sched_push_ready(struct thread_info *th)
{
    spin_lock(&sched_lock);
    list_insert_ordered(&ready_list, &(th->elem), thread_order_priority, 0);
    spin_unlock(&sched_lock);
}

// Reschedule the current thread
// Priority schedulering with round robbin
void sched_do()
{
    if (intr_get_level() == INTR_ON)
    {
        PANIC("Scheduling while atomic");
        debug_backtrace();
    }

    struct thread_info *cur_thread = thread_current_s();
    if (cur_thread->status == THREAD_RUNNUNG)
    {
        cur_thread->status = THREAD_READY;
        sched_push_ready(cur_thread);
    }

    struct thread_info *next_thread = sched_next();
    if (next_thread == idle_thread)
    {
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
    ticks++;

    enum intr_level old_level;

    old_level = intr_disable();

    while (!list_empty(&sleep_list))
    {
        struct thread_info *t = list_entry(list_front(&sleep_list), struct thread_info, elem);
        if (t->sleep_ticks <= timer_ticks())
        {
            list_pop_front(&sleep_list);
            thread_unblock(t);
            continue;
        }

        break;
    }

    struct thread_info *cur_thread = thread_current_s();

    // MLFQ
    // Increase the current thread's cpu usage by 1
    if (idle_thread != thread_current_s())
    {
        cur_thread->recent_cpu = ADD_REAL_INT(cur_thread->recent_cpu, 1);
    }

    // Re-calculate the load average and current cpu's priority
    if (timer_ticks() % SCHED_TIMER_MLFQ_INTVAL == 0)
    {
        int ready_thread_size = (int)list_size(&ready_list) + (cur_thread != idle_thread ? 1 : 0);
        load_average = ADD_REAL(MUL_REAL(DIV_REAL(59, 60), load_average), MUL_REAL_INT(DIV_REAL(1, 60), ready_thread_size));

        struct list_elem *e;

        // Recalculate recent cpu
        thread_foreach(mlfqs_recent_cpu, NULL);
        // Recalculate priority
        thread_foreach(mlfqs_priority, NULL);
    }

    if (timer_ticks() % SCHED_TIMER_TICK_INTVAL == 0)
    {
        mlfqs_priority(cur_thread, NULL);
    }

    intr_set_level(old_level);

    if (!list_empty(&ready_list) && cur_thread->priority < list_entry(list_front(&ready_list), struct thread_info, elem)->priority)
        return YIELD_ON_RETURN;
    else if (ticks % SCHED_TIMER_TICK_INTVAL == 0)
        return YIELD_ON_RETURN;

    return OK;
}

static bool thread_order_sleep(const struct list_elem *A,
                               const struct list_elem *B, void *aux UNUSED)
{
    const struct thread_info *thread_a = list_entry(A, struct thread_info, elem);
    const struct thread_info *thread_b = list_entry(B, struct thread_info, elem);
    return thread_a->sleep_ticks < thread_b->sleep_ticks;
}

// Sleep the current thread with sec and ns
void sched_sleep(timespec_t tm)
{
    time_t ticks = (tm.tv_sec * 1000000000 + tm.tv_nsec) * TIMER_FREQ / 1000000000;

    ASSERT(intr_get_level() == INTR_ON);
    if (ticks >= 0)
    {
        sleep_ticks(ticks);
    }
    else
    {
        sleep_busy(ticks);
    }
}

// Sleep the current thread with ticks
static void sleep_ticks(time_t ticks)
{
    struct thread_info *cur = thread_current();
    time_t start = timer_ticks();
    enum intr_level old_level;

    ASSERT(!intr_context());
    old_level = intr_disable();

    // for idle thread blocking must be ignored
    if (cur != idle_thread)
    {
        cur->sleep_ticks = ticks + start;
        list_insert_ordered(&sleep_list, &cur->elem, thread_order_sleep, 0);
        thread_block();
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

    if (!list_empty(&ready_list) && cur_thread->priority < list_entry(list_front(&ready_list), struct thread_info, elem)->priority)
    {
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
    if (idle_thread != t)
    {
        t->recent_cpu = ADD_REAL_INT(MUL_REAL(DIV_REAL(MUL_REAL_INT(load_average, 2), ADD_REAL_INT(MUL_REAL_INT(load_average, 2), 1)), t->recent_cpu), t->nice);
    }
}

static void mlfqs_priority(struct thread_info *t, void *aux UNUSED)
{
    if (idle_thread != t)
        t->priority = PRIORITY_MAX - REAL_TO_INT_ROUND((t->recent_cpu / 4)) - t->nice * 2;

    if (t->priority > PRIORITY_MAX)
    {
        t->priority = PRIORITY_MAX;
    }
    else if (t->priority < PRIORITY_MIN)
    {
        t->priority = PRIORITY_MIN;
    }
}