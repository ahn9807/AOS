extern interrupt_handler
section .text

%macro ISR_ERROR_CODE 1
global isr%1
isr%1:
    push %1
    jmp _interrupt_common
%endmacro

%macro ISR_NOERR_CODE 1
global isr%1
isr%1:
    push 0
    push %1
    jmp _interrupt_common
%endmacro

%macro _pusha 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro _popa 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

_interrupt_common:
    sub rsp, 0x10
    mov [rsp + 0x8], es
    mov [rsp + 0x0], ds
    _pusha
    cld

    mov rdi, rsp

    call interrupt_handler

    _popa
    mov ds, [rsp + 0x8]
    mov es, [rsp + 0x0]

    add rsp, 0x20 ; pop errcode and int number

    iretq

ISR_NOERR_CODE 0 ; Divide error
ISR_NOERR_CODE 1 ; Single steop
ISR_NOERR_CODE 2 ; Non-maskable interrupt
ISR_NOERR_CODE 3 ; Break point
ISR_NOERR_CODE 4 ; OverFlow
ISR_NOERR_CODE 5 ; Bounds
ISR_NOERR_CODE 6 ; OPCode Error
ISR_NOERR_CODE 7 ; Unable to detect processor
ISR_ERROR_CODE 8 ; Double fault
ISR_NOERR_CODE 9 ; Reserved
ISR_ERROR_CODE 10 ; Invalid TSS
ISR_ERROR_CODE 11 ; Invalid Segment
ISR_ERROR_CODE 12 ; Invalid Stack
ISR_ERROR_CODE 13 ; General Protection fault
ISR_ERROR_CODE 14 ; Page fault
ISR_NOERR_CODE 15 ; Reserved
ISR_NOERR_CODE 16 ; Math Fault
ISR_ERROR_CODE 17 ; Align Check
ISR_NOERR_CODE 18 ; Machine Check
ISR_NOERR_CODE 19 ; SIMD operation exception
ISR_NOERR_CODE 20
ISR_NOERR_CODE 21
ISR_NOERR_CODE 22
ISR_NOERR_CODE 23
ISR_NOERR_CODE 24
ISR_NOERR_CODE 25
ISR_NOERR_CODE 26
ISR_NOERR_CODE 27
ISR_NOERR_CODE 28
ISR_NOERR_CODE 29
ISR_ERROR_CODE 30
ISR_NOERR_CODE 31
ISR_NOERR_CODE 32 ; 8259A PIC
ISR_NOERR_CODE 33 ; 8259A PIC
ISR_NOERR_CODE 34 ; 8259A PIC
ISR_NOERR_CODE 35 ; 8259A PIC
ISR_NOERR_CODE 36 ; 8259A PIC
ISR_NOERR_CODE 37 ; 8259A PIC
ISR_NOERR_CODE 38 ; 8259A PIC
ISR_NOERR_CODE 39 ; 8259A PIC
ISR_NOERR_CODE 40 ; 8259A PIC
ISR_NOERR_CODE 41 ; 8259A PIC
ISR_NOERR_CODE 42 ; 8259A PIC
ISR_NOERR_CODE 43 ; 8259A PIC
ISR_NOERR_CODE 44 ; 8259A PIC
ISR_NOERR_CODE 45 ; 8259A PIC
ISR_NOERR_CODE 46 ; 8259A PIC
ISR_NOERR_CODE 47 ; 8259A PIC
%assign num 48
%rep 256-48
    ISR_NOERR_CODE num
%assign num (num + 1)
%endrep

global intr_vectors
intr_vectors:
%assign num 0
%rep 256
    dq isr%+ num
%assign num (num + 1)
%endrep