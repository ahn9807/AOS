#pragma once

#include "lib/types.h"

extern uintptr_t lapic_base_addr;
extern uintptr_t ioapic_base_addr;

void apic_init();
void lapic_send_ipi(int id, uint32_t val);