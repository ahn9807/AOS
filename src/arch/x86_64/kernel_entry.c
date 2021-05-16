#include "vga_text.h"
#include "multiboot2.h"
#include "x86.h"
#include "memory.h"
#include "interrupt.h"
#include "pic8259.h"
#include "port.h"
#include "keyboard.h"

extern uint64_t p4_table;
extern uint64_t temp_table;

void timer_interrupt(struct intr_frame *f) {
    pic_end_of_interrupt(f->vec_no);
}

int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    uint64_t kernel_start, kernel_end, multiboot_start, multiboot_end;

    terminal_initialize();
    multiboot_init(magic, P2V(multiboot_addr), &kernel_start, &kernel_end, &multiboot_start, &multiboot_end) != 0 ? panic("check multiboot2 magic!\n") : 0;
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end);
    interrupt_init();
    pic_init();
    keyboard_init();

    bind_interrupt_with_name(0x20, &timer_interrupt, "Timer");

    enable_interrupt();

    panic("OS SUSPENDED!\n");
}