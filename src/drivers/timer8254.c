#include "timer.h"
#include "interrupt.h"
#include "port.h"
#include "sched.h"

static time_t ticks = 0;
static enum irq_handler_result timer_interrupt(struct intr_frame *f)
{
	ticks++;
	return sched_tick();
}

void timer_init()
{
	/* 8254 input frequency divided by TIMER_FREQ, rounded to
   nearest. */
	uint16_t count = (1193180 + TIMER_FREQ / 2) / TIMER_FREQ;

	outb(0x43, 0x34); /* CW: counter 0, LSB then MSB, mode 2, binary. */
	outb(0x40, count & 0xff);
	outb(0x40, count >> 8);

	bind_interrupt_with_name(0x20, timer_interrupt, "8254 Timer");
}

time_t timer_ticks()
{
	enum intr_level old_level = intr_disable();
	int64_t t = ticks;
	intr_set_level(old_level);

	return t;
}

time_t timer_real_time();