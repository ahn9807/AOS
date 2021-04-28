#include "multiboot2.h"
#include "vga_text.h"

#include <stdint.h>

void debug_multiboot2(unsigned long multiboot_addr) {
    struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_addr);
    unsigned size;

    size = *(unsigned *)multiboot_addr;

    for (tag = (struct multiboot_tag *)(multiboot_addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        if (tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO)
        {
            struct multiboot_tag_basic_meminfo *mem_info = (struct multiboot_tag_basic_meminfo *)tag;
            printf("mem_lower = 0x%xKB, mem_upper = 0x%xKB\n",
                   ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower,
                   ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper);
        }
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            multiboot_memory_map_t *mmap;

            printf("mmap\n");

            for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
                (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
                mmap = (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)tag)->entry_size))
                printf(" base_addr = 0x%x%x,"
                       " length = 0x%x%x, type = 0x%x\n",
                       (unsigned)(mmap->addr >> 32),
                       (unsigned)(mmap->addr & 0xffffffff),
                       (unsigned)(mmap->len >> 32),
                       (unsigned)(mmap->len & 0xffffffff),
                       (unsigned)mmap->type);
        }
        if(tag->type == MULTIBOOT_TAG_TYPE_ELF_SECTIONS) {
            multiboot_tag_elf_sections* elf_section = (multiboot_tag_elf_sections*)tag;
            printf("entsize: 0x%x, num: 0x%x, shndx: 0x%x, size: 0x%x\n", elf_section->entsize, elf_section->num, elf_section->shndx, elf_section->size);

            for(int i=0;i<elf_section->num;i++) {
                multiboot_elf64_shdr* shdr = (multiboot_elf64_shdr*)((uintptr_t)elf_section->sections + elf_section->entsize * i);
                printf("Kernel ELF Addr: 0x%d Len: 0x%0x\n", shdr->addr, shdr->size);
            }

            printf("sections = %s\n", elf_section->sections);
        }
    }
}

multiboot_tag* parse_multiboot(uint8_t  multiboot_tag_type, unsigned long multiboot_addr) {
    struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_addr);

    for (tag = (struct multiboot_tag *)(multiboot_addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        if(tag->type == multiboot_tag_type) {
            return tag;
        }
    }
}