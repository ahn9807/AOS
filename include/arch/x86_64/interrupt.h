#pragma once

#include <stdint.h>
#include "vga_text.h"

enum intr_level {
	INTR_OFF,
	INTR_ON,
};

struct idt_registers {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
} __attribute__((packed));

struct intr_frame {
	/* Pushed by intr_entry in intr-stubs.S.
	   These are the interrupted task's saved registers. */
	struct idt_registers reg;
	uint16_t es;
	uint16_t __pad1;
	uint32_t __pad2;
	uint16_t ds;
	uint16_t __pad3;
	uint32_t __pad4;
	/* Pushed by intrNN_stub in intr-stubs.S. */
	uint64_t vec_no; /* Interrupt vector number. */
/* Sometimes pushed by the CPU,
   otherwise for consistency pushed as 0 by intrNN_stub.
   The CPU puts it just under `eip', but we move it here. */
	uint64_t error_code;
/* Pushed by the CPU.
   These are the interrupted task's saved registers. */
	uintptr_t rip;
	uint16_t cs;
	uint16_t __pad5;
	uint32_t __pad6;
	uint64_t eflags;
	uintptr_t rsp;
	uint16_t ss;
	uint16_t __pad7;
	uint32_t __pad8;
} __attribute__((packed));

struct idt_entry {
    unsigned off_lowerbits : 16;   // low 16 bits of offset in segment
	unsigned selector : 16;         // segment selector
	unsigned ist : 3;        // # args, 0 for interrupt/trap gates
	unsigned rsv1 : 5;        // reserved(should be zero I guess)
	unsigned type : 4;        // type(STS_{TG,IG32,TG32})
	unsigned s : 1;           // must be 0 (system)
	unsigned dpl : 2;         // descriptor(meaning new) privilege level
	unsigned p : 1;           // Present
	unsigned off_middlebits : 16;  // high bits of offset in segment
	uint32_t off_higherbits;
	uint32_t rsv2;
};

#define install_idt(idt, function, d, t) \
{ \
    *(idt) = (struct idt_entry) { \
        .off_lowerbits = (uint64_t) (function) & 0xffff, \
        .selector = 0x8, \
        .ist = 0, \
        .rsv1 = 0, \
        .type = (t), \
        .s = 0, \
        .dpl = (d), \
        .p = 1, \
        .off_middlebits = ((uint64_t) (function) >> 16) & 0xffff, \
        .off_higherbits = ((uint64_t) (function) >> 32) & 0xffffffff, \
        .rsv2 = 0, \
    }; \
} \

#define install_interrupt(idt, function, d) install_idt((idt), (function), (d), 14)
#define install_trap(idt, function, d) install_idt((idt), (function), (d), 15)

typedef struct intr_frame *(*intr_handler_t)(struct intr_frame *);

void interrupt_init();
void enable_interrupt();
void disable_interrupt();
intr_handler_t bind_interrupt(uint32_t num, intr_handler_t fn);
intr_handler_t bind_interrupt_with_name(uint32_t num, intr_handler_t fn, const char* name);
void debug_intr_frame(struct intr_frame *frame);