#pragma once

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE 512

#define KERNEL_OFFSET 0xFFFFFF8000000000

#define PRESENT 1 << 0
#define WRITEABLE 1 << 1
#define USER_ACCESSIBLE 1 << 2
#define WRITE_THROUGH 1 << 3
#define NO_CACHE 1 << 4
#define ACCESSED 1 << 5
#define DIRTY 1 << 6
#define HUGE_PAGE 1 << 7
#define GLOBAL 1 << 8
#define NO_EXECUTE 1 << 63

#define VA_TO_PA(a) ((uintptr_t)(a) & ~KERNEL_OFFSET)
#define PA_TO_VA(a) ((void*)((uint64_t)(a) | KERNEL_OFFSET))

typedef uint64_t physical_address_t;
typedef uint64_t virtual_address_t;

typedef uint64_t frame_t;
typedef uint64_t page_t;

typedef uint64_t page_table_entry_t;

page_t virtual_addr_to_page(virtual_address_t addr);

virtual_address_t page_to_virtual_addr(page_t p);

uint64_t p4_index(page_t p);

uint64_t p3_index(page_t p);

uint64_t p2_index(page_t p);

uint64_t p1_index(page_t p);

frame_t physical_addr_to_frame(physical_address_t addr);

physical_address_t frame_to_physical_addr(frame_t f);