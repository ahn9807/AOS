ENTRY(start)
OUTPUT_FORMAT(elf64-x86-64)

KERNEL_OFFSET = 0xFFFFFF8000000000;

SECTIONS {
    . = 1M;

    .bootstrap ALIGN(4K) :
    {
        KEEP(*(.multiboot_header))
        *(.bootstrap. .bootstrap.*)
    }

    . += KERNEL_OFFSET;
    . += 0x10000;

    /* code section */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_OFFSET)
    {
        *(.text)
    }

    /* Read-only data. */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_OFFSET)
    {
        *(.rodata .rodata.*)
    }

    .eh_frame ALIGN(4K) : AT(ADDR(.eh_frame) - KERNEL_OFFSET)
    {
        *(.eh_frame)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_OFFSET)
    {
        *(.padata)
        *(.data .data.* )
    }

    .syscall_info_table ALIGN(4K) : AT(ADDR(.syscall_info_table) - KERNEL_OFFSET)
    {        
        _start_syscall_info_table = .;
        *(__syscalls_info)
        _end_syscall_info_table = .;
    }

    /* Read-write data (uninitialized) and stack */
    _start_bss = .;
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_OFFSET)
    {
        *(.bss .bss.*)
    }
    _end_bss = .;

    .kernel_stack ALIGN(4K) : AT(ADDR(.kernel_stack) - KERNEL_OFFSET)
    {
        *(.kernel_stack)
    }

    /* extended page table for kernel usage */
    .kernel_page_table ALIGN(4K) : AT(ADDR(.kernel_page_table) - KERNEL_OFFSET)
    {
        *(.kernel_page_table)
    }

    .gcc_except_table ALIGN(4K) : AT(ADDR(.gcc_except_table) - KERNEL_OFFSET)
    {
        *(.gcc_except_table)
    }

    /* DWARF debug sections.
    Symbols in the DWARF debugging sections are relative to the beginning
    of the section so we begin them at 0.  */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    .debug_ranges   0 : { *(.debug_ranges) }
    .debug_macro    0 : { *(.debug_macro) }
    .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) }
}