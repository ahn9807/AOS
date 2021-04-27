global long_mode_start
extern kernel_entry
KERNEL_OFFSET equ 0xFFFFFFFF80000000

section .bootstrap.boot64
bits 64
long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call kernel_entry - KERNEL_OFFSET
    hlt