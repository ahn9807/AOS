#include "process.h"
#include "thread.h"
#include "interrupt.h"
#include "layout.h"
#include "cpu_flags.h"
#include "elf.h"
#include "debug.h"

int process_exec(char *f_name) {
	struct intr_frame _if;
	_if.ds = _if.es = _if.ss = SEL_UDSEG;
	_if.cs = SEL_UCSEG;
	_if.eflags = FLAG_IF;

	if(elf_load(f_name, &_if) == false) {
		return -1;
	}

	do_iret(&_if);
	NOT_REACHED();		
}

static void cleanup(void) {
	struct thread_info *thread = thread_current_s();

	if(thread->p4 != NULL) {
				
	}
}