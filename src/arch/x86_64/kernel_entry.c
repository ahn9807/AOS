#include "debug.h"
#include "vga_text.h"
#include "multiboot2.h"
#include "memory.h"
#include "interrupt.h"
#include "pic8259.h"
#include "port.h"
#include "keyboard.h"
#include "intrinsic.h"
#include "spin_lock.h"
#include "kmalloc.h"
#include "pmm.h"
#include "thread.h"
#include "sched.h"

extern uint64_t p4_table;
extern uint64_t temp_table;

static int timer_intr = 0;

static char* gatsby_quote = "In my younger . . . years my father gave me some advice . . . Whenever you feel like criticizing any one . . . just remember that all the people in this world haven't had the advantages that you've had. \0";

enum irq_handler_result timer_interrupt(struct intr_frame *f) {
    // if(gatsby_quote[timer_intr] == '\0') {
    //     timer_intr = 0;
    // }
    // printf("%c", gatsby_quote[timer_intr ++]);
    if(timer_intr ++ == 10) {
        timer_intr = 0;
        return YIELD_ON_RETURN;
    }
    return OK;
}

void temp_thread() {
    int tick = 0;
    while(1) {
        if(tick++ % 100000000 == 0)
            printf("Thread #%d, tick = %d\n", thread_current_s()->tid, tick / 10000);
    }
}

void temp_thread2() {
    int tick = 0;
    while(1) {
        if(tick++ % 100000000 == 0)
            printf("Thread #%d, tick = %d\n", thread_current_s()->tid, tick / 10000);
    }
}

void temp_thread3() {

}

int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    uint64_t kernel_start, kernel_end, multiboot_start, multiboot_end;
    vga_init();
    interrupt_init();
    multiboot_init(magic, multiboot_addr, &kernel_start, &kernel_end, &multiboot_start, &multiboot_end) != 0 ? panic("check multiboot2 magic!\n") : 0;
    debug_multiboot2(multiboot_addr);
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end, multiboot_addr);
    pic_init();
    keyboard_init();
    bind_interrupt_with_name(0x20, &timer_interrupt, "Timer");
    thread_init();
    intr_enable();

    thread_create("temp th", &temp_thread, NULL);
    thread_create("temp th2", &temp_thread2, NULL);
    thread_create("temp th3", &temp_thread3, NULL);

    printf("cur thread name: %s\n", thread_current()->name);

    PANIC("OS SUSPENDED!\n");
}