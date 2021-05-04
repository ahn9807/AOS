#include "area_frame_allocator.h"

static struct allocated_frame_list {
    frame_t next_free_frame;
    frame_t kernel_start;
    frame_t kernel_end;
    frame_t multiboot_start;
    frame_t multiboot_end;
    multiboot_memory_map_t* current_area;
    multiboot_tag_mmap_t* areas;
} kernel_mem;

void init_allocator(
    physical_address_t kernel_start,
    physical_address_t kernel_end,
    physical_address_t multiboot_start,
    physical_address_t multiboot_end,
    multiboot_tag_mmap_t* mmaps
) {
    kernel_mem.kernel_start = physical_addr_to_frame(kernel_start);
    kernel_mem.kernel_end = physical_addr_to_frame(kernel_end);
    kernel_mem.multiboot_start = physical_addr_to_frame(kernel_start);
    kernel_mem.multiboot_end = physical_addr_to_frame(kernel_end);
    kernel_mem.next_free_frame = physical_addr_to_frame(0);
    kernel_mem.areas = mmaps;
    kernel_mem.current_area = NULL;

    choose_next_area();
}

frame_t allocate_frame() {
    if(kernel_mem.current_area != NULL) {
        frame_t f = kernel_mem.next_free_frame;
        frame_t current_area_last_frame = physical_addr_to_frame(
            kernel_mem.current_area->addr + kernel_mem.current_area->len - 1
        );

        if(f > current_area_last_frame) {
            choose_next_area();
        } else if(f >= kernel_mem.kernel_start && f <= kernel_mem.kernel_end) {
            kernel_mem.next_free_frame = kernel_mem.kernel_end + 1;
        } else if(f >= kernel_mem.multiboot_start && f <= kernel_mem.multiboot_end) {
            kernel_mem.next_free_frame = kernel_mem.multiboot_end + 1;
        } else {
            kernel_mem.next_free_frame += 1;
            return f;
        }
        return allocate_frame();
    } else {
        panic("out of free frame_t");
    }
    panic("allocate failed!");
    return 0;
}

void deallocate_frame(frame_t f) {
    panic("not implemented!");
}

void choose_next_area() {
    uint64_t min_address = UINT64_MAX;
    uint64_t base_addr = 0;

    for (multiboot_memory_map_t* mmap = ((struct multiboot_tag_mmap *)kernel_mem.areas)->entries;
    (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)kernel_mem.areas + kernel_mem.areas->size;
    mmap = (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)kernel_mem.areas)->entry_size)) {
        uint64_t address = mmap->addr + mmap->len - 1;
        if(physical_addr_to_frame(address) >= kernel_mem.next_free_frame && min_address > mmap->addr) {
            min_address = mmap->addr;
            kernel_mem.current_area = mmap;
        }
    }

    if(kernel_mem.current_area != NULL) {
        frame_t start_frame = physical_addr_to_frame(kernel_mem.current_area->addr);

        if(kernel_mem.next_free_frame < start_frame) {
            kernel_mem.next_free_frame = start_frame;
        }
    } else{
        panic("panic from choose next frame_t!");
    }
}