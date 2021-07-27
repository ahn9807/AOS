extern ap_main
extern p4_table
global ap_trampoline

; this code will be relocated to 0x8000, sets up environment for calling a C function
bits 16
ap_trampoline:
    cli
    cld
    jmp 0:0x8010

align 0x10
_L8010:
    xor ax, ax
    mov ds, ax ; ds <- 0
    lgdt [0x8118] ; load 32bit segment
    mov eax, cr0
    or eax, 1 ; CR0_PE
    mov cr0, eax
    jmp 8:0x8040 ; long jmp to gdt:code_segment

align 0x20
bits 32
_L8040:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov eax, [0x8148] ; load pml4e
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5 ; CR4_PAE
    mov cr4, eax

    ; Enable long mode
    mov ecx, 0xC0000080 ; EFER_MSR
    rdmsr
    or eax, 1 << 8 ; EFER_LME
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    xor eax, eax
    xor eax, eax
    ; Load gdt64
    lgdt [0x8138]
    mov esp, 0x8170
    push 0x8
    push 0x80a0
    retf

align 0x20
bits 64
_L80A0:
    ; Get LAPIC ID
    mov eax, 1
    cpuid
    shr ebx, 24
    mov rdi, rbx
    ; Call secondary_init_kernel
    mov rax, [0x8150]
    jmp rax
    ud2 ; Generates an invalid opcode exception

align 0x100
_L8100_GDT_Table:
	dq 0 ; NULL Segment
	dq 0xCF9A000000FFFF ; Code segment
	dq 0xCF92000000FFFF ; Data segment
_L8118_GDT_value:
	dw 0x17
	dd 0x8100
align 0x20
_L8120_GDT64_table:
    dq 0 ; NULL segment
    dq 0x00af9a000000ffff ; Code segment
    dq 0x00cf92000000ffff ; Data segment
__L8138_GDT64_value:
    dw 0x17
    dq 0x8120
align 0x8
_L8148_plm4e:
    dq p4_table
_L8150_secondary_kernel_init:
    dq ap_main
_L8158_retf_stack_end:
    dq 0
    dq 0
    dq 0
_L8170_retf_stack_start:

global ap_trampoline_end
ap_trampoline_end:
