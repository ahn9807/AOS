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
    add rsp, 0x8
    mov ds, [rsp + 0x8]
    mov es, [rsp + 0x0]
    add rsp, 0x10 ; pop error code and int number

    add rsp, 0x20 ; pop errcode and int number

    iretq

ISR_NOERR_CODE 0
ISR_NOERR_CODE 1
ISR_NOERR_CODE 2
ISR_NOERR_CODE 3
ISR_NOERR_CODE 4
ISR_NOERR_CODE 5
ISR_NOERR_CODE 6
ISR_NOERR_CODE 7
ISR_ERROR_CODE 8
ISR_NOERR_CODE 9
ISR_ERROR_CODE 10
ISR_ERROR_CODE 11
ISR_ERROR_CODE 12
ISR_ERROR_CODE 13
ISR_ERROR_CODE 14
ISR_NOERR_CODE 15
ISR_NOERR_CODE 16
ISR_ERROR_CODE 17
ISR_NOERR_CODE 18
ISR_NOERR_CODE 19
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
ISR_NOERR_CODE 32
ISR_NOERR_CODE 33
ISR_NOERR_CODE 34
ISR_NOERR_CODE 35
ISR_NOERR_CODE 36
ISR_NOERR_CODE 37
ISR_NOERR_CODE 38
ISR_NOERR_CODE 39
ISR_NOERR_CODE 40
ISR_NOERR_CODE 41
ISR_NOERR_CODE 42
ISR_NOERR_CODE 43
ISR_NOERR_CODE 44
ISR_NOERR_CODE 45
ISR_NOERR_CODE 46
ISR_NOERR_CODE 47
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