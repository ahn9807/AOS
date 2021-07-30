#include "syscalls.h"

SYSCALL_DEFINE2(21, access, const char *, pathname, int, mode) {
	printf("%s %d\n", pathname, mode);

	return -1;
}