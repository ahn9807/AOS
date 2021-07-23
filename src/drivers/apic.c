#include "acpi.h"
#include "apic.h"
#include "vga_text.h"
#include "cpu.h"
#include "debug.h"
#include "memory.h"

void apic_init() {
	struct acpi_rsdt* rsdt = acpi_get_rsdt("APIC");

	struct acpi_madt *madt = (void *)rsdt;
	uintptr_t lapic_base = madt->local_controller_address;
	uintptr_t ioapic_base = NULL;
	int cur_core = 0;
	int cur_io_num = 0;

	ASSERT(lapic_base != NULL);

	for(struct acpi_madt_entry * entry = madt->entries; (uintptr_t)entry < (uintptr_t)rsdt + (uintptr_t)madt->header.length; entry = (uintptr_t)entry + (uintptr_t)entry->length) {
		switch (entry->type)
		{
			case ACPI_MADT_LOCAL_APIC:
				// CPU i is enabled
				if(entry->data.local_apic.flags & 0x1) {
					cpu_info_table[cur_core].cpuid = cur_core;
					cpu_info_table[cur_core].lapicid = entry->data.local_apic.apic_id;	
					cur_core++;
					if(cur_core > MAX_CPU_NUM) {
						PANIC("UNSUPPORTED NUM OF CPU (128");	
					}
				}
				break;
			case ACPI_MADT_IO_APIC:
				// IOAPIC Must be one at a entire CPU
				ASSERT(cur_io_num != 1);
				ioapic_base = entry->data.ioapic.ioapic_address;
				cur_io_num++;
				break;
			case ACPI_MADT_INT_SRC_OVERRIDE:
				break;
			default:
				break;
		}
	}

	num_of_cpu = cur_core;
	lapic_base_addr = P2V(lapic_base);
	ioapic_base_addr = P2V(ioapic_base_addr);

	ASSERT(num_of_cpu != 0);
}

void apic_ipi();