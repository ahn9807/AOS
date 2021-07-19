#include "memory.h"

#define FLAGS_MASK (PAGE_SIZE-1)
#define MASK_FLAGS(addr) ((uint64_t)addr & ~FLAGS_MASK)

#define PRESENT(p) (p & PAGE_PRESENT)

#define PT(ptr) ((uint64_t *)P2V(MASK_FLAGS(ptr)))
// Get the entry correspoding to address addr in page dir P4
// for P4 table (P4E), P3 table (P3E) and so on.
// Note: Those macros requires variables to be named
#define P4E (PT(P4)[P4_OFFSET(addr)])
#define P3E PT(P4E)[P3_OFFSET(addr)]
#define P2E PT(P3E)[P2_OFFSET(addr)]
#define P1E PT(P2E)[P1_OFFSET(addr)]

uint64_t vmm_new_p4();
void vmm_activate(uintptr_t p4);
uint64_t vmm_get_page(uint64_t P4, uint64_t addr);
int vmm_set_page(uint64_t P4, uint64_t addr, uint64_t page, uint16_t flags);
int vmm_set_pages(uint64_t P4, uint64_t addr, uint64_t page, uint16_t flags, size_t num); 
void vmm_clear_page(uint64_t P4, uint64_t addr, int free);
size_t memcpy_to_p4(uint64_t P4, void *dst, void *src, size_t n);
size_t memcpy_from_p4(void *dst, uint64_t P4, void *src, size_t n);

extern union PTE p4_table;
extern int kernel_start, kernel_end;