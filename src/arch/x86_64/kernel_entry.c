#include <time.h>
#include "debug.h"
#include "vga_text.h"
#include "multiboot2.h"
#include "memory.h"
#include "interrupt.h"
#include "pic8259.h"
#include "port.h"
#include "intrinsic.h"
#include "spin_lock.h"
#include "kmalloc.h"
#include "pmm.h"
#include "thread.h"
#include "sched.h"
#include "vfs.h"
#include "string.h"
#include "device.h"
#include "ext2.h"
#include "ata.h"
#include "bitmap.h"
#include "ext2.h"
#include "stat.h"
#include "semaphore.h"
#include "elf.h"
#include "tss.h"
#include "process.h"
#include "gdt.h"
#include "syscalls.h"
#include "cmos.h"
#include "acpi.h"
#include "apic.h"
#include "cpu.h"

extern uint64_t p4_table;
extern uint64_t temp_table;

static int timer_intr = 0;

static char* gatsby_quote = "In my younger . . . years my father gave me some advice . . . Whenever you feel like criticizing any one . . . just remember that all the people in this world haven't had the advantages that you've had. \0";

enum irq_handler_result timer_interrupt(struct intr_frame *f) {
    // if(gatsby_quote[timer_intr] == '\0') {
    //     timer_intr = 0;
    // }
    // printf("%c", gatsby_quote[timer_intr ++]);
    return YIELD_ON_RETURN;
}

void temp_thread() {
    uint64_t tick = 0;
    while(1) {
        if(tick++ % 100000000 == 0)
            printf("Thread #%d, tick = %d\n", thread_current_s()->tid, tick / 10000);
    }
}

void temp_thread2() {
    uint64_t tick = 0;
    while(1) {
        if(tick++ % 100000000 == 0)
            printf("Thread #%d, tick = %d\n", thread_current_s()->tid, tick / 10000);
    }
}

void exec() {
    if(process_exec("/prog_1")) {
        PANIC("EXEC FAILED");
    }
}


int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    uint64_t kernel_start, kernel_end, multiboot_start, multiboot_end;
    vga_init();
    multiboot_init(magic, multiboot_addr, &kernel_start, &kernel_end, &multiboot_start, &multiboot_end) != 0 ? panic("check multiboot2 magic!\n") : 0;
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end, multiboot_addr);
    cmos_init();
    acpi_init();
    apic_init();
    cpu_init();
    tss_init();
    gdt_init();
    pic_init();
    interrupt_init();
    bind_interrupt_with_name(0x20, &timer_interrupt, "Timer");
    thread_init();
    intr_enable();
    syscall_init();
    vfs_init();
    dev_init();
    ext2_init();

    char *dev_path = "/dev/disk0/";
    inode_t *root_node = kmalloc(sizeof(inode_t));
    vfs_mount(dev_path, root_node);
    vfs_bind("/", root_node);

    file_t file;

    // vfs_bind("/dev", vfs_mountpoint("dev"));
    // vfs_open_by_path("/dev/", &file);
    // temp_ls(root_node);

    thread_create("exec", &exec, NULL);
    // Have to call explicitly. Cause without this,
    // rip goes to the end of the bootloader and
    // unrecover kernel panic. Also this changes the current kernel_entry
    // Function to the idle thread.
    // Rest of the powerup process in done by init.
    thread_run_idle();
}

int ap_main();

void temp_ls(inode_t *dir_node) {
    if(dir_node == NULL) {
        printf("ls: ?\n");
        return;
    }
    struct dentry dir;
    int cur_offset = 0;
    while(vfs_readdir(dir_node, cur_offset, &dir) > 0) {
        printf("%s %d ", dir.name, dir.inode->size);
        cur_offset++;
    }
    printf("\n");
}

void temp_cat(file_t *file) {
    printf("cat %s\n", file->name);
    if(file->name == NULL) {
        printf("cat: ?\n");
        return;
    }
    char* buf = kmalloc(4096);

    for(int page = 0; page < vfs_get_size(file); page += 4096) {
        int read_size = vfs_read(file, buf, 4096);
        for(int i=0;i<read_size;i++) {
            printf("%c", buf[i]);
        }
    }
    printf("\n");

    kfree(buf);
}