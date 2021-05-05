#pragma once

#include <stdint.h>
#include "multiboot2.h"
#include "vga_text.h"
#include "memory.h"

void pmm_free(uint64_t page);
uint64_t pmm_alloc();
uint64_t pmm_calloc();