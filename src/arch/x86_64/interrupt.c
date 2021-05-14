#include "interrupt.h"
#include "intrinsic.h"

#define INTERRUPT_LEN 256
#define FLAG_IF (1<<9)

static struct idt_entry idt[INTERRUPT_LEN];
static intr_handler_t intr_handlers[INTERRUPT_LEN];
static const char *intr_names[INTERRUPT_LEN];
extern uintptr_t intr_vectors[];

void temp_interrupt_handler() {
    panic("INTERRUPTED!");
}

struct {
    uint16_t len;
    struct idt_entry *addr;
} __attribute__ ((packed)) idtr;

intr_handler_t temp_int3_handler(struct intr_frame *frame) {
    panic("BREAKPOINT");
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

    bind_interrupt(3, &temp_int3_handler);

    lidt(&idtr);
}

intr_handler_t bind_interrupt(uint32_t num, intr_handler_t fn) {
    intr_handler_t old = intr_handlers[num];
    intr_handlers[num] = fn;
    return old;
}

struct intr_frame* interrupt_handler(struct intr_frame *frame) {
    if(intr_handlers[frame->vec_no])
        return intr_handlers[frame->vec_no](frame);

    panic("UNHANDLED INTERRUPT OCCURRED!");
}
