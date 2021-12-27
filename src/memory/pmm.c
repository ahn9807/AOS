#include "memory.h"
#include "pmm.h"
#include "multiboot2.h"
#include "string.h"

// https://github.com/evanw/buddy-malloc/blob/master/buddy-malloc.c

// Virtual addres of next free page
uint64_t next = 0;

struct memory_area
{
	uint64_t start;
	uint64_t end;
};

struct free_frame_list
{
	uint64_t next_free_frame;
	struct memory_area kernel_area;
	struct memory_area multiboot_area;
	struct multiboot_mmap_entry *current_pool;
	struct multiboot_tag_mmap *memory_pool;
} pmm_frame_list;

void init_pmm(uint64_t kernel_start, uint64_t kernel_end, uint64_t multiboot_start, uint64_t multiboot_end, struct multiboot_tag_mmap *mmap_pool)
{
	pmm_frame_list.kernel_area.start = PA_TO_FRAME(kernel_start);
	pmm_frame_list.kernel_area.end = PA_TO_FRAME(kernel_end);
	pmm_frame_list.multiboot_area.start = PA_TO_FRAME(multiboot_start);
	pmm_frame_list.multiboot_area.end = PA_TO_FRAME(multiboot_end);
	pmm_frame_list.memory_pool = (struct multiboot_tag_mmap *)P2V(mmap_pool);
	pmm_frame_list.next_free_frame = PA_TO_FRAME(0);

	choose_next_area();
}

void pmm_free(void *page)
{
	// do nothing
}

void *pmm_alloc()
{
	if (pmm_frame_list.current_pool != NULL)
	{
		uint64_t f = pmm_frame_list.next_free_frame;
		uint64_t current_area_last_frame = PA_TO_FRAME(pmm_frame_list.current_pool->addr + pmm_frame_list.current_pool->len - 1);

		if (f > current_area_last_frame)
		{
			choose_next_area();
		}
		else if (f >= pmm_frame_list.kernel_area.start && f <= pmm_frame_list.kernel_area.end)
		{
			pmm_frame_list.next_free_frame = pmm_frame_list.kernel_area.end + 1;
		}
		else if (f >= pmm_frame_list.multiboot_area.start && f <= pmm_frame_list.multiboot_area.end)
		{
			pmm_frame_list.next_free_frame = pmm_frame_list.multiboot_area.end + 1;
		}
		else
		{
			pmm_frame_list.next_free_frame += 1;
			return (void *)V2P(FRAME_TO_PA(f));
		}
		return pmm_alloc();
	}
	else
	{
		return NULL;
	}
	return NULL;
}

// Must be changed!!!
void *pmm_alloc_pages(size_t size)
{
	void *first_addr = pmm_alloc();
	size -= 1;
	while (size != 0)
	{
		pmm_alloc();
		size -= 1;
	}
	return first_addr;
}

void *pmm_free_pages(void *s_addr, size_t size)
{
	// do nothing
}

void *pmm_calloc()
{
	void *page = pmm_alloc();
	memset((void *)P2V(page), 0, PAGE_SIZE);
	return page;
}

static void choose_next_area()
{
	uint64_t min_address = UINT64_MAX;
	uint64_t base_addr = 0;

	for (multiboot_memory_map_t *mmap = ((struct multiboot_tag_mmap *)pmm_frame_list.memory_pool)->entries;
		 (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)pmm_frame_list.memory_pool + pmm_frame_list.memory_pool->size;
		 mmap = (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)pmm_frame_list.memory_pool)->entry_size))
	{
		if (mmap->addr < 0xffff)
		{
			continue;
		}
		uint64_t address = PA_TO_FRAME(mmap->addr + mmap->len - 1);
		if (address >= pmm_frame_list.next_free_frame && min_address > mmap->addr && mmap->type == 0x1)
		{
			min_address = mmap->addr;
			pmm_frame_list.current_pool = mmap;
		}
	}

	if (pmm_frame_list.current_pool != NULL)
	{
		uint64_t start_frame = PA_TO_FRAME(pmm_frame_list.current_pool->addr);
		if (pmm_frame_list.next_free_frame < start_frame)
		{
			pmm_frame_list.next_free_frame = start_frame;
		}
	}
	else
	{
		panic("panic from choose next frame_t!");
	}
}