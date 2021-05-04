global start

extern long_mode_start
extern kernel_entry
KERNEL_OFFSET equ 0xFFFFFF8000000000
PAGE_PRESENT equ 1 << 0
PAGE_WRITEABLE equ 1 << 1
PAGE_GLOBAL equ 1 << 8
; KERNEL_OFFSET equ 0x0

section .bootstrap
bits 32

; After booting Multiboot
; EAX 0x2BADB002 EBX 32-bit physical address of the Multiboot information
; CS 32-bit r/w code segment Offset 0 Limit 0xFFFFFFF
; DS, ES, FS, GS, SS 32-bit r/w data segment Offset 0 Limit 0xFFFFFFF
; A20 gate enabled
; CR0 PG 0 PE 1 (Others - undefined)
; EFLAGS VM 0 IF 0 (Others - undefined)
; ESP undefined
; GDTR can be invalid
; IDTR undefined
start:
    mov esp, stack_top
    mov edi, eax ; multiboot2 specification
    mov esi, ebx ; multiboot2 specification

    call check_multiboot
    call check_cpuid
    call check_long_mode

    ; call set_up_page_tables
    call enable_paging

    lgdt [gdt64.ptr_low - KERNEL_OFFSET]
    jmp gdt64.code:long_mode_start

    ; print `OK` to screen
    mov dword [0xb8000], 0x2f4b2f4f
    hlt

check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "0"
    jmp error

check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

set_up_page_tables:
    ; map first P4 entry to P3 table
    mov eax, p3_table - KERNEL_OFFSET
    or eax, 0b11 ; present + writable
    mov [p4_table - KERNEL_OFFSET], eax

    ; map first P3 entry to P2 table
    mov eax, p2_table - KERNEL_OFFSET
    or eax, 0b11 ; present + writable
    mov [p3_table - KERNEL_OFFSET], eax

    ; map each P2 entry to a huge 2MiB page
    mov ecx, 0         ; counter variable

.map_p2_table:
    ; map ecx-th P2 entry to a huge page that starts at address 2MiB*ecx
    mov eax, 0x200000  ; 2MiB
    mul ecx            ; start address of ecx-th page
    or eax, 0b10000011 ; present + writable + huge
    mov [p2_table - KERNEL_OFFSET + ecx * 8], eax ; map ecx-th entry

    inc ecx            ; increase counter
    cmp ecx, 512       ; if counter == 512, the whole P2 table is mapped
    jne .map_p2_table  ; else map the next entry

    ret

enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, p4_table - KERNEL_OFFSET
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

; Prints `ERR: ` and the given error code to screen and hangs.
; parameter: error code (in ascii) in al
error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

stack_bottom:
    times 4096 dw 0
stack_top:

bits 64

section .bootstrap.boot64
long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rax, kernel_address_space
    jmp rax
    hlt

section .text
kernel_address_space:
    mov rax, KERNEL_OFFSET
    add rsp, KERNEL_OFFSET

    mov rax, gdt64.ptr
    lgdt [rax]
    mov rax, 0
    mov ss, rax
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax
    mov rax, qword _kernel_entry
    push qword 0x8
    push rax
    retfq
    hlt

_kernel_entry:
    mov rax, kernel_entry
    call rax
section .kernel_stack
align 4096
kernel_stack_bottom:
    times 4096 dq 0
kernel_stack_top:

section .rodata
gdt64:
	dq 0 ; zero entry
.code equ $ - gdt64
	dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53) ; code segment
.data equ $ - gdt64
	dq (1<<44) | (1<<47) | (1<<41) ; data segment
.end equ $
.ptr_low:
	dw .end - gdt64 - 1
	dd gdt64 - KERNEL_OFFSET
.ptr:
	dw .end - gdt64 - 1
	dq gdt64

; temporary page table for booting
section .kernel_page_table
global p4_table
global p3_table
global p2_table
global p1_table

align 4096
p4_table:
    dq p3_table - KERNEL_OFFSET + (PAGE_PRESENT | PAGE_WRITEABLE)
    times 510 dq 0
    dq p3_table - KERNEL_OFFSET + (PAGE_PRESENT | PAGE_WRITEABLE | PAGE_GLOBAL)
p3_table:
    dq p2_table - KERNEL_OFFSET + (PAGE_PRESENT | PAGE_WRITEABLE | PAGE_GLOBAL)
    times 511 dq 0
p2_table:
    dq p1_table - KERNEL_OFFSET + (PAGE_PRESENT | PAGE_WRITEABLE | PAGE_GLOBAL)
    times 511 dq 0
p1_table:
%assign i 0
%rep 512
    dq (i << 12) + (PAGE_PRESENT | PAGE_WRITEABLE | PAGE_GLOBAL)
%assign i i+1
%endrep