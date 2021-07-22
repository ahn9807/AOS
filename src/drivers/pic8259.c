#include "pic8259.h"
#include "port.h"

/*
    IRQ 0 ~ IRQ 7
    System Timer, Keyboard, Slave PIC, COM2 & COM4, COM1 & COM3, LPT2, Floppy, LPT1
    Master PIC Command: 0x20
    Master PIC Data: 0x21

    IRQ 8 ~ IRQ 15
    Real Timer, Reserved, Reserved, Reserved, PS/2, ISA, IDE1, IDE2
    Slave PIC Command: 0xA0
    Slave PIC Data: 0xA1
*/
void pic_init() {
    /* Mask all interrupts on both PICs. */
	outb (0x21, 0xff);
	outb (0xa1, 0xff);

	/* Initialize master. */
	outb (0x20, 0x11); /* ICW1: single mode, edge triggered, expect ICW4. */
	outb (0x21, 0x20); /* ICW2: line IR0...7 -> irq 0x20...0x27. */
	outb (0x21, 0x04); /* ICW3: slave PIC on line IR2. */
	outb (0x21, 0x01); /* ICW4: 8086 mode, normal EOI, non-buffered. */

	/* Initialize slave. */
	outb (0xa0, 0x11); /* ICW1: single mode, edge triggered, expect ICW4. */
	outb (0xa1, 0x28); /* ICW2: line IR0...7 -> irq 0x28...0x2f. */
	outb (0xa1, 0x02); /* ICW3: slave ID is 2. */
	outb (0xa1, 0x01); /* ICW4: 8086 mode, normal EOI, non-buffered. */

	/* Unmask all interrupts. */
	outb (0x21, 0x00);
	outb (0xa1, 0x00);
}

void pic_end_of_interrupt(int irq) {
	// ASSERT (irq >= 0x20 && irq < 0x30);

	/* Acknowledge master PIC. */
	outb (0x20, 0x20);

	/* Acknowledge slave PIC if this is a slave interrupt. */
	if (irq >= 0x28)
		outb (0xa0, 0x20);
}

void pic_disable() {
    /* Mask all interrupts on both PICs. */
	outb (0x21, 0xff);
	outb (0xa1, 0xff);
}