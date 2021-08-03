#include "keyboard.h"
#include "interrupt.h"
#include "port.h"
#include "pic8259.h"
#include "vfs.h"

static enum irq_handler_result keyboard_handler(struct intr_frame *f) {
    uint8_t scan_code = inb(0x60);

    // Key Released
    if(scan_code & 0x80) {
    }
    // Key Pressed
    else {
        printf("%c", kbdus[scan_code]);
    }

    pic_end_of_interrupt(f->vec_no);
}

void keyboard_init() {
    bind_interrupt_with_name(0x21, &keyboard_handler, "Keyboard");
}

DEVICE_INSTALL(keyboard, &keyboard_init);