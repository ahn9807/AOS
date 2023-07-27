#include "apic.h"
#include "acpi.h"
#include "cpu.h"
#include "lib/debug.h"
#include "lib/stddef.h"
#include "lib/types.h"
#include "memory.h"
#include "vga_text.h"
#include "vmm.h"

uintptr_t lapic_base_addr;
uintptr_t ioapic_base_addr;

static inline void lapic_write(size_t addr, uint32_t value)
{
	*((volatile uint32_t *)(lapic_base_addr + addr)) = value;
	asm volatile("" ::: "memory"); // inline assembly barrier for block memory reordering
}

static inline uint32_t lapic_read(size_t addr)
{
	return *((volatile uint32_t *)lapic_base_addr + addr);
}

void lapic_send_ipi(int id, uint32_t val)
{
	ASSERT(lapic_base_addr != 0); // should be removed for speed up!
	lapic_write(0x310, id << 24);
	lapic_write(0x300, val);
	do {
		asm volatile("pause" : : : "memory");
	} while (lapic_read(0x300) & (1 << 12));
}

void apic_init()
{
	struct acpi_rsdt *rsdt = acpi_get_rsdt("APIC");
	ASSERT(rsdt != NULL);

	struct acpi_madt *madt = (void *)rsdt;
	uintptr_t lapic_base = madt->local_controller_address;
	uintptr_t ioapic_base = 0;
	int cur_core = 0;
	int cur_io_num = 0;

	ASSERT(lapic_base != 0);

	for (struct acpi_madt_entry *entry = madt->entries;
	     (uintptr_t)entry < (uintptr_t)rsdt + (uintptr_t)madt->header.length;
	     entry = (struct acpi_madt_entry *)((uintptr_t)entry + (uintptr_t)entry->length)) {
		switch (entry->type) {
		case ACPI_MADT_LOCAL_APIC:
			// CPU i is enabled
			if (entry->data.local_apic.flags & 0x1) {
				cpu_info_table[cur_core].cpuid = cur_core;
				cpu_info_table[cur_core].lapicid = entry->data.local_apic.apic_id;
				cur_core++;
				if (cur_core > MAX_CPU_NUM) {
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
	vmm_set_page(kernel_P4, lapic_base_addr, lapic_base, PAGE_PRESENT);
	vmm_set_page(kernel_P4, ioapic_base_addr, ioapic_base, PAGE_PRESENT);
	vmm_activate(kernel_P4);

	ASSERT(num_of_cpu != 0);
}