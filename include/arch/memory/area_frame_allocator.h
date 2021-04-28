#pragma once

#include <stdint.h>
#include "multiboot2.h"
#include "vga_text.h"

typedef uint64_t frame;

#define PAGE_SIZE 4096

class area_frame_allocator {
    frame next_free_frame;
    frame kernel_start;
    frame kernel_end;
    frame multiboot_start;
    frame multiboot_end;
    multiboot_memory_map_t* current_area;
    multiboot_tag_mmap* areas;

    public:
        area_frame_allocator(uint64_t kernel_start, uint64_t kernel_end, uint64_t multiboot_start, uint64_t multiboot_end, multiboot_tag_mmap* mmaps) {
            next_free_frame = containing_address(0);
            this->kernel_start = containing_address(kernel_start);
            this->kernel_end = containing_address(kernel_end);
            this->multiboot_start = containing_address(multiboot_start);
            this->multiboot_end = containing_address(multiboot_end);
            this->areas = mmaps;
            this->current_area = nullptr;

            choose_next_area();
        };
        frame allocate_frame();
        void deallocate_frame(frame f);

        static frame containing_address(uint64_t addr) {
            return frame(addr / PAGE_SIZE);
        }
    private:
        void choose_next_area();
};