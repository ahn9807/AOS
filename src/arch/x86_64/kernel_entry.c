#include "vga_text.h"
#include "multiboot2.h"
#include "area_frame_allocator.h"
#include "virtual_memory.h"
#include "page_table.h"
#include "x86.h"
#include "mm.h"

extern uint64_t p4_table;

int kernel_entry(unsigned long magic, unsigned long multiboot_addr)
{
    terminal_initialize();
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        panic("Multiboot2 is not initialized!");
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
    struct multiboot_tag_mmap* mmap = (struct multiboot_tag_mmap*)parse_multiboot(MULTIBOOT_TAG_TYPE_MMAP, multiboot_addr);
    struct multiboot_tag_basic_meminfo* available_mmap = (struct multiboot_tag_basic_meminfo*)parse_multiboot(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, multiboot_addr);
    struct multiboot_tag_elf_sections* elf_section = (struct multiboot_tag_elf_sections*)parse_multiboot(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, multiboot_addr);
    printf("entsize: 0x%x, num: 0x%x, shndx: 0x%x, size: 0x%x\n", elf_section->entsize, elf_section->num, elf_section->shndx, elf_section->size);
    for(int i=0;i<elf_section->num;i++) {
        struct multiboot_elf64_shdr* shdr = (struct multiboot_elf64_shdr*)((uintptr_t)elf_section->sections + elf_section->entsize * i);
        printf("Kernel ELF Addr: 0x%x Len: 0x%x type: 0x%x\n", shdr->addr, shdr->size, shdr->type);
        if(shdr->size == 0x0)
            continue;
        if(shdr->addr + shdr->size > kernel_end) {
            kernel_end = shdr->addr + shdr->size;
        }
        if(shdr->addr < kernel_start) {
            kernel_start = shdr->addr;
        }
    }
    printf("[ Allocate Frames ] kernel_start: 0x%x kernel_end: 0x0%x \n[ Allocate Frames ] multiboot_start: 0x%0x multiboot_end: 0x%0x\n",
        kernel_start, kernel_end, multiboot_start, multiboot_end
    );
    printf("PD: 0x%x\n", &p4_table);
    init_allocator(kernel_start, (uint64_t)kernel_end, multiboot_start, multiboot_end, mmap);
    // frame_t kernel_p4 = allocate_frame();
    vm_init((uint64_t)&p4_table);
    uint64_t cur_page = kernel_start;

    int index = 0;
    for (multiboot_memory_map_t* entry = mmap->entries;
        (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap->size;
        entry = (multiboot_memory_map_t *)((unsigned long)entry + ((struct multiboot_tag_mmap *)mmap)->entry_size)) {
            printf("start: 0x%x end: 0x%x type: 0x%x\n", (uint32_t)entry->addr, entry->addr + entry->len, entry->type);
            for(uint64_t p = entry->addr; p < entry->addr + entry->len; p += PAGE_SIZE) {
                if(p >= kernel_start && p < kernel_end)
                    continue;
                if(p >= multiboot_start && p < multiboot_end)
                    continue;

                virtual_address_t addr = PA_TO_VA(p);
                vm_map_page_to_frame(virtual_addr_to_page(addr), physical_addr_to_frame(p), GLOBAL | WRITEABLE | PRESENT);
                index ++;
            }
        }

    printf("%d\n",index);
    panic("halt");
    // page_t p1 = 1;
    // vm_map(p1, ACCESSED);
    // printf("Allocated page: 0x%x frame: 0x%x\n", p1, vm_translate_page(p1));
    // page_t p2 = 2;
    // vm_map(p2, ACCESSED);
    // printf("Allocated page: 0x%x frame: 0x%x\n", p2, vm_translate_page(p2));
    // page_t p3 = 3;
    // vm_map(p3, ACCESSED);
    // printf("Allocated page: 0x%x frame: 0x%x\n", p3, vm_translate_page(p3));
    // frame_t f = 0x100101;
    // vm_map_identity(f, ACCESSED);
    // printf("Allocated page: 0x%x frame: 0x%x\n", f, vm_translate_virtual_addr(f));
    // lcr3((uint64_t)frame_to_physical_addr(kernel_p4));
    panic("panic kernel entry");
    // virtual_memory kernel_vm = virtual_memory((physical_address)&_kernel_page_table);
    // printf("0x%x\n", kernel_vm.get_p4());
    // printf("fa: 0x%x\n", &fa);
    // area_frame_allocator* pa_ptr = &fa;
    // printf("&fa: 0x%x\n", (uintptr_t)pa_ptr);
    // printf("kernel:offset 0x%x\n", KERNEL_OFFSET);
    // printf("&fa - KERNEL_OFFSET: 0x%x\n", (uint64_t)pa_ptr - (uint64_t)KERNEL_OFFSET);
    // (&fa)->allocate_frame();
    // // (pa_ptr)->allocate_frame();
    // uint64_t cur_page = kernel_start;
    // panic("kernel_entry");
    // while(cur_page < kernel_end - KERNEL_OFFSET) {
    //     kernel_vm.identity_map(frame::containing_address(cur_page), page_flags::ACCESSED, &fa);
    //     cur_page += PAGE_SIZE;
    //
    // }
    // printf("P4: 0x%x\n", (uint64_t)kernel_vm.get_p4());
    // panic("halt");
    // lcr3((uint64_t)kernel_vm.get_p4());
    // page p1 = 1;
    // kernel_vm.map(p1, page_flags::ACCESSED, &fa);
    // printf("Allocated page: 0x%x frame: %x\n", p1, kernel_vm.translate_page(p1));
    // page p2 = 2;
    // kernel_vm.map(p2, page_flags::ACCESSED, &fa);
    // printf("Allocated page: 0x%x frame: %x\n", p2, kernel_vm.translate_page(p2));
    // page p3 = 3;
    // kernel_vm.map(p3, page_flags::ACCESSED, &fa);
    // printf("Allocated page: 0x%x frame: %x\n", p3, kernel_vm.translate_page(p3));
    // frame f = 0x100101;
    // kernel_vm.identity_map(f, page_flags::ACCESSED, &fa);
    // printf("Allocated page: 0x%x frame: 0x%x\n", f, kernel_vm.translate_va(f.start_paddress()));

    printf("OS SUSPENDED!\n");
}