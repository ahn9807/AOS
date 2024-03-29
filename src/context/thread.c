#include "thread.h"
#include "lib/list.h"
#include "lib/list.h"
#include "kmalloc.h"
#include "lib/debug.h"
#include "intrinsic.h"
#include "memory.h"
#include "string.h"
#include "pmm.h"
#include "layout.h"
#include "cpu_flags.h"
#include "sched.h"
#include "lib/time.h"
#include "vmem.h"

#define THREAD_MAGIC 0xdeaddead

extern uint64_t kernel_stack_top;
LIST_HEAD(thread_list);

static tid_t next_tid = 1;
static void thread_entry(thread_func *, void *aux);

struct {
    uint16_t len;
    struct idt_entry *addr;
} __attribute__((packed)) gdtr;

// Initialize the caller function to the thread (initd)
// And start Scheduler and push to the scheduler top
// Also initialize list and etc...
void thread_init()
{
    intr_disable();
    struct thread_info *kernel_entry_th = thread_current();

    sched_init();
    initialize_thread(kernel_entry_th, "thread_start");

    kernel_entry_th->status = THREAD_RUNNUNG;
    kernel_entry_th->tid = next_tid++;
    kernel_entry_th->priority = 31;
    kernel_entry_th->nice = 0;

    intr_enable();
    thread_launch(kernel_entry_th);
}

// Create new thread
tid_t thread_create(const char *name, thread_func *func, void *aux)
{
    struct thread_info *th = (struct thread_info *)P2V(pmm_alloc_pages(1));

    ASSERT(func != NULL);
    ASSERT(name != NULL);
    ASSERT(th != NULL);

    initialize_thread(th, name);
    th->tid = next_tid++;

    th->thread_frame.rip = (uint64_t)thread_entry;
    th->thread_frame.reg.rdi = (uint64_t)func;
    th->thread_frame.reg.rsi = (uint64_t)aux;
    th->thread_frame.ds = SEL_KDSEG;
    th->thread_frame.es = SEL_KDSEG;
    th->thread_frame.ss = SEL_KDSEG;
    th->thread_frame.cs = SEL_KCSEG;
    th->thread_frame.eflags = FLAG_IF;
    th->magic = THREAD_MAGIC;
    th->status = THREAD_READY;

    sched_push_ready(th);
    list_add_tail(&th->allelem, &thread_list);

    return th->tid;
}

static void initialize_thread(struct thread_info *th, const char *name)
{
    ASSERT(th != NULL);
    ASSERT(name != NULL);

    memset(th, 0, sizeof(struct thread_info));

    // This allocate stack pointer to the start of the given stack
    th->thread_frame.rsp = (uint64_t)th + PAGE_SIZE - sizeof(void *);
    strcpy(th->name, name);
    th->status = THREAD_EMBRYO;
    th->magic = THREAD_MAGIC;
    th->nice = thread_current_s()->nice;
    th->priority = thread_current_s()->priority;
    th->mm = kmalloc(sizeof(struct mm_struct));
    mm_init(th->mm);
}

static void thread_entry(thread_func *func, void *aux)
{
    ASSERT(func != NULL);

    intr_enable();
    func(aux);
    thread_exit();
}

void thread_exit()
{
    // move thread to exit list
    // temp scheduler have to change!!!
    kfree(thread_current_s()->mm);
    mm_destory(thread_current_s()->mm);
    intr_disable();
    thread_current_s()->status = THREAD_EXITED;
    list_del_init(&thread_current_s()->allelem);
    sched_do();
    NOT_REACHED();
}

void thread_validate()
{
    if (thread_current()->magic != THREAD_MAGIC)
    {
        PANIC("Stack overflow detected!");
    }
}

void do_iret(struct intr_frame *tf)
{
    __asm __volatile(
        "movq %0, %%rsp\n"
        "movq 0(%%rsp),%%r15\n"
        "movq 8(%%rsp),%%r14\n"
        "movq 16(%%rsp),%%r13\n"
        "movq 24(%%rsp),%%r12\n"
        "movq 32(%%rsp),%%r11\n"
        "movq 40(%%rsp),%%r10\n"
        "movq 48(%%rsp),%%r9\n"
        "movq 56(%%rsp),%%r8\n"
        "movq 64(%%rsp),%%rsi\n"
        "movq 72(%%rsp),%%rdi\n"
        "movq 80(%%rsp),%%rbp\n"
        "movq 88(%%rsp),%%rdx\n"
        "movq 96(%%rsp),%%rcx\n"
        "movq 104(%%rsp),%%rbx\n"
        "movq 112(%%rsp),%%rax\n"
        "addq $120,%%rsp\n"
        "movw 8(%%rsp),%%ds\n"
        "movw (%%rsp),%%es\n"
        "addq $32, %%rsp\n"
        "iretq"
        :
        : "g"((uint64_t)tf)
        : "memory");
}

void thread_launch(struct thread_info *th)
{
    uint64_t current_thread = (uint64_t)(&thread_current()->thread_frame);
    uint64_t incomming_thread = (uint64_t)(&th->thread_frame);

    __asm __volatile(
        /* Store registers that will be used. */
        "push %%rax\n"
        "push %%rbx\n"
        "push %%rcx\n"
        /* Fetch input once */
        "movq %0, %%rax\n"
        "movq %1, %%rcx\n"
        "movq %%r15, 0(%%rax)\n"
        "movq %%r14, 8(%%rax)\n"
        "movq %%r13, 16(%%rax)\n"
        "movq %%r12, 24(%%rax)\n"
        "movq %%r11, 32(%%rax)\n"
        "movq %%r10, 40(%%rax)\n"
        "movq %%r9, 48(%%rax)\n"
        "movq %%r8, 56(%%rax)\n"
        "movq %%rsi, 64(%%rax)\n"
        "movq %%rdi, 72(%%rax)\n"
        "movq %%rbp, 80(%%rax)\n"
        "movq %%rdx, 88(%%rax)\n"
        "pop %%rbx\n" // Saved rcx
        "movq %%rbx, 96(%%rax)\n"
        "pop %%rbx\n" // Saved rbx
        "movq %%rbx, 104(%%rax)\n"
        "pop %%rbx\n" // Saved rax
        "movq %%rbx, 112(%%rax)\n"
        "addq $120, %%rax\n"
        "movw %%es, (%%rax)\n"
        "movw %%ds, 8(%%rax)\n"
        "addq $32, %%rax\n"
        "call __next\n" // read the current rip.
        "__next:\n"
        "pop %%rbx\n"
        "addq $(out_iret -  __next), %%rbx\n"
        "movq %%rbx, 0(%%rax)\n" // rip
        "movw %%cs, 8(%%rax)\n"  // cs
        "pushfq\n"
        "popq %%rbx\n"
        "mov %%rbx, 16(%%rax)\n" // eflags
        "mov %%rsp, 24(%%rax)\n" // rsp
        "movw %%ss, 32(%%rax)\n"
        "mov %%rcx, %%rdi\n"
        "call do_iret\n"
        "out_iret:\n"
        :
        : "g"(current_thread), "g"(incomming_thread)
        : "memory");
}

// convert current thread to the idle thread
void thread_run_idle()
{
    strcpy(thread_current_s()->name, "idle");
    thread_set_priority(-1);
    sched_set_idle(thread_current());
    while (1)
    {
        asm volatile("sti\n"
                     "hlt\n");
    }
}

void thread_block()
{
    thread_current()->status = THREAD_BLOCKED;
    enum intr_level prev_level = intr_disable();
    sched_do();
    intr_set_level(prev_level);
}

void thread_unblock(struct thread_info *th)
{
    ASSERT(th->status == THREAD_BLOCKED);

    enum intr_level prev_level = intr_disable();
    // push to the shceduler ready list
    th->status = THREAD_READY;
    sched_push_ready(th);
    intr_set_level(prev_level);
}

// Safely get current thread
struct thread_info *thread_current_s()
{
    ASSERT(thread_current()->magic == THREAD_MAGIC);
    return thread_current();
}

void thread_yield()
{
    ASSERT(!intr_context());

    enum intr_level prev_level = intr_disable();
    // schedule to next thread
    sched_do();
    intr_set_level(prev_level);
}

void thread_set_priority(int priority)
{
    enum intr_level old_level = intr_disable();
    thread_current_s()->priority = priority;
    intr_set_level(old_level);
}

int thread_get_priority(void)
{
    return thread_current_s()->priority;
}

void thread_ssleep(uint64_t sec)
{
    timespec_t tm;
    tm.tv_sec = sec;
    tm.tv_nsec = 0;
    sched_sleep(tm);
}

/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void thread_foreach(thread_action_func *func, void *aux)
{
    struct thread_info *th;
    ASSERT(intr_get_level() == INTR_OFF);

    list_for_each_entry(th, &thread_list, allelem) {
        func(th, aux);
    }
}

void thread_msleep(uint64_t ms)
{
    timespec_t tm;
    tm.tv_sec = (ms / 1000);
    tm.tv_nsec = (ms % 1000) * 1000000;
    sched_sleep(tm);
}

void thread_usleep(uint64_t us)
{
    timespec_t tm;
    tm.tv_sec = (us / 1000000);
    tm.tv_nsec = (us % 1000000) * 1000000000;
    sched_sleep(tm);
}

void thread_nsleep(uint64_t ns)
{
    timespec_t tm;
    tm.tv_sec = (ns / 1000000000);
    tm.tv_nsec = ns % 1000000000;
    sched_sleep(tm);
}
