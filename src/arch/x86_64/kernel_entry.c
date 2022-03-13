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
#include "timer.h"
#include "sched.h"

extern uint64_t p4_table;
extern uint64_t temp_table;

void temp_exec(void *path)
{
    if (process_exec((char *)path))
    {
        printf("\n");
        printf("exec failed");
        printf("\n");
    }
}

int ap_main();

void temp_ls(inode_t *dir_node)
{
    if (dir_node == NULL)
    {
        printf("ls: ?");
        return;
    }
    struct dentry dir;
    int cur_offset = 0;
    while (vfs_readdir(dir_node, cur_offset, &dir) > 0)
    {
        printf("%s %d ", dir.name, dir.inode->size);
        cur_offset++;
    }
}

void temp_cat(file_t *file)
{
    if (file->name == NULL)
    {
        printf("cat: not found");
        return;
    }
    cls();
    char *buf = kmalloc(4096);

    int line_count = 1;

    for (int page = 0; page < vfs_get_size(file); page += 4096)
    {
        int read_size = vfs_read(file, buf, 4096);
        for (int i = 0; i < read_size; i++)
        {
            if (buf[i] == '\n')
            {
                line_count++;
            }

            if (line_count == VGA_HEIGHT - 1)
            {
                printf("\n press kbd to continue (q to quit)...");
                while (1)
                {
                    char buffer[256];
                    int size = dev_read(vfs_mountpoint("/dev/char0")->inode->device, 0, 1, buffer);
                    if (size > 0)
                    {
                        if (buffer[0] == 'q')
                        {
                            return;
                        }
                        line_count = 0;
                        break;
                    }
                }
            }
            printf("%c", buf[i]);
        }
    }
    kfree(buf);
}

void temp_shell()
{
    char input_buffer[256];
    int input_index = 0;
    char *working_dir = kmalloc(256);
    strcpy(working_dir, "/");
    inode_t *work_dir = vfs_mountpoint(working_dir)->inode;

    while (1)
    {
        printf("\nminish_monitor> ");
        while (1)
        {
            char buffer;
            int size = dev_read(vfs_mountpoint("/dev/char0")->inode->device, 0, 1, &buffer);
            if (size > 0)
            {
                printf("%c", buffer);
                if (buffer == '\n')
                {
                    input_buffer[input_index++] = '\0';
                    input_index = 0;
                    break;
                }
                input_buffer[input_index++] = buffer;
            }
        }

        if (!strcmp(input_buffer, "ls"))
        {
            temp_ls(work_dir);
        }
        else if (!strcmp(input_buffer, "clear"))
        {
            cls();
        }
        else if (!strncmp(input_buffer, "cat", 3))
        {
            char name[128];
            strcpy(name, &input_buffer[4]);
            file_t file;
            char *path = path_absolute(working_dir, name);
            vfs_open_by_path(path, &file);
            kfree(path);
            temp_cat(&file);
        }
        else if (!strncmp(input_buffer, "cd", 2))
        {
            char name[128];
            strcpy(name, &input_buffer[3]);
            char *temp_dir = kmalloc(256);
            temp_dir = path_absolute(working_dir, name);
            dentry_t dir;
            int ret = vfs_lookup(work_dir, name, &dir);
            if (ret == FS_DIRECTORY)
            {
                kfree(working_dir);
                working_dir = temp_dir;
                work_dir = dir.inode;
            }
            else
            {
                printf("cd: no such file or direcotry: %s", name);
            }
        }
        else if (!strncmp(input_buffer, "exec", 4))
        {
            char name[128];
            strcpy(name, &input_buffer[5]);
            thread_create("exec", &temp_exec, name);
        }
        else if (!strcmp(input_buffer, "pwd"))
        {
            printf("%s", working_dir);
        }
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
    timer_init();
    thread_init();
    intr_enable();
    syscall_init();
    vfs_init();
    dev_init();
    ext2_init();

    ASSERT(vfs_mount("/", vfs_mountpoint("/dev/disk0")->inode) == 0);

    thread_create("shell", &temp_shell, NULL);
    // Have to call explicitly. Cause without this,
    // rip goes to the end of the bootloader and
    // unrecover kernel panic. Also this changes the current kernel_entry
    // Function to the idle thread.
    // Rest of the powerup process in done by init.
    thread_run_idle();
}