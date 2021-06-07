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
    debug_multiboot2(multiboot_addr);
    memory_init(kernel_start, kernel_end, multiboot_start, multiboot_end, multiboot_addr);
    pic_init();
    bind_interrupt_with_name(0x20, &timer_interrupt, "Timer");
    thread_init();
    intr_enable();
    vfs_init();
    dev_init();
    struct inode in;
    in.refcount = 123123;
    vfs_bind("/a/b/c/", &in);
    printf("pa: %s\n", path_absolute("/a/b/c", "../../../../../a/b/c/.."));
    struct inode *out = kmalloc(sizeof(struct inode));
    int index = 0;
    // while(1) {
    //     strdup("ASD");
    //     printf(".");
    // }
    printf("iop size: %d\n", sizeof(struct inode_operations));
    in.i_op = kmalloc(sizeof(struct inode_operations));
    in.i_op->readdir = &readdir;
    out->i_op = kmalloc(sizeof(struct inode_operations));
    out->i_op->readdir = &readdir;
    vfs_root->i_op = kmalloc(sizeof(struct inode_operations));
    vfs_root->i_op->readdir = &readdir;
    // vfs_root->i_op->readdir(out, NULL);
    char *dev_path = "/dev/disk0/foo/bar";
    device_t *reg_dev = vfs_mountpoint(path_tokenize(dev_path))->root->device;
    printf("device %s\n", reg_dev == NULL ? "null" : reg_dev->name);
    printf("super: %s\n", dev_path);

    char buffer[1024];
    for(int i=0;i<1024;i++) {
        buffer[i] = 'a';
    }
    dev_read(reg_dev, 1024, 1024, buffer);
    printf("[SUPER BLOCK]\nMAGIC %x\nINODES %d\nBLOCKS %d\n",
        ((ext2_superblock_t *)buffer)->magic,
        ((ext2_superblock_t *)buffer)->inodes_count,
        ((ext2_superblock_t *)buffer)->blocks_count
    );
    struct bitmap *bm = bitmap_create(128);
    bitmap_set(bm, 64, true);
    bitmap_dump(bm);
    // Have to call explicitly. Cause without this,
    // rip goes to the end of the bootloader and
    // unrecover kernel panic. Also this changes the current kernel_entry
    // Function to the idle thread.
    // Rest of the powerup process in done by init.
    thread_run_idle();
}