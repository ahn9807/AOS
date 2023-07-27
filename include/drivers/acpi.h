// +---------+    +-------+    +--------+    +------------------------+
// |  RSDP   | +->| XSDT  | +->|  FADT  |    |  +-------------------+ |
// +---------+ |  +-------+ |  +--------+  +-|->|       DSDT        | |
// | Pointer | |  | Entry |-+  | ...... |  | |  +-------------------+ |
// +---------+ |  +-------+    | X_DSDT |--+ |  | Definition Blocks | |
// | Pointer |-+  | ..... |    | ...... |    |  +-------------------+ |
// +---------+    +-------+    +--------+    |  +-------------------+ |
//                | Entry |------------------|->|       SSDT        | |
//                +- - - -+                  |  +-------------------| |
//                | Entry | - - - - - - - -+ |  | Definition Blocks | |
//                +- - - -+                | |  +-------------------+ |
//                                         | |  +- - - - - - - - - -+ |
//                                         +-|->|       SSDT        | |
//                                           |  +-------------------+ |
//                                           |  | Definition Blocks | |
//                                           |  +- - - - - - - - - -+ |
//                                           +------------------------+
//                                                        |
//                                           OSPM Loading |
//                                                       \|/
//                                                 +----------------+
//                                                 | ACPI Namespace |
//                                                 +----------------+
#pragma once

#include "lib/types.h"

struct acpi_rsdp {
    uint8_t     signature[8];
    uint8_t  checksum;
    uint8_t     oem_id[6];
    uint8_t  rev;
    uint32_t rsdt;
} __attribute__((packed));

struct acpi_rsdt_header {
    uint8_t     signature[4];
    uint32_t length;
    uint8_t  rev;
    uint8_t  checksum;
    uint8_t     oemid[6];
    uint8_t     oem_table_id[8];
    uint32_t oem_rev;
    uint32_t creator_id;
    uint32_t creator_rev;
} __attribute__((packed));

struct acpi_rsdt {
    struct acpi_rsdt_header header;
    uint32_t sdt[0];
} __attribute__((packed));

#define ACPI_MADT_LOCAL_APIC 0
#define ACPI_MADT_IO_APIC 1
#define ACPI_MADT_INT_SRC_OVERRIDE 2
#define ACPI_MADT_NMI_INT_SRC 3
#define ACPI_MADT_LAPIC_NMI 4

struct acpi_madt_entry {
    uint8_t type;
    uint8_t length;
    union {
        struct { /* LOCAL APIC */
			/// Processor ID
            uint8_t processor_id;
			/// Local APIC ID
            uint8_t apic_id;
			/// Flags. 1 means that the processor is enabled
            uint32_t flags;
        } __attribute__((packed)) local_apic;
        struct { /* IO APIC */
            uint8_t ioapic_id;
            uint8_t __reserved;
            uint32_t ioapic_address;
            uint32_t global_system_interrupt_base;
        } __attribute__((packed)) ioapic;
        struct { /* INTERRUPT SOURCE OVERRIDE */
            uint8_t bus_source;
            uint8_t irq_source;
            uint32_t global_system_interrupt;
        } __attribute__((packed)) interrupt_source_override;
    } __attribute__((packed)) data;
}__attribute__((packed));

struct acpi_madt {
    struct acpi_rsdt_header header;
    uint32_t local_controller_address;
    uint32_t flags;
    struct acpi_madt_entry entries[0];
}__attribute__((packed));

void acpi_init();
struct acpi_rsdt *acpi_get_rsdt(char signature[4]);