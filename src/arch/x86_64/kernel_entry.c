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

extern uint64_t p4_table;
extern uint64_t temp_table;

void timer_interrupt(struct intr_frame *f) {
    printf(".");
}

int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    uint64_t kernel_start, kernel_end, multiboot_start, multiboot_end;
    terminal_initialize();
    interrupt_init();
    multiboot_init(magic, multiboot_addr, &kernel_start, &kernel_end, &multiboot_start, &multiboot_end) != 0 ? panic("check multiboot2 magic!\n") : 0;
    debug_multiboot2(multiboot_addr);
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end, multiboot_addr);
    pic_init();
    keyboard_init();
    bind_interrupt_with_name(0x20, &timer_interrupt, "Timer");

    printf("pmm: 0x%x\n", pmm_alloc());
    printf("pmm: 0x%x\n", pmm_alloc());
    printf("pmm: 0x%x\n", pmm_alloc());
    printf("pmm: 0x%x\n", pmm_alloc());

    uint64_t* temp = pmm_alloc();

    temp[0] = 123;

    printf("%d\n", temp[0]);

    printf("0x%x\n", kmalloc(1));
    printf("0x%x\n", kmalloc(1));
    printf("0x%x\n", kmalloc(1));
    printf("0x%x\n", kmalloc(1));
    printf("0x%x\n", kmalloc(1));
    printf("0x%x\n", kmalloc(1));

    intr_enable();

    PANIC("OS SUSPENDED!\n");
}