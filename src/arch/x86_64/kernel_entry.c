#include "debug.h"
#include "vga_text.h"
#include "multiboot2.h"
#include "x86.h"
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

extern uint64_t p4_table;
extern uint64_t temp_table;

static int timer_intr = 0;

static char* gatsby_quote = "In my younger . . . years my father gave me some advice . . . Whenever you feel like criticizing any one . . . just remember that all the people in this world haven't had the advantages that you've had. \0";

void timer_interrupt(struct intr_frame *f) {
    if(gatsby_quote[timer_intr] == '\0') {
        timer_intr = 0;
    }
    printf("%c", gatsby_quote[timer_intr ++]);
}

int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    uint64_t kernel_start, kernel_end, multiboot_start, multiboot_end;
    terminal_initialize();
    printf("asdf");
    interrupt_init();
    multiboot_init(magic, multiboot_addr, &kernel_start, &kernel_end, &multiboot_start, &multiboot_end) != 0 ? panic("check multiboot2 magic!\n") : 0;
    debug_multiboot2(multiboot_addr);
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end, multiboot_addr);
    pic_init();
    keyboard_init();
    bind_interrupt_with_name(0x20, &timer_interrupt, "Timer");
    thread_init();

    intr_enable();

    PANIC("OS SUSPENDED!\n");
}