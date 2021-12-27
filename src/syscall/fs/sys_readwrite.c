#include "syscalls.h"
#include "kmalloc.h"
#include "memory.h"

struct iovec {
	void  *iov_base;    /* Starting address */
	size_t iov_len;     /* Number of bytes to transfer */
};

SYSCALL_DEFINE3(1, write, uint64_t, fd, const char *, buf, size_t, size) {
	char *buffer = kmalloc(size + 1);
	memcpy(buffer, buf, size);
	buffer[size] = '\0';
	printf("%s", buffer);
	kfree(buffer);

	return 0;
}

SYSCALL_DEFINE2(20, writev, const void, *iov, int, iovcnt) {
	struct iovec *iovec = (struct iovec*)iov;
	printf("%s", iovec->iov_base);
	return 0;
}