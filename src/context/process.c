#include "process.h"
#include "thread.h"
#include "interrupt.h"
#include "layout.h"
#include "cpu_flags.h"
#include "elf.h"
#include "debug.h"
#include "vmm.h"
#include "tss.h"


void panic_() {
	*(uint16_t *)(0x123123123123) = 0;
	panic("halt");
}

int process_exec(char *f_name) {
	struct intr_frame _if;
	memset(&_if, 0, sizeof(struct intr_frame));
	_if.ds = _if.es = _if.ss = SEL_UDSEG;
	_if.cs = SEL_UCSEG;
	_if.eflags = FLAG_IF | thread_current()->thread_frame.eflags;

	if(elf_load(f_name, &_if)) {
		return -1;
	}
	cls();
	printf("result: 0x%x\n", *(uint64_t *)(vmm_get_page(thread_current()->p4, _if.rip)));
	vmm_activate(thread_current()->p4);
	printf("0x%x\n", *(char *)_if.rip);
	// PANIC("HALT");

	do_iret(&_if);
	NOT_REACHED();
}