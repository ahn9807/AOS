#include "process.h"
#include "thread.h"
#include "interrupt.h"
#include "layout.h"

tid_t process_exec(char *f_name) {
	struct intr_frame _if;
	_if.ds = _if.es = _if.ss = SEL_UDSEG;
}