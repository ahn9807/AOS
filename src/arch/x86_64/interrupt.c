#include <stdbool.h>
#include "interrupt.h"
#include "intrinsic.h"
#include "pic8259.h"
#include "string.h"
#include "sched.h"
#include "debug.h"
#include "cpu_flags.h"

#define INTERRUPT_LEN 256

static struct idt_entry idt[INTERRUPT_LEN];
static intr_handler_t intr_handlers[INTERRUPT_LEN];
static const char *intr_names[INTERRUPT_LEN];
extern uintptr_t intr_vectors[];

static bool in_hardware_interrupt = false;

struct {
    uint16_t len;
    struct idt_entry *addr;
} __attribute__ ((packed)) idtr;

static enum irq_handler_result temp_int_handler(struct intr_frame *frame) {
    intr_debug(frame);
    return FAILED;
}

void interrupt_init() {
    memset(idt, 0, sizeof(idt));
    memset(intr_handlers, 0, sizeof(intr_handlers));

    for(int i=0;i<INTERRUPT_LEN;i++) {
        install_interrupt(&idt[i], intr_vectors[i], 0);
        intr_names[i] = "unknown";
    }

    idtr.addr = idt;
    idtr.len = sizeof(idt) -1;

    bind_interrupt_with_name(3, &temp_int_handler, "#BP Breakpoint Exception");
    bind_interrupt_with_name(14, &temp_int_handler, "#PF Page-Fault Exception");
    bind_interrupt_with_name(8, &temp_int_handler, "#DF Double Fault Exception");
    bind_interrupt_with_name(13, &temp_int_handler, "#GP General Protection Exception");
    // bind_interrupt_with_name(0, &temp_int_handler, "DE");
    intr_names[0] = "#DE Divide Error";
	intr_names[1] = "#DB Debug Exception";
	intr_names[2] = "NMI Interrupt";
	intr_names[3] = "#BP Breakpoint Exception";
	intr_names[4] = "#OF Overflow Exception";
	intr_names[5] = "#BR BOUND Range Exceeded Exception";
	intr_names[6] = "#UD Invalid Opcode Exception";
	intr_names[7] = "#NM Device Not Available Exception";
	intr_names[8] = "#DF Double Fault Exception";
	intr_names[9] = "Coprocessor Segment Overrun";
	intr_names[10] = "#TS Invalid TSS Exception";
	intr_names[11] = "#NP Segment Not Present";
	intr_names[12] = "#SS Stack Fault Exception";
	intr_names[13] = "#GP General Protection Exception";
	intr_names[14] = "#PF Page-Fault Exception";
	intr_names[16] = "#MF x87 FPU Floating-Point Error";
	intr_names[17] = "#AC Alignment Check Exception";
	intr_names[18] = "#MC Machine-Check Exception";
	intr_names[19] = "#XF SIMD Floating-Point Exception";

    lidt(&idtr);
}

intr_handler_t bind_interrupt(uint32_t num, intr_handler_t fn) {
    intr_handler_t old = intr_handlers[num];
    intr_handlers[num] = fn;
    return old;
}

intr_handler_t bind_interrupt_with_name(uint32_t num, intr_handler_t fn, const char* name) {
    intr_names[num] = name;
    return bind_interrupt(num, fn);
}

// Handlers for all interrupts, faults, and exceptions.
// This function is bind to intr_vectors.asm
// DO NOT CHANGE NAME OR PARAMETERS!!
void interrupt_handler(struct intr_frame *frame) {
    bool is_hardware = false;
    enum irq_handler_result return_val = OK;

    // This interrupt is originated from pic (hardware interrupt)
    // This interrupt must be handle one at a time.
    is_hardware = frame->vec_no >= 0x20 && frame->vec_no < 0x30;
    if(is_hardware) {
        in_hardware_interrupt = true;
    }

    // INTR is not NULL
    if(intr_handlers[frame->vec_no]) {
        return_val = intr_handlers[frame->vec_no](frame);
    }
    // INTR is NULL. Panic and debug INTR
    else {
        intr_debug(frame);
        panic("UNHANDLED INTERRUPT OCCURRED!");
    }

    if(is_hardware) {
        in_hardware_interrupt = false;
        pic_end_of_interrupt(frame->vec_no);
    }

    switch (return_val)
    {
        case OK:
            // do noting
            break;
        case YIELD_ON_RETURN:
            sched_tick();
            break;
        case KILL_ON_RETURN:
            panic("NOT IMPLEMENTED");
            break;
        case FAILED:
            panic("FAILED TO HANDLE INTERRUPT");
            break;
        default:
            break;
    }
}

enum intr_level intr_get_level() {
    return read_eflags() & FLAG_IF ? INTR_ON : INTR_OFF;
}

enum intr_level intr_set_level(enum intr_level irl) {
    return irl == INTR_ON ? intr_enable() : intr_disable();
}

bool intr_context() {
    return in_hardware_interrupt;
}

enum intr_level intr_enable() {
    enum intr_level prev_intr = intr_get_level();
    __asm__ __volatile__ ("sti\n");
    return prev_intr;
}

enum intr_level intr_disable() {
    enum intr_level prev_intr = intr_get_level();
    __asm__ __volatile__ ("cli\n");
    return prev_intr;
}

void intr_debug(struct intr_frame *f) {
	/* CR2 is the linear address of the last page fault.
	   See [IA32-v2a] "MOV--Move to/from Control Registers" and
	   [IA32-v3a] 5.14 "Interrupt 14--Page Fault Exception
	   (#PF)". */
	uint64_t cr2 = rcr2();
	printf ("Interrupt 0x%x (%s) at rip=0x%x\n",
			f->vec_no, intr_names[f->vec_no], f->rip);
	printf ("cr2=0x%x error=%d\n", cr2, f->error_code);
	printf ("rax 0x%x rbx 0x%x rcx 0x%x rdx 0x%x\n",
			f->reg.rax, f->reg.rbx, f->reg.rcx, f->reg.rdx);
	printf ("rsp 0x%x rbp 0x%x rsi 0x%x rdi 0x%x\n",
			f->rsp, f->reg.rbp, f->reg.rsi, f->reg.rdi);
	printf ("rip 0x%x r8 0x%x  r9 0x%x r10 0x%x\n",
			f->rip, f->reg.r8, f->reg.r9, f->reg.r10);
	printf ("r11 0x%x r12 0x%x r13 0x%x r14 0x%x\n",
			f->reg.r11, f->reg.r12, f->reg.r13, f->reg.r14);
	printf ("r15 0x%x rflags 0x%x\n", f->reg.r15, f->eflags);
	printf ("es: %04x ds: %04x cs: %04x ss: %04x\n",
			f->es, f->ds, f->cs, f->ss);
}