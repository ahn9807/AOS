#include "syscalls.h"
#include "msr_flags.h"

#define ARCH_SET_GS		0x1001
#define ARCH_SET_FS		0x1002
#define ARCH_GET_FS		0x1003
#define ARCH_GET_GS		0x1004

#define ARCH_GET_CPUID		0x1011
#define ARCH_SET_CPUID		0x1012

#define ARCH_MAP_VDSO_X32	0x2001
#define ARCH_MAP_VDSO_32	0x2002
#define ARCH_MAP_VDSO_64	0x2003

SYSCALL_DEFINE2(158, arch_prctl, int, code, unsigned long, addr) {
	switch (code)
	{
		case ARCH_SET_GS:
			write_msr(MSR_GS_BASE, addr);
			break;
		case ARCH_SET_FS:
			write_msr(MSR_FS_BASE, addr);
			break;
		case ARCH_GET_FS:
			return -ENOTSUP;
			break;
		case ARCH_GET_GS:
			return -ENOTSUP;
			break;
		default:
			return -EINVAL;
			break;
	}

	return 0;
}