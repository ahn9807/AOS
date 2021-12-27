#pragma once

#include <stddef.h>
#include <stdint.h>
#include "interrupt.h"
#include "intrinsic.h"
#include "list.h"
#include "process.h"

#define thread_current() ((struct thread_info*)((rrsp()) & ~((uint64_t)((1<<12) - 1))))

typedef uint32_t tid_t;

enum thread_status {
    THREAD_EMBRYO,
    THREAD_READY,
    THREAD_RUNNUNG,
    THREAD_SLEEP,
    THREAD_BLOCKED,
    THREAD_EXITED,
};

struct thread_info {
    tid_t tid;
    enum thread_status status;
    char name[32];
    uint64_t p4;
    uintptr_t tls_base;
    size_t tls_size;
    struct intr_frame thread_frame;
    struct list_elem elem;
    uint64_t magic;
    process_info_t* owner;
};

typedef void thread_func (void *aux);

void thread_init();
void thread_validate();
tid_t thread_create(const char* name, thread_func*, void *);
void thread_start();
void thread_block();
void thread_unblock(struct thread_info *);
void thread_exit();
void thread_yield();
void do_iret(struct intr_frame*);
struct thread_info* thread_current_s();
void launch_thread(struct thread_info *th);

static void initialize_thread(struct thread_info *th, const char *name);
void thread_run_idle();