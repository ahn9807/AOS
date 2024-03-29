#include "lib/debug.h"
#include "interrupt.h"
#include "lib/stdarg.h"
#include "lib/stddef.h"
#include "vga_text.h"

void debug_panic(const char *file, int line, const char *function, const char *message, ...)
{
	static int level;
	va_list args;

	intr_disable();

	level++;
	if (level == 1) {
		printf("Kernel PANIC at %s:%d in %s(): ", file, line, function);

		va_start(args, message);
		printf(message, va_arg(args, char *));
		printf("\n");
		va_end(args);

		debug_backtrace();
	} else if (level == 2)
		printf("Kernel PANIC recursion at %s:%d in %s().\n", file, line, function);
	else {
		/* Don't print anything: that's probably why we recursed. */
	}

	for (;;)
		;
}

void debug_backtrace(void)
{
	void **frame;

	printf("Call stack:");
	int level = 0;
	for (frame = __builtin_frame_address(0); frame != NULL && frame[0] != NULL; frame = frame[0]) {
		printf(" 0x%x", frame[1]);
		level++;
		if (level == MAX_BACKTRACE_LEVEL) {
			printf("\nBacktrace stack is now over %d\nPlease check stack pointer for overflowing",
			       MAX_BACKTRACE_LEVEL);
			break;
		}
	}
	printf(".\n");
}