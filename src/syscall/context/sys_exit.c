#include "syscalls.h"

SYSCALL_DEFINE1(60, exit, int, error_code) {
	thread_current()->status = error_code;
	thread_exit();
}