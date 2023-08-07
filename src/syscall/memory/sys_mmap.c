#include "lib/compiler.h"
#include "lib/debug.h"
#include "lib/errno.h"
#include "lib/types.h"
#include "memory.h"
#include "syscalls.h"
#include "thread.h"
#include "vmem.h"

#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0x4000000
#endif /* !MAP_UNINITIALIZED */

#define VALID_PROT_MASK (PROT_READ | PROT_WRITE | PROT_EXEC)

#define USE_BPF

static int do_mmap(void **addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	struct mm_struct *mm = thread_current_s()->mm;
	vaddr_t vaddr;
	const struct uk_vma_ops *vops;
	int rc;

	if (unlikely(len == 0))
		return -EINVAL;

	/* len will overflow when aligning it to page size */
	if (unlikely(len > SIZE_MAX - PAGE_SIZE))
		return -ENOMEM;

	if (unlikely(offset < 0 || !PAGE_ALIGNED(offset)))
		return -EINVAL;

	if (unlikely((size_t)offset > SIZE_MAX - len))
		return -EOVERFLOW;

	if (*addr) {
		if (flags & MAP_FIXED)
			PANIC("NOT IMPLEMENTED");

		vaddr = (vaddr_t)*addr;
	} else {
		/* Linux returns -EPERM if addr is NULL for a fixed mapping */
		if (unlikely(flags & MAP_FIXED))
			return -EPERM;

		if (unlikely(flags & MAP_FIXED_NOREPLACE))
			return -EPERM;

		vaddr = 0;
	}

	if (flags & MAP_ANONYMOUS) {
		if ((flags & MAP_SHARED) || (flags & MAP_SHARED_VALIDATE) == MAP_SHARED_VALIDATE) {
			/* MAP_SHARED(_VALIDATE): Note, we ignore it for
			 * anonymous memory since we only have a single
			 * process. There is no one to share the mapping with.
			 * It is thus fine to create a private mapping.
			 */
		} else if (unlikely(!(flags & MAP_PRIVATE)))
			return -EINVAL;

		/* We do not support unrestricted VMAs */
		if (unlikely(flags & MAP_GROWSDOWN))
			return -ENOTSUP;

		if (flags & MAP_UNINITIALIZED)
			PANIC("NOT IMPLEMENTED");

		if (flags & MAP_HUGETLB) {
			return -EINVAL;
		}
	} else {
		return -ENOTSUP;
	}

	/* Linux will always align len to the selected page size */
	len = PAGE_ALIGN(len);

	if (flags & MAP_POPULATE)
		PANIC("NOT IMPLEMENTED");

	/* MAP_LOCKED: Ignored for now */
	/* MAP_NONBLOCKED: Ignored for now */
	/* MAP_NORESERVE : Ignored for now */

	// need lock
	rc = vma_map(mm, &vaddr, len, prot, flags, NULL);
	// need unlock

	*addr = (void *)vaddr;

	return rc;
}

SYSCALL_DEFINE6(9, mmap, void *, addr, size_t, len, int, prot, int, flags, int, fd, off_t, offset)
{
	int ret;
	ret = do_mmap(&addr, len, prot, flags, fd, offset);

	if (unlikely(ret)) {
		return (long)-ret;
	}

	return (long)addr;
}

SYSCALL_DEFINE3(10, mprotect, void *, addr, size_t, len, int, prot)
{
	struct mm_struct *mm = thread_current_s()->mm;
	vaddr_t vaddr = (vaddr_t)addr;
	int rc;

	rc = vma_set_prot(mm, vaddr, PAGE_ALIGN(len), prot);
	if (unlikely(rc)) {
		if (rc == -ENOENT)
			return -ENOMEM;

		return rc;
	}

	return 0;
}

SYSCALL_DEFINE2(11, munmap, void *, addr, size_t, len)
{
	struct mm_struct *mm = thread_current_s()->mm;
	int rc;

	if (unlikely(len == 0))
		return -EINVAL;

	rc = vma_unmap(mm, (vaddr_t)addr, PAGE_ALIGN(len));
	if (unlikely(rc)) {
		if (rc == -ENOENT)
			return 0;

		return rc;
	}
	return 0;
}

SYSCALL_DEFINE3(28, madvise, void *, addr, size_t, len, int, advice)
{
	//	printf("madvise(%llx, %lx, %d)\n", (uint64_t) addr, len, advice);
	struct mm_struct *mm = thread_current_s()->mm;
	vaddr_t vaddr = (vaddr_t)addr;
	int rc;

	rc = vma_advise(mm, vaddr, len, advice);

	if (unlikely(rc)) {
		if (rc == -ENOENT)
			return -ENOMEM;

		return rc;
	}

	return 0;
}