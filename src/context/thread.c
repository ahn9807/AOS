#include "thread.h"
#include "list.h"
#include "kmalloc.h"
#include "debug.h"
#include "intrinsic.h"
#include "memory.h"
#include "string.h"

#define THREAD_MAGIC 0xaaaeaa

static tid_t next_tid = 1;

struct {
    uint16_t len;
    struct idt_entry *addr;
} __attribute__ ((packed)) gdtr;

// Initialize the caller function to the thread (initd)
// And start Scheduler and push to the scheduler top
// Also initialize list and etc...
void thread_init() {
    struct thread_info *kernel_entry_th = current_thread();

    // Change kernel_entry.c to thread
    initialize_thread(kernel_entry_th, "initd");
    // Init scheduler

    // Push kernel_entry.c thread to scheduler

    // WarmUP
}

// Create new thread
tid_t thread_create(const char* name, thread_func* func, void * aux) {
    struct thread_info *th = kmalloc(sizeof(struct thread_info));

    ASSERT(func != NULL);
    ASSERT(name != NULL);


    th->magic = THREAD_MAGIC;
    th->status = THREAD_READY;
    th->tid = next_tid++;
}

static void initialize_thread(struct thread_info *th, const char *name) {
    memset(th, 0, sizeof(struct thread_info));
    strcpy(th->name, name);
}

// void thread_start();
// void thread_block();
// void thread_unblock(struct thread_info *);
// struct thread_info* thread_current();
// void thread_exit();
// void thread_yield();
// void do_iret(struct intr_frame*);