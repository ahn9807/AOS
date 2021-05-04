#include "mm.h"
#include "page_table.h"
#include "vga_text.h"
#include "area_frame_allocator.h"

pte_t frame_to_page_table_entry(frame_t f, uint64_t flags) {
    if(frame_to_physical_addr(f) & !0x000ffffffffff000 == 0) {
        panic("malacious frame!");
    }
    return frame_to_physical_addr(f) | flags;
}

uint64_t pte_get_flags(pte_t pte) {
    return pte & 0xfff0000000000fff;
}

bool pte_get_unused(pte_t pte) {
    return pte == 0;
}

void pte_set_unused(pte_t* pte) {
    *pte = 0;
}

physical_address_t pte_to_physical_addr(pte_t pte) {
    if(pte_get_flags(pte) & PRESENT) {
        return pte & 0x000ffffffffff000;
    } else {
        panic("Not present pte!");
    }
}

void set_page_table(pte_t* pte, uint64_t index, pte_t next_pte) {
    if(index > PAGE_TABLE_SIZE) {
        panic("Page table size is 512");
    }

    pte[index] = next_pte;
}

pte_t* get_page_table(pte_t* pte, uint64_t index) {
    if(index > PAGE_TABLE_SIZE) {
        panic("Page table size is 512");
    }

    return &pte[index];
}

void clear_page_table(pte_t* pte) {
    for(int i=0;i<PAGE_TABLE_SIZE;i++) {
        pte_set_unused(&pte[i]);
    }
}

pte_t* next_page_table(pte_t* pte, uint64_t index) {
    printf("pte: 0x%x index: %d\n", pte, index);
    panic("cnt");
    pte_t entry = pte[index - 2];
    if((pte_get_flags(entry) & PRESENT) && ! ( pte_get_flags(entry) & HUGE_PAGE)) {
        return (pte_t*)(pte_to_physical_addr(pte[index]));
    } else {
        return NULL;
    }
}

pte_t* create_next_table(pte_t* pte, uint64_t index) {
    if(next_page_table(pte, index) == NULL) {
        if(pte_get_flags(*get_page_table(pte, index)) & HUGE_PAGE) {
            panic("Mapping does not support huge page");
        }
        frame_t f = allocate_frame();
        set_page_table(pte, index, frame_to_page_table_entry(f, PRESENT | WRITEABLE));
        clear_page_table(next_page_table(pte, index));
    }

    return next_page_table(pte, index);
}