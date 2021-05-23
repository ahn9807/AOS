#pragma once

#include <stddef.h>
#include <stdint.h>
#include "interrupt.h"
#include "intrinsic.h"

#define current_thread() ((struct thread_info*)(rrsp() << 12))

typedef uint32_t tid_t;

enum thread_status {
    THREAD_RUNNUNG,
    THREAD_READY,
    THREAD_BLOCKED,
    THREAD_DYING,
};

struct thread_info {
    tid_t tid;
    enum thread_status status;
    char name[32];
    uint64_t *p4;
    struct intr_frame thread_frame;
    uint64_t magic;
};

typedef void thread_func (void *aux);

void thread_init();
tid_t thread_create(const char* name, thread_func*, void *);
void thread_start();
void thread_block();
void thread_unblock(struct thread_info *);
struct thread_info* thread_current();
void thread_exit();
void thread_yield();
void do_iret(struct intr_frame*);

static void initialize_thread(struct thread_info *th, const char *name);
