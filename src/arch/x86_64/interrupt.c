#include "interrupt.h"
#include "intrinsic.h"

#define INTERRUPT_LEN 256
#define FLAG_IF (1<<9)

static struct idt_entry idt[INTERRUPT_LEN];
static intr_handler_t intr_handlers[INTERRUPT_LEN];
static const char *intr_names[INTERRUPT_LEN];
extern uintptr_t intr_vectors[];

struct {
    uint16_t len;
    struct idt_entry *addr;
} __attribute__ ((packed)) idtr;

static intr_handler_t temp_int_handler(struct intr_frame *frame) {
    debug_intr_frame(frame);
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

    bind_interrupt_with_name(3, &temp_int_handler, "#BP");
    bind_interrupt_with_name(14, &temp_int_handler, "#PF");
    bind_interrupt_with_name(8, &temp_int_handler, "#DF");
    bind_interrupt_with_name(13, &temp_int_handler, "#GP");
    // bind_interrupt_with_name(0, &temp_int_handler, "DE");

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

struct intr_frame* interrupt_handler(struct intr_frame *frame) {
    if(intr_handlers[frame->vec_no])
        return intr_handlers[frame->vec_no](frame);

    debug_intr_frame(frame);
    panic("UNHANDLED INTERRUPT OCCURRED!");
}

void enable_interrupt() {
    __asm__ __volatile__ ("sti\n");
}

void disable_interrupt() {
    __asm__ __volatile__ ("cld\n");
}

void debug_intr_frame(struct intr_frame *f) {
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