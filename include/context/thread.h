#pragma once

#include "lib/list.h"
#include "lib/time.h"
#include "interrupt.h"
#include "intrinsic.h"
#include "process.h"
#include "vmem.h"

#define thread_current() ((struct thread_info *)((rrsp()) & ~((uint64_t)((1 << 12) - 1))))

typedef uint32_t tid_t;

enum thread_status
{
    THREAD_EMBRYO,
    THREAD_READY,
    THREAD_RUNNUNG,
    THREAD_SLEEP,
    THREAD_BLOCKED,
    THREAD_EXITED,
};

struct thread_info
{
    tid_t tid;
    enum thread_status status;
    char name[32];
    uint64_t p4;
    uintptr_t tls_base;
    size_t tls_size;
    struct intr_frame thread_frame;
    // list elem for scheduler
    struct list_head list;
    // list elem for ''all'' thread list
    struct list_head allelem;
    uint64_t magic;
    process_info_t *owner;

    struct mm_struct *mm;

    // MLFQ Priority scheduler
    time_t sleep_ticks;
    int priority;
    // This value is used in scheduler to manage MLFQ
    int nice;
    int recent_cpu;
};

typedef void thread_func(void *aux);
typedef void thread_action_func(struct thread_info *t, void *aux);

void thread_init();
void thread_validate();
tid_t thread_create(const char *name, thread_func *, void *);
void thread_start();
void thread_block();
void thread_unblock(struct thread_info *);
void thread_exit();
void thread_yield();
void do_iret(struct intr_frame *);
struct thread_info *thread_current_s();
void thread_launch(struct thread_info *th);

static void initialize_thread(struct thread_info *th, const char *name);
void thread_run_idle();

void thread_ssleep(uint64_t sec);
void thread_msleep(uint64_t ms);
void thread_usleep(uint64_t us);
void thread_nsleep(uint64_t ns);

void thread_set_priority(int priority);
int thread_get_priority(void);

void thread_foreach(thread_action_func *func, void *aux);