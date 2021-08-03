#pragma once

#include "thread.h"
#include "errno.h"

#define MAX_SYSCALL_NR 255

#define __MAP0(m, ...)
#define __MAP1(m, t, a, ...) m(t, a)
#define __MAP2(m, t, a, ...) m(t, a), __MAP1(m, __VA_ARGS__)
#define __MAP3(m, t, a, ...) m(t, a), __MAP2(m, __VA_ARGS__)
#define __MAP4(m, t, a, ...) m(t, a), __MAP3(m, __VA_ARGS__)
#define __MAP5(m, t, a, ...) m(t, a), __MAP4(m, __VA_ARGS__)
#define __MAP6(m, t, a, ...) m(t, a), __MAP5(m, __VA_ARGS__)
#define __MAP(n, ...) __MAP##n(__VA_ARGS__)

#define __SC_DECL(t, a) t a

typedef union syscall_ptr
{
	long (*syscall_arg0)(void);
	long (*syscall_arg1)(unsigned long);
	long (*syscall_arg2)(unsigned long, unsigned long);
	long (*syscall_arg3)(unsigned long, unsigned long, unsigned long);
	long (*syscall_arg4)(unsigned long, unsigned long, unsigned long, unsigned long);
	long (*syscall_arg5)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
	long (*syscall_arg6)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
} syscall_ptr_t;

typedef struct syscall_info {
	const char *syscall_name;
	uint32_t syscall_nr;
	uint32_t arg_nr;
	syscall_ptr_t syscall_p;
} syscall_info_t;

#define __SYSCALL_DEFINE_METADATA(nr, x, name) 		\
    struct syscall_info __syscall_info_sys##name = { 			\
        .syscall_name = "sys"#name, 							\
        .syscall_nr = nr, 										\
        .arg_nr = x, 											\
        .syscall_p = NULL,										\
    };															\
	struct syscall_info __attribute__((section("__syscalls_info"))) \
	*__p_syscall_meta_##name = &__syscall_info_sys##name;

#define __SYSCALL_DEFINEx(nr, x, name, ...) \
	__SYSCALL_DEFINE_METADATA(nr, x, name) \
	long sys##name(__MAP(x, __SC_DECL, __VA_ARGS__))

#define SYSCALL_DEFINE0(nr, name) \
	__SYSCALL_DEFINE_METADATA(nr, 0, name) \
	long sys_##name()
#define SYSCALL_DEFINE1(nr, name, ...) __SYSCALL_DEFINEx(nr, 1, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE2(nr, name, ...) __SYSCALL_DEFINEx(nr, 2, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE3(nr, name, ...) __SYSCALL_DEFINEx(nr, 3, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE4(nr, name, ...) __SYSCALL_DEFINEx(nr, 4, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE5(nr, name, ...) __SYSCALL_DEFINEx(nr, 5, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE6(nr, name, ...) __SYSCALL_DEFINEx(nr, 6, _##name, __VA_ARGS__)

#define SYSCALL_DEFINE_MAXARGS 6

#define VALIDATE_PTR(PTR) \
	do { \
		if(syscall_validate_ptr((uintptr_t)(PTR))) return -EINVAL; \
	} while(0)

int syscall_validate_ptr(uintptr_t ptr);

/// Initialize the system call tables
void syscall_init();

// From this, definition of system calls goes by

/* Default system call */
long sysc_ni();

/* syscall/context */
long sys_exit(int);

/* syscall/fs */
long sys_write(uint64_t, const char *, size_t); // 01
long sys_writev(const void *, int);
long sys_access(const char *, int); // 21

/* syscall/futex */

/* syscall/net */

/* syscall/proc */

/* syscall/sys */
long sys_brk(unsigned long); // 12
long sys_uname(void *);
long sys_getuid(); // 102
long sys_geteuid(); // 107
long sys_arch_prctl(int, unsigned long);