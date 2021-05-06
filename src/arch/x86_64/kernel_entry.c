#include "vga_text.h"
#include "multiboot2.h"
#include "x86.h"
#include "memory.h"

extern uint64_t p4_table;
extern uint64_t temp_table;

int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    uint64_t kernel_start, kernel_end, multiboot_start, multiboot_end;

    terminal_initialize();
    multiboot_init(magic, P2V(multiboot_addr), &kernel_start, &kernel_end, &multiboot_start, &multiboot_end) != 0 ? panic("check multiboot2 magic!\n") : 0;
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end);

    panic("OS SUSPENDED!\n");
}