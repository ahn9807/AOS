extern syscall_handler

global syscall_entry
extern tss

section .text
syscall_entry:
	mov [rel temp1], rbx
	mov [rel temp2], r12
	mov rbx, rsp
	mov r12, tss ; copy &tss to r12
	mov qword r12, [r12] ; copy *tss to r12
	mov qword rsp, [r12 + 4] ; copy tss->rsp0 to r12
	push 0x1B ; ss
	push rbx ; rsp 
	push r11 ; eflags
	push 0x23 ; cs
	push rcx ; rip
	push qword 0 ; skip error_cde and vec_no
	push qword 0
	push 0x1B ; ds
	push 0x1B ; es
	push rax ; now push registers
	mov qword rbx, [rel temp1]
	push rbx
	push qword 0
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push qword 0
	mov qword r12, [rel temp2]
	push r12
	push r13
	push r14
	push r15
	mov rdi, rsp ; first argument - intr_frame
check_intr:
	bts r11, 9
	jnb no_sti
	sti
no_sti:
	mov r12, syscall_handler
	call r12
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
	add rsp, 0x20
	pop rcx
	add rsp, 0x8
	pop r11
	pop rsp
	o64 sysret ; o64 neended for return to 64bit mode.
	
section .data
temp1:
	dq 0
temp2:
	dq 0