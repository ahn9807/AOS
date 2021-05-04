#include "mm.h"
#include "vga_text.h"

page_t virtual_addr_to_page(virtual_address_t addr) {
    if (!(addr < 0x0000800000000000 || addr >= 0xffff800000000000))
    {
        panic("out of mmap!");
    }

    return addr / PAGE_SIZE;
}

virtual_address_t page_to_virtual_addr(page_t p) {
    return p * PAGE_SIZE;
}

uint64_t p4_index(page_t p)
{
    return (p >> 27) & 0777;
}

uint64_t p3_index(page_t p)
{
    return (p >> 18) & 0777;
}

uint64_t p2_index(page_t p)
{
    return (p >> 9) & 0777;
}

uint64_t p1_index(page_t p)
{
    return (p >> 0) & 0777;
}

frame_t physical_addr_to_frame(physical_address_t addr) {
    return addr / PAGE_SIZE;
}

physical_address_t frame_to_physical_addr(frame_t f) {
    return f * PAGE_SIZE;
}