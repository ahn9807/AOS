#pragma once

#include "lib/types.h"
#include "multiboot2.h"
#include "vga_text.h"

void pmm_free(void *page);
void *pmm_alloc();
void *pmm_calloc();
static void choose_next_area();
void init_pmm(uint64_t kernel_start, uint64_t kernel_end, uint64_t multiboot_start, uint64_t multiboot_end,
	      struct multiboot_tag_mmap *mmap_pool);
void *pmm_alloc_pages(size_t size);
void *pmm_free_pages(void *s_addr, size_t size);