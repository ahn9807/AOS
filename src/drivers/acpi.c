#include "acpi.h"
#include "string.h"
#include "memory.h"
#include "vga_text.h"
#include "vmm.h"

static struct acpi_rsdt* global_rsdt_pointer = NULL;

static inline int check_rsdp(uint8_t *rsdp) {
	uint8_t check = 0;
	size_t size = sizeof(struct acpi_rsdp);

	if(!strncmp(rsdp, "RSD PTR ", 8)) {
		while(size--) check += (int8_t) *rsdp++;
		return check;
	}

	return -1;
}

static struct acpi_rsdp * get_rsdt() {
	// Get Extended BIOS data area pointer address
	uintptr_t ebda_pa = *(uint16_t *)P2V(0x40e);
	uint8_t *ebda = (uint8_t *)P2V(ebda_pa);
	uint8_t *ebda_cursor = ebda;

	// Get virtual address of EBDA
	while(ebda_cursor <= ebda + 0x400) {
		if(!check_rsdp(ebda_cursor)) {
			return (struct acpi_rsdp *)ebda_cursor;
		}

		ebda_cursor += 0x10;
	}

	ebda_cursor = (uint8_t *)0x000e0000;
	while(ebda_cursor < P2V(0x000fffff)) {
		if(!check_rsdp(ebda_cursor)) {
			return (struct acpi_rsdp *)ebda_cursor;
		}

		ebda_cursor += 0x10;
	}

	return NULL;
}

void acpi_init() {
	struct acpi_rsdp* rsdp = get_rsdt();
	global_rsdt_pointer = (struct acpi_rsdt *)P2V(rsdp->rsdt & 0xffffffff);

	// This rsdt area is marked as 0x1 in multiboot2 mma flags.
	// Thus memory_init ignore this area as an allocation point.
	// Also physical memory manager ignore this area for managing.
	// So we have to set VMM to use this area.
	uint64_t addr = (uint64_t)P2V(global_rsdt_pointer);
	vmm_set_page(kernel_P4, addr, rsdp->rsdt, PAGE_PRESENT);
	vmm_activate(kernel_P4);
}

struct acpi_rsdt *acpi_get_rsdt(char signature[4]) {
	struct acpi_rsdt* rsdt = global_rsdt_pointer;

	if(!rsdt) {
		return NULL;
	}

	int entries = (rsdt->header.length - sizeof(rsdt->header)) / sizeof(uint32_t);

	for(int i = 0; i < entries; ++i) {
		struct acpi_rsdt_header *hdr = (struct acpi_rsdt_header *)P2V(rsdt->sdt[i]);

		if(!strncmp(hdr->signature, signature, 4)) {
			return hdr;
		}
	}

	return NULL;
}