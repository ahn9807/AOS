#include "memory.h"
#include "multiboot2.h"
#include "vmm.h"
#include "vga_text.h"
#include "frame_allocator.h"

uint64_t kernel_P4;

void memory_init(uintptr_t kernel_start, uintptr_t kernel_end, uintptr_t multiboot_start, uintptr_t multiboot_end)
{
	kernel_P4 = (uint64_t)&p4_table;
	uint64_t start, end;
	uint32_t type, i = 1;

	init_pmm(kernel_start, kernel_end, multiboot_start, multiboot_end, boot_mmap);

	while (!multiboot_get_memory_area(i++, &start, &end, &type))
	{
		for (uint64_t p = start; p < end; p += PAGE_SIZE)
		{
			if(type != 0x1)
				continue;
			if (p >= kernel_start && p < kernel_end)
				continue;
			if (p >= multiboot_start && p < multiboot_end)
				continue;

			uint64_t addr = (uint64_t)P2V(p);
			kernel_P4 = pmm_calloc();
			uint64_t page = vmm_get_page(kernel_P4, addr);
			if (!PAGE_EXIST(page) || !(page & PAGE_PRESENT))
			{
				uint16_t flags = PAGE_GLOBAL | PAGE_WRITE;
				vmm_set_page(kernel_P4, addr, p, flags | PAGE_PRESENT);
			}

			// If type is mmap_free
			if (type == 0x1)
				pmm_free(p);
		}
	}
}

void *memcpy(void *dst, const void *src, size_t n)
{
	char *dp = dst;
	const char *sp = src;
	while (n--)
		*dp++ = *sp++;
	return dst;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *p = s;
	while (n--)
		*p++ = (unsigned char)c;
	return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
	if (src == dst)
		return dst;

	const void *src_end = (const void *)((uintptr_t)src + n);
	if (src < dst && dst < src_end)
	{
		char *dp = dst;
		const char *sp = src;
		while (n--)
			dp[n] = sp[n];
		return dst;
	}

	memcpy(dst, src, n);
	return dst;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *p1 = s1, *p2 = s2;
	for (; n--; p1++, p2++)
	{
		if (*p1 != *p2)
			return *p1 - *p2;
	}
	return 0;
}