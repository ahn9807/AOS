#include "multiboot2.h"
#include "lib/types.h"
#include "memory.h"
#include "vga_text.h"

void debug_multiboot2(unsigned long multiboot_addr)
{
	struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_addr);
	unsigned size;

	size = *(unsigned *)multiboot_addr;

	for (tag = (struct multiboot_tag *)(multiboot_addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
		if (tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO) {
			struct multiboot_tag_basic_meminfo *mem_info = (struct multiboot_tag_basic_meminfo *)tag;
			printf("mem_lower = 0x%xKB, mem_upper = 0x%xKB\n",
			       ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower,
			       ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper);
		}
		if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
			multiboot_memory_map_t *mmap;

			printf("mmap\n");

			for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
			     (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
			     mmap = (multiboot_memory_map_t *)((unsigned long)mmap +
							       ((struct multiboot_tag_mmap *)tag)->entry_size)) {
				printf(" base_addr = 0x%x, length = 0x%x, type = 0x%x\n", (unsigned)(mmap->addr),
				       (unsigned)(mmap->len), (unsigned)(mmap->type));
			}
		}
		// if (tag->type == MULTIBOOT_TAG_TYPE_ELF_SECTIONS)
		// {
		// 	struct multiboot_tag_elf_sections *elf_section = (struct multiboot_tag_elf_sections *)tag;
		// 	printf("entsize: 0x%x, num: 0x%x, shndx: 0x%x, size: 0x%x\n", (uint64_t)elf_section->entsize, (uint64_t)elf_section->num, (uint64_t)elf_section->shndx, (uint64_t)elf_section->size);

		// 	for (int i = 0; i < elf_section->num; i++)
		// 	{
		// 		struct multiboot_elf64_shdr *shdr = (struct multiboot_elf64_shdr *)((uintptr_t)elf_section->sections + elf_section->entsize * i);
		// 		printf("Kernel ELF Addr: 0x%x Len: 0x%x\n", (uint64_t)shdr->addr, (uint64_t)shdr->size);
		// 	}
		// }
	}
}

struct multiboot_tag *parse_multiboot(uint8_t multiboot_tag_type, unsigned long multiboot_addr)
{
	struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_addr);

	for (tag = (struct multiboot_tag *)(multiboot_addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
		if (tag->type == multiboot_tag_type) {
			return tag;
		}
	}
}

int multiboot_init(uint64_t magic, unsigned long multiboot_addr, uint64_t *kernel_start, uint64_t *kernel_end,
		   uint64_t *multiboot_start, uint64_t *multiboot_end)
{
	if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
		boot_mmap = parse_multiboot(MULTIBOOT_TAG_TYPE_MMAP, multiboot_addr);
		*multiboot_start = multiboot_addr;
		*multiboot_end = *multiboot_start + *(unsigned *)multiboot_addr;
		*kernel_start = UINTMAX_MAX;
		*kernel_end = 0;

		struct multiboot_tag_elf_sections *elf_section = (struct multiboot_tag_elf_sections *)parse_multiboot(
			MULTIBOOT_TAG_TYPE_ELF_SECTIONS, multiboot_addr);
		for (int i = 0; i < elf_section->num; i++) {
			struct multiboot_elf64_shdr *shdr =
				(struct multiboot_elf64_shdr *)((uintptr_t)elf_section->sections +
								elf_section->entsize * i);
			if (shdr->size == 0x0)
				continue;
			if (V2P(shdr->addr) + shdr->size > *kernel_end) {
				*kernel_end = V2P(shdr->addr) + shdr->size;
			}
			if (V2P(shdr->addr) < *kernel_start) {
				*kernel_start = V2P(shdr->addr);
			}
		}
	} else
		return -1;

	return 0;
}

int multiboot_get_memory_area(uint64_t count, uintptr_t *start, uintptr_t *end, uint32_t *type)
{
	int index = 0;
	multiboot_memory_map_t *mmap;
	for (mmap = ((struct multiboot_tag_mmap *)boot_mmap)->entries;
	     (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)boot_mmap + boot_mmap->size;
	     mmap = (multiboot_memory_map_t *)((unsigned long)mmap +
					       ((struct multiboot_tag_mmap *)boot_mmap)->entry_size)) {
		if (index == count) {
			*start = mmap->addr;
			*end = mmap->addr + mmap->len;
			*type = mmap->type;
			return 0;
		}
		index++;
	}

	return -1;
}
