#pragma once

#include <stdint.h>
#include "multiboot2.h"
#include "vga_text.h"
#include "mm.h"

typedef struct multiboot_tag_mmap multiboot_tag_mmap_t;

void init_allocator(
    physical_address_t kernel_start,
    physical_address_t kernel_end,
    physical_address_t multiboot_start,
    physical_address_t multiboot_end,
    multiboot_tag_mmap_t* mmaps
);

frame_t allocate_frame();

void deallocate_frame(frame_t f);

void choose_next_area();