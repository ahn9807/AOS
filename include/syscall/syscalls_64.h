#pragma once

#include "syscalls.h"

/* This file is included at the end of the syscall initialization
	const sys_call_ptr_t sys_call_table[__NR_syscall_max+1] = {
    [0 ... __NR_syscall_max] = &sys_ni_syscall,
    [0] = sys_read,
    [1] = sys_write,
    [2] = sys_open,
    ...
    ...
    ...
}; */

#define __SYSCALL_COMMON(nr, sym) __SYSCALL_64(nr, sym)
#define __SYSCALL_64(nr, sym) [nr] = sym,

__SYSCALL_COMMON(1, sys_write)
__SYSCALL_COMMON(12, sys_brk)
__SYSCALL_COMMON(20, sys_writev)
__SYSCALL_COMMON(21, sys_access)
__SYSCALL_COMMON(60, sys_exit)
__SYSCALL_COMMON(63, sys_uname)
__SYSCALL_COMMON(102, sys_getuid)
__SYSCALL_COMMON(107, sys_geteuid)
__SYSCALL_COMMON(158, sys_arch_prctl)