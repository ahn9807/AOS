#include "memory.h"
#include "multiboot2.h"
#include "vmm.h"
#include "vga_text.h"
#include "pmm.h"
#include "intrinsic.h"
#include "string.h"
#include "thread.h"

uintptr_t kernel_P4;
extern uintptr_t _start_bss;
extern uintptr_t _end_bss;

static void bss_init()
{
	memset(&_start_bss, 0, &_end_bss - &_start_bss);
}

/* Initializing memory */
uintptr_t memory_init(uintptr_t kernel_start, uintptr_t kernel_end, uintptr_t multiboot_start, uintptr_t multiboot_end, uintptr_t multiboot_addr)
{
	kernel_P4 = (uint64_t)&p4_table;
	uint64_t start, end;
	uint32_t type, i = 1;

	init_pmm(kernel_start, kernel_end, multiboot_start, multiboot_end, (struct multiboot_tag_mmap *)parse_multiboot(MULTIBOOT_TAG_TYPE_MMAP, multiboot_addr));

	while (!multiboot_get_memory_area(i++, &start, &end, &type))
	{
		for (uint64_t p = start; p < end; p += PAGE_SIZE)
		{
			uint16_t flags = PAGE_GLOBAL | PAGE_WRITE;

			if (type != 0x1)
				continue;
			if (p >= kernel_start && p < kernel_end)
				flags &= ~PAGE_WRITE;
			if (p >= multiboot_start && p < multiboot_end)
				flags &= ~PAGE_WRITE;

			uint64_t addr = (uint64_t)P2V(p);
			vmm_set_page(kernel_P4, addr, p, flags | PAGE_PRESENT);
		}
	}
	bss_init();
	lcr3((uintptr_t)V2P(kernel_P4));

	return kernel_P4;
}

/* Creates a new page map level 4 (pml4) has mappings for kernel
 * virtual addresses, but none for user virtual addresses.
 * Returns the new page directory, or a null pointer if memory
 * allocation fails. This pml4 points to the old kernel's p3 directory.
 * setup kernel part of memory */
uintptr_t mm_create_p4()
{
	uint64_t pml4 = (uint64_t)P2V(pmm_alloc());
	printf("pmm: 0x%x\n", pml4);
	if (pml4)
		memcpy((void *)pml4, (void *)kernel_P4, PAGE_SIZE);
	return pml4;
}

int mm_is_user(void *ptr) {
	if((uintptr_t)ptr >= KERNEL_OFFSET) {
		return 0;
	} 

	return 1;
}