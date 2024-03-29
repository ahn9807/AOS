#include "vmm.h"
#include "intrinsic.h"
#include "lib/debug.h"
#include "lib/stddef.h"
#include "lib/string.h"
#include "memory.h"
#include "pmm.h"
#include "vga_text.h"

uint64_t vmm_new_p4()
{
	uint64_t p4 = (uint64_t)pmm_alloc();
	memcpy((void *)P2V(p4), (void *)kernel_P4, PAGE_SIZE);
	return p4;
}

static int page_exists(uint64_t P4, uint64_t addr)
{
	if (P4 && PRESENT(P4E) && PRESENT(P3E) && PRESENT(P2E))
		return 1;
	return 0;
}

static int touch_page(uint64_t P4, uint64_t addr, uint16_t flags)
{
	if (!P4)
		return -1;

	if ((!PRESENT(P4E)) && (!(P4E = (uint64_t)pmm_calloc())))
		return -1;
	P4E |= flags | PAGE_PRESENT;

	if ((!PRESENT(P3E)) && (!(P3E = (uint64_t)pmm_calloc())))
		return -1;
	P3E |= flags | PAGE_PRESENT;

	if ((!PRESENT(P2E)) && (!(P2E = (uint64_t)pmm_calloc())))
		return -1;
	P2E |= flags | PAGE_PRESENT;

	return 0;
}

uint64_t vmm_get_page(uint64_t P4, uint64_t addr)
{
	if (page_exists(P4, addr)) {
		return P1E;
	}

	return -1;
}

int vmm_set_page(uint64_t P4, uint64_t addr, uint64_t page, uint16_t flags)
{
	if (!page_exists(P4, addr)) {
		if (touch_page(P4, addr, flags))
			return -1;
	}

	// We have to set PAGE_USER_ACCESSIBLE to all page directories
	if (flags & PAGE_USER_ACCESSIBLE) {
		touch_page(P4, addr, flags);
	}
	P1E = page | flags | PAGE_PRESENT;

	return 0;
}

int vmm_set_pages(uint64_t P4, uint64_t addr, uint64_t page, uint16_t flags, size_t num)
{
	for (int i = 0; i < num; i++) {
		if (vmm_set_page(P4, addr + PAGE_SIZE * i, page + PAGE_SIZE * i, flags)) {
			for (int j = 0; j <= i; j++) {
				vmm_clear_page(P4, addr + PAGE_SIZE * i, true);
			}
		}
	}

	return 0;
}

void vmm_activate(uintptr_t p4)
{
	if (p4 == 0) {
		lcr3(V2P((uint64_t)&p4_table));
	} else {
		lcr3(V2P((uint64_t)p4));
	}
}

void vmm_free(uintptr_t p4)
{
	ASSERT(p4 != (uintptr_t)&p4_table);

	if (p4 == 0)
		return;

	PANIC("NOT IMPLEMENTED");
}

void vmm_clear_page(uint64_t P4, uint64_t addr, int free)
{
	if (!page_exists(P4, addr))
		return;

	uint64_t *pt;

	P1E = 0;

	if (!free)
		return;

	pt = PT(P2E);
	for (int i = 0; i < ENTRIES_PER_PT; i++)
		if (pt[i])
			return;
	pmm_free((void *)MASK_FLAGS(P2E));
	P2E = 0;

	pt = PT(P3E);
	for (int i = 0; i < ENTRIES_PER_PT; i++)
		if (pt[i])
			return;
	pmm_free((void *)MASK_FLAGS(P3E));
	P3E = 0;

	pt = PT(P4E);
	for (int i = 0; i < ENTRIES_PER_PT; i++)
		if (pt[i])
			return;
	pmm_free((void *)MASK_FLAGS(P4E));
	P4E = 0;
}

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define offset(p) ((uintptr_t)(p) % PAGE_SIZE)
#define remaining(p) (PAGE_SIZE - offset(p))
size_t memcpy_to_p4(uint64_t P4, void *dst, void *src, size_t n)
{
	size_t copied = 0;
	while (n) {
		size_t bytes = min(remaining(dst), n);
		uintptr_t page = vmm_get_page(P4, (uintptr_t)dst);
		if (!PAGE_EXIST(page))
			return copied;

		void *to = (void *)P2V(MASK_FLAGS(page) + offset(dst));
		memcpy(to, src, bytes);

		copied += bytes;
		n -= bytes;
		dst = incptr(dst, bytes);
		src = incptr(src, bytes);
	}
	return copied;
}

size_t memcpy_from_p4(void *dst, uint64_t P4, void *src, size_t n)
{
	size_t copied = 0;
	while (n) {
		size_t bytes = min(remaining(src), n);
		uintptr_t page = vmm_get_page(P4, (uintptr_t)src);
		if (!PAGE_EXIST(page))
			return copied;

		void *from = (void *)P2V(MASK_FLAGS(page) + offset(src));
		memcpy(dst, from, bytes);

		copied += bytes;
		n -= bytes;
		dst = incptr(dst, bytes);
		src = incptr(src, bytes);
	}
	return copied;
}