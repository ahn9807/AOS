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

size_t readdir(struct inode* inode, struct dentry** dentries_ret) {
    struct dentry *d1 = kmalloc(sizeof(struct dentry));
    struct dentry *d2 = kmalloc(sizeof(struct dentry));
    struct dentry *d3 = kmalloc(sizeof(struct dentry));
    d1->name = "a";
    d2->name = "b";
    d3->name = "c";
    dentries_ret = kmalloc(sizeof(struct dentry*) * 3);
    dentries_ret[0] = d1;
    dentries_ret[1] = d2;
    dentries_ret[2] = d2;
    return 3;
}

int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    uint64_t kernel_start, kernel_end, multiboot_start, multiboot_end;
    vga_init();
    interrupt_init();
    multiboot_init(magic, multiboot_addr, &kernel_start, &kernel_end, &multiboot_start, &multiboot_end) != 0 ? panic("check multiboot2 magic!\n") : 0;
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end, multiboot_addr);
    pic_init();
    bind_interrupt_with_name(0x20, &timer_interrupt, "Timer");
    thread_init();
    intr_enable();
    vfs_init();
    dev_init();
    ext2_init();
    char *dev_path = "/dev/disk0/foo/bar";
    inode_t *root_node = kmalloc(sizeof(inode_t));
    device_t *reg_dev = vfs_mountpoint(dev_path)->inode->device;
    vfs_mount(dev_path, root_node);
    printf("device %s\n", reg_dev == NULL ? "null" : reg_dev->name);
    struct dentry dir;
    temp_ls(root_node);
	if(vfs_readdir(root_node, 6, &dir) == 0) {
        PANIC("HALT");
    }
    file_t file = {
        .f_op = root_node->i_fop,
        .inode = dir.inode,
        .name = dir.name,
    };
	vfs_trunc(&file, 2 * 1024);
    temp_cat(&file);
    temp_ls(root_node);


    char bbuf[8];
    for(int i=0;i<8;i++) {
        bbuf[i] = 0b11110000;
    }
    struct bitmap *map = bitmap_create_from_buf(64, bbuf);
    bitmap_dump(map);

    // bitmap_dump(map);

    // Have to call explicitly. Cause without this,
    // rip goes to the end of the bootloader and
    // unrecover kernel panic. Also this changes the current kernel_entry
    // Function to the idle thread.
    // Rest of the powerup process in done by init.
    thread_run_idle();
}

void temp_ls(inode_t *dir_node) {
    struct dentry dir;
    int cur_offset = 0;
    while(vfs_readdir(dir_node, cur_offset, &dir) > 0) {
        printf("%s %d\n", dir.name, dir.inode->size);
        cur_offset++;
    }
    printf("\n");
}

void temp_cat(file_t *file) {
    printf("cat %s\n", file->name);
    char* buf = kmalloc(4096);

    vfs_read(file, buf, 0, 0);

    for(int i=0;i<64;i++) {
        printf("%c", buf[i]);
    }
    printf("\n");

    kfree(buf);
}