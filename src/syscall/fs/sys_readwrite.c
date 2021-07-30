#include "syscalls.h"

SYSCALL_DEFINE3(1, write, uint64_t, fd, const char *, buf, size_t, size) {
	char *buffer = kmalloc(size + 1);
	memcpy(buffer, buf, size);
	buffer[size] = '\0';
	printf("%s", buffer);
	kfree(buffer);

	return 0;
}