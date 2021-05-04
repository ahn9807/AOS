#include <assert.h>
#include <stdint.h>
#include "virtual_memory.h"

void vm_init(physical_address_t page_table_addr) {
    k_page_table_p4 = page_table_addr;
    //clear_page_table(k_page_table_p4);
}

pte_t* vm_get_p4_table() {
    return (pte_t*)PA_TO_VA(k_page_table_p4 & ~511);
}

physical_address_t vm_translate_virtual_addr(virtual_address_t vaddr) {
    uint64_t offset = vaddr % PAGE_SIZE;

    return frame_to_physical_addr(vm_translate_page(vaddr)) + offset;
}

frame_t vm_translate_page(page_t p) {
    pte_t* p4 = vm_get_p4_table();
    pte_t* p3 = next_page_table(p4, p4_index(p));
    pte_t* p2 = next_page_table(p3, p3_index(p));
    pte_t* p1 = next_page_table(p2, p2_index(p));

    return physical_addr_to_frame(pte_to_physical_addr(*get_page_table(p1, p1_index(p))));
}

void vm_map_page_to_frame(page_t p, frame_t f, uint64_t flags) {
    pte_t* p4 = vm_get_p4_table();
    // printf("p 0x%x f 0x%x\n", p, f);
    // panic("halt");
    pte_t* p3 = create_next_table(p4, p4_index(p));
    pte_t* p2 = create_next_table(p3, p3_index(p));
    pte_t* p1 = create_next_table(p2, p2_index(p));

    if(!pte_get_unused(*get_page_table(p1, p1_index(p)))) {
        panic("page table is already allocated!");
    }

    set_page_table(p1, p1_index(p), frame_to_page_table_entry(f, PRESENT | flags));
}
void vm_map(page_t p, uint64_t flags) {
    frame_t f = allocate_frame();
    vm_map_page_to_frame(p, f, flags);
}

void vm_map_identity(frame_t f, uint64_t flags) {
    page_t p = page_to_virtual_addr(frame_to_physical_addr(f));
    vm_map_page_to_frame(p, f, flags);
}

void vm_unmap(page_t p) {
    pte_t* p4 = vm_get_p4_table();
    pte_t* p3 = next_page_table(p4, p4_index(p));
    pte_t* p2 = next_page_table(p3, p3_index(p));
    pte_t* p1 = next_page_table(p2, p2_index(p));

    frame_t f = frame_to_physical_addr(pte_to_physical_addr(*get_page_table(p1, p1_index(p))));
    pte_set_unused(get_page_table(p1, p1_index(p)));
}