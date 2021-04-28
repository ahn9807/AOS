#include "vga_text.h"
#include "multiboot2.h"
#include "area_frame_allocator.h"

extern "C" int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    terminal_initialize();

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        printf("Multiboot2 ...");
    }
    else
    {
        printf("AOS init started!\n");
    }

    unsigned size;

    if (multiboot_addr & 7)
    {
        printf("Unaligned mbi: 0x%x\n", multiboot_addr);
        panic("kernel panic");
    }

    uint64_t multiboot_start = multiboot_addr;
    uint64_t multiboot_end = multiboot_start + *(unsigned *)multiboot_addr;
    uint64_t kernel_start = UINT64_MAX;
    uint64_t kernel_end = 0;

    debug_multiboot2(multiboot_addr);
    multiboot_tag_mmap* mmap = (multiboot_tag_mmap*)parse_multiboot(MULTIBOOT_TAG_TYPE_MMAP, multiboot_addr);
    multiboot_tag_basic_meminfo* available_mmap = (multiboot_tag_basic_meminfo*)parse_multiboot(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, multiboot_addr);
    multiboot_tag_elf_sections* elf_section = (multiboot_tag_elf_sections*)parse_multiboot(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, multiboot_addr);
    printf("entsize: 0x%x, num: 0x%x, shndx: 0x%x, size: 0x%x\n", elf_section->entsize, elf_section->num, elf_section->shndx, elf_section->size);
    for(int i=0;i<elf_section->num;i++) {
        multiboot_elf64_shdr* shdr = (multiboot_elf64_shdr*)((uintptr_t)elf_section->sections + elf_section->entsize * i);
        printf("Kernel ELF Addr: 0x%d Len: 0x%0x\n", shdr->addr, shdr->size);
        if(shdr->size == 0x0)
            continue;
        if(shdr->addr + shdr->size > kernel_end) {
            kernel_end = shdr->addr + shdr->size;
        }
        if(shdr->addr < kernel_start) {
            kernel_start = shdr->addr;
        }
    }
    printf("[ Allocate Frames ] kernel_start: 0x%x kernel_end: 0x%x \n[ Allocate Frames ] multiboot_start: 0x%0x multiboot_end: 0x%x\n",
        kernel_start, kernel_end, multiboot_start, multiboot_end
    );
    area_frame_allocator frame_allocator = area_frame_allocator(kernel_start, kernel_end, multiboot_start, multiboot_end, mmap);
    int index =0;
    while(1) {
        frame allocated_frame = frame_allocator.allocate_frame();
        printf("Allocated Frame 0x%x\n", allocated_frame * 4096);
    }
}