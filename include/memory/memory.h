#pragma once

#include <stdint.h>
#include <stddef.h>

#define KERNEL_OFFSET 0xFFFFFF8000000000
#define V2P(a) ((uintptr_t)(a) & ~KERNEL_OFFSET)
#define P2V(a) (((uintptr_t)(a) | KERNEL_OFFSET))
#define VA_TO_PAGE(a) ((uint64_t)(a) / PAGE_SIZE)
#define PAGE_TO_VA(a) ((uint64_t)(a) * PAGE_SIZE)
#define PA_TO_FRAME(a) ((uint64_t)(a) / PAGE_SIZE)
#define FRAME_TO_PA(a) ((uint64_t)(a) * PAGE_SIZE)

#define incptr(p, n) ((void *)(((uintptr_t)(p)) + (n)))

#define P1_OFFSET(a) (((a)>>12) & 0x1FF)
#define P2_OFFSET(a) (((a)>>21) & 0x1FF)
#define P3_OFFSET(a) (((a)>>30) & 0x1FF)
#define P4_OFFSET(a) (((a)>>39) & 0x1FF)

#define PAGE_EXIST(p) ((p) != (uint64_t)-1)

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER_ACCESSIBLE 0x4
#define PAGE_WRITE_THROUGH 0x8
#define PAGE_NO_CACHE 0x10
#define PAGE_ACCESSED 0x20
#define PAGE_DIRTY 0x40
#define PAGE_HUGE_PAGE 0x80
#define PAGE_GLOBAL 0x100
#define PAGE_NO_EXECUTE 0x8000000000000000000000000

#define PAGE_SIZE       0x1000
#define ENTRIES_PER_PT  512

extern uint64_t kernel_P4;

struct frame {

};

struct page {

};

uintptr_t memory_init(uintptr_t kernel_start, uintptr_t kernel_end, uintptr_t multiboot_start, uintptr_t multiboot_end, uintptr_t multiboot_addr);
void bss_init();
uintptr_t kvm_init();
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);