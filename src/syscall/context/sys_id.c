#include "syscalls.h"

SYSCALL_DEFINE0(107, geteuid) {
	return thread_current_s()->owner->euid;
}

SYSCALL_DEFINE0(102, getuid) {
	return thread_current_s()->owner->uid;
}