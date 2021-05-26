#include "sched.h"
#include "list.h"
#include "spin_lock.h"
#include "thread.h"
#include "debug.h"

struct list read_list;
struct list runnung_list;
struct list sleep_list;
struct list blocked_list;
struct list dying_list;

static spinlock_t sched_lock;

// Initialize the scheduler
void sched_init() {
    list_init(&read_list);
    list_init(&runnung_list);
    list_init(&sleep_list);
    list_init(&blocked_list);
    list_init(&dying_list);
}

// Pick next thread to run
struct thread_info *sched_next() {
    spin_lock(&sched_lock);
    struct thread_info *th_next = list_entry(list_pop_front(&runnung_list),struct thread_info, elem);
    spin_unlock(&sched_lock);
    // Have to change this to handle idle thread
    return th_next;
}

// Push thread into the sceduler list
void sched_push(struct thread_info *th) {
    spin_lock(&sched_lock);
    list_push_back(&runnung_list, &(th->elem));
    spin_unlock(&sched_lock);
}

// Reschedule the current thread
void sched_do() {
    ASSERT(intr_get_level() == INTR_OFF);

    struct thread_info *cur_thread = thread_current_s();
    if(cur_thread->status == THREAD_RUNNUNG) {
        cur_thread->status = THREAD_READY;
        sched_push(cur_thread);
    }

    struct thread_info *next_thread = sched_next();
    next_thread->status = THREAD_RUNNUNG;

    intr_enable();
    launch_thread(next_thread);
}

// Periodic event on kernel timer tick
void sched_tick() {
    sched_do();
}