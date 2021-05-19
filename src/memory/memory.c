#include "memory.h"
#include "multiboot2.h"
#include "vmm.h"
#include "vga_text.h"
#include "pmm.h"
#include "intrinsic.h"

uintptr_t kernel_P4;
extern uintptr_t _start_bss;
extern uintptr_t _end_bss;

uintptr_t memory_init(uintptr_t kernel_start, uintptr_t kernel_end, uintptr_t multiboot_start, uintptr_t multiboot_end, uintptr_t multiboot_addr)
{
	kernel_P4 = (uint64_t)&p4_table;
	uint64_t start, end;
	uint32_t type, i = 1;

	init_pmm(kernel_start, kernel_end, multiboot_start, multiboot_end, parse_multiboot(MULTIBOOT_TAG_TYPE_MMAP, multiboot_addr));

	while (!multiboot_get_memory_area(i++, &start, &end, &type))
	{
		for (uint64_t p = start; p < end; p += PAGE_SIZE)
		{
			uint16_t flags = PAGE_GLOBAL | PAGE_WRITE;

			if(type != 0x1)
				continue;
			if (p >= kernel_start && p < kernel_end)
				flags &= ~PAGE_WRITE;
			if (p >= multiboot_start && p < multiboot_end)
				flags &= ~PAGE_WRITE;

			uint64_t addr = (uint64_t)P2V(p);
			uint64_t page = vmm_get_page(kernel_P4, addr);
			if (page != -1)
			{
				vmm_set_page(kernel_P4, addr, p, flags | PAGE_PRESENT);
			}
		}
	}

	lcr3(V2P(kernel_P4));

	return kernel_P4;
}

/* Creates a new page map level 4 (pml4) has mappings for kernel
 * virtual addresses, but none for user virtual addresses.
 * Returns the new page directory, or a null pointer if memory
 * allocation fails. This pml4 points to the old kernel's p3 directory.
 * setup kernel part of memory */
uintptr_t kvm_init() {
	uint64_t pml4 = P2V(pmm_alloc());
	printf("pmm: 0x%x\n", pml4);
	if(pml4)
		memcpy((void *)pml4, (void *)kernel_P4, PAGE_SIZE);
	return pml4;
}

void bss_init() {
	memset(&_start_bss, 0, &_end_bss - &_start_bss);
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