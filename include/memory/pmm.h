#pragma once

#include <stdint.h>
#include "multiboot2.h"
#include "vga_text.h"
#include "memory.h"

void pmm_free(uint64_t page);
uint64_t pmm_alloc();
uint64_t pmm_calloc();
static void choose_next_area();
void init_pmm(uint64_t kernel_start, uint64_t kernel_end, uint64_t multiboot_start, uint64_t multiboot_end, struct multiboot_tag_mmap *mmap_pool);
uint64_t pmm_alloc_pages(size_t size);
uint64_t pmm_free_pages(uint64_t s_addr, size_t size);