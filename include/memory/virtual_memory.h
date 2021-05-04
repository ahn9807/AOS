#pragma once

#include "mm.h"
#include "page_table.h"
#include "area_frame_allocator.h"

static physical_address_t k_page_table_p4;

void vm_init(physical_address_t page_table_addr);

pte_t* vm_get_p4_table();

physical_address_t vm_translate_virtual_addr(virtual_address_t vaddr);

frame_t vm_translate_page(page_t p);

void vm_map_page_to_frame(page_t p, frame_t f, uint64_t flags);

void vm_map(page_t p, uint64_t flags);

void vm_map_identity(frame_t f, uint64_t flags);

void vm_unmap(page_t p);