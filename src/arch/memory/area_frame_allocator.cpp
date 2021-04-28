#include "area_frame_allocator.h"

frame area_frame_allocator::allocate_frame() {
    if(current_area != nullptr) {
        frame f = next_free_frame;
        frame current_area_last_frame = containing_address(
            current_area->addr + current_area->len - 1
        );

        if(f > current_area_last_frame) {
            choose_next_area();
        } else if(f >= kernel_start && f <= kernel_end) {
            next_free_frame = kernel_end + 1;
        } else if(f >= multiboot_start && f <= multiboot_end) {
            next_free_frame = multiboot_end + 1;
        } else {
            next_free_frame += 1;
            return f;
        }
        return allocate_frame();
    } else {
        panic("out of free frame");
    }
    panic("allocate failed!");
}

void area_frame_allocator::deallocate_frame(frame f) {
    panic("not implemented!");
}

void area_frame_allocator::choose_next_area() {
    uint64_t min_address = UINT64_MAX;
    uint64_t base_addr = 0;

    for (multiboot_memory_map_t* mmap = ((struct multiboot_tag_mmap *)areas)->entries;
    (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)areas + areas->size;
    mmap = (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)areas)->entry_size)) {
        uint64_t address = mmap->addr + mmap->len - 1;
        if(containing_address(address) >= next_free_frame && min_address > mmap->addr) {
            min_address = mmap->addr;
            current_area = mmap;
        }
    }

    if(current_area != nullptr) {
        frame start_frame = containing_address(current_area->addr);
        if(next_free_frame < start_frame) {
            next_free_frame = start_frame;
        }
    } else{
        panic("panic from choose next frame!");
    }
}