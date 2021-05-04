#pragma once

#include <stdint.h>
#include "area_frame_allocator.h"
#include "mm.h"
#include "vga_text.h"

typedef uint64_t pte_t;

pte_t frame_to_page_table_entry(frame_t f, uint64_t flags);

uint64_t pte_get_flags(pte_t pte);

bool pte_get_unused(pte_t pte);

void pte_set_unused(pte_t* pte);

physical_address_t pte_to_physical_addr(pte_t pte);

void set_page_table(pte_t* pte, uint64_t index, pte_t next_pte);

pte_t* get_page_table(pte_t* pte, uint64_t index);

void clear_page_table(pte_t* pte);

pte_t* next_page_table(pte_t* pte, uint64_t index);

pte_t* create_next_table(pte_t* pte, uint64_t index);