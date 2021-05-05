#pragma once

#include <stdint.h>
#include <stddef.h>

#define KERNEL_OFFSET 0xFFFFFF8000000000
#define V2P(a) ((uintptr_t)(a) & ~KERNEL_OFFSET)
#define P2V(a) ((void *)((uintptr_t)(a) | KERNEL_OFFSET))
#define VA_TO_PAGE(a) ((uint64_t)(a) / PAGE_SIZE)
#define PAGE_TO_VA(a) ((uint64_t)(a) * PAGE_SIZE)
#define PA_TO_FRAME(a) ((uint64_t)(a) / PAGE_SIZE)
#define FRAME_TO_PA(a) ((uinte64_t)(a) * PAGE_SIZE)

#define incptr(p, n) ((void *)(((uintptr_t)(p)) + (n)))

#define P1_OFFSET(a) (((a)>>12) & 0x1FF)
#define P2_OFFSET(a) (((a)>>21) & 0x1FF)
#define P3_OFFSET(a) (((a)>>30) & 0x1FF)
#define P4_OFFSET(a) (((a)>>39) & 0x1FF)

#define PAGE_EXIST(p) ((p) != (uint64_t)-1)

#define PAGE_PRESENT 1 << 0
#define PAGE_WRITE 1 << 1
#define PAGE_USER_ACCESSIBLE 1 << 2
#define PAGE_WRITE_THROUGH 1 << 3
#define PAGE_NO_CACHE 1 << 4
#define PAGE_ACCESSED 1 << 5
#define PAGE_DIRTY 1 << 6
#define PAGE_HUGE_PAGE 1 << 7
#define PAGE_GLOBAL 1 << 8
#define PAGE_NO_EXECUTE 1 << 63

#define PAGE_SIZE       0x1000
#define ENTRIES_PER_PT  512

extern uint64_t kernel_P4;

typedef uint64_t physical_address_t;
typedef uint64_t virtual_address_t;

typedef uint64_t frame_t;
typedef uint64_t page_t;

int multiboot_init(uint64_t magic, unsigned long multiboot_addr, uint64_t *kernel_start, uint64_t *kernel_end, uint64_t *multiboot_start, uint64_t *multiboot_end);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);