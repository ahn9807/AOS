#include "syscalls.h"
#include "memory.h"
#include "vmm.h"
#include "pmm.h"

#define UTS_SYSNAME "AOS\0"
#define UTS_NODENAME "In Libra\0"
#define UTS_RELEASE "BETA 0.1\0"
#define UTS_VERSION "In Libra\0"
#define UTS_MACHINE "x86_64\0"


struct utsname {
	char *sysname;    /* Operating system name (e.g., "Linux") */
	char *nodename;   /* Name within "some implementation-defined
							network" */
	char *release;    /* Operating system release
							(e.g., "2.6.28") */
	char *version;    /* Operating system version */
	char *machine;    /* Hardware identifier */
};

SYSCALL_DEFINE1(63, uname, void *, buf) {
	struct utsname *uts = (struct utsname *)buf;
	// check validation
	// If failed return EFAULT
	memcpy(uts->sysname, UTS_SYSNAME, 4);
	memcpy(uts->nodename, UTS_NODENAME, 9);
	memcpy(uts->release, UTS_RELEASE, 9);
	memcpy(uts->version, UTS_VERSION, 9);
	memcpy(uts->machine, UTS_MACHINE, 7);

	return 0;
}