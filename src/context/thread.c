#include "thread.h"
#include "list.h"
#include "kmalloc.h"
#include "debug.h"
#include "intrinsic.h"
#include "memory.h"
#include "string.h"
#include "pmm.h"

#define THREAD_MAGIC 0xaaaeaa

extern uint64_t kernel_stack_top;

static tid_t next_tid = 1;

struct {
    uint16_t len;
    struct idt_entry *addr;
} __attribute__ ((packed)) gdtr;

// Initialize the caller function to the thread (initd)
// And start Scheduler and push to the scheduler top
// Also initialize list and etc...
void thread_init() {
    intr_disable();
    struct thread_info *kernel_entry_th = current_thread();

    initialize_thread(kernel_entry_th, "initd");

    kernel_entry_th->status = THREAD_RUNNUNG;
    kernel_entry_th->tid = next_tid++;

    intr_enable();
}

// Create new thread
tid_t thread_create(const char* name, thread_func* func, void * aux) {
    struct thread_info *th = P2V(pmm_alloc_pages(1));

    ASSERT(func != NULL);
    ASSERT(name != NULL);


    th->magic = THREAD_MAGIC;
    th->status = THREAD_READY;
    th->tid = next_tid++;
}

static void initialize_thread(struct thread_info *th, const char *name) {
    ASSERT(th != NULL);
    ASSERT(name != NULL);

    memset(th, 0, sizeof(struct thread_info));

    //This allocate stack pointer to the start of the given stack
    th->thread_frame.rsp = (uint64_t)th + PAGE_SIZE - sizeof(void *);
    strcpy(th->name, name);
    th->status = THREAD_EMBRYO;
    th->magic = THREAD_MAGIC;
}

// void thread_start();
// void thread_block();
// void thread_unblock(struct thread_info *);
// struct thread_info* thread_current();
// void thread_exit();
// void thread_yield();
// void do_iret(struct intr_frame*);