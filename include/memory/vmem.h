#pragma once

#include "lib/debug.h"
#include "lib/list.h"
#include "memory.h"

#define VADDR_MAX -1UL
/** Invalid virtual addres. **/
#define VADDR_INV -1UL

#define VADDR_BASE 0x1000

#define PROT_READ 0x1 /* Page can be read.  */
#define PROT_WRITE 0x2 /* Page can be written.  */
#define PROT_EXEC 0x4 /* Page can be executed.  */
#define PROT_NONE 0x0 /* Page can not be accessed.  */
#define PROT_GROWSDOWN                                                                                                 \
	0x01000000 /* Extend change to start of
					   growsdown vma (mprotect only).  */
#define PROT_GROWSUP                                                                                                   \
	0x02000000 /* Extend change to start of
					   growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED 0x01 /* Share changes.  */
#define MAP_PRIVATE 0x02 /* Changes are private.  */
#define MAP_SHARED_VALIDATE 0x03 /* Share changes and validate extension flags.  */
#define MAP_TYPE 0x0f /* Mask for type of mapping.  */

/* Other flags.  */
#define MAP_FIXED 0x10 /* Interpret addr exactly.  */
#define MAP_FILE 0
#define MAP_ANONYMOUS 0x20 /* Don't use a file.  */
#define MAP_ANON MAP_ANONYMOUS
#define MAP_HUGE_SHIFT 26
#define MAP_HUGE_MASK 0x3f

#define MAP_GROWSDOWN 0x00100 /* Stack-like segment.  */
#define MAP_DENYWRITE 0x00800 /* ETXTBSY.  */
#define MAP_EXECUTABLE 0x01000 /* Mark it as an executable.  */
#define MAP_LOCKED 0x02000 /* Lock the mapping.  */
#define MAP_NORESERVE 0x04000 /* Don't check for reservations.  */
#define MAP_POPULATE 0x08000 /* Populate (prefault) pagetables.  */
#define MAP_NONBLOCK 0x10000 /* Do not block on IO.  */
#define MAP_STACK 0x20000 /* Allocation is for a stack.  */
#define MAP_HUGETLB 0x40000 /* Create huge page mapping.  */
#define MAP_SYNC 0x80000 /* Perform synchronous page faults for the mapping.  */
#define MAP_FIXED_NOREPLACE 0x100000 /* MAP_FIXED but do not unmap underlying mapping.  */

/* Flags to `msync'.  */
#define MS_ASYNC 1 /* Sync memory asynchronously.  */
#define MS_SYNC 4 /* Synchronous memory sync.  */
#define MS_INVALIDATE 2 /* Invalidate the caches.  */

/* Advice to `madvise'.  */
#define MADV_NORMAL 0 /* No further special treatment.  */
#define MADV_RANDOM 1 /* Expect random page references.  */
#define MADV_SEQUENTIAL 2 /* Expect sequential page references.  */
#define MADV_WILLNEED 3 /* Will need these pages.  */
#define MADV_DONTNEED 4 /* Don't need these pages.  */
#define MADV_FREE 8 /* Free pages only if memory pressure.  */
#define MADV_REMOVE 9 /* Remove these pages and resources.  */
#define MADV_DONTFORK 10 /* Do not inherit across fork.  */
#define MADV_DOFORK 11 /* Do inherit across fork.  */
#define MADV_MERGEABLE 12 /* KSM may merge identical pages.  */
#define MADV_UNMERGEABLE 13 /* KSM may not merge identical pages.  */
#define MADV_HUGEPAGE 14 /* Worth backing with hugepages.  */
#define MADV_NOHUGEPAGE 15 /* Not worth backing with hugepages.  */
#define MADV_DONTDUMP 16 /* Explicity exclude from the core dump, overrides the coredump filter bits.  */
#define MADV_DODUMP 17 /* Clear the MADV_DONTDUMP flag.  */
#define MADV_WIPEONFORK 18 /* Zero memory on fork, child only.  */
#define MADV_KEEPONFORK 19 /* Undo MADV_WIPEONFORK.  */
#define MADV_COLD 20 /* Deactivate these pages.  */
#define MADV_PAGEOUT 21 /* Reclaim these pages.  */
#define MADV_HWPOISON 100 /* Poison a page for testing.  */

/* The POSIX people had to invent similar names for the same things.  */
#define POSIX_MADV_NORMAL 0 /* No further special treatment.  */
#define POSIX_MADV_RANDOM 1 /* Expect random page references.  */
#define POSIX_MADV_SEQUENTIAL 2 /* Expect sequential page references.  */
#define POSIX_MADV_WILLNEED 3 /* Will need these pages.  */
#define POSIX_MADV_DONTNEED 4 /* Don't need these pages.  */

#define VADDR_MAX -1UL
/** Invalid virtual addres. **/
#define VADDR_INV -1UL

#define VADDR_BASE 0x1000

struct mm_struct {
	vaddr_t vma_base;
	struct list_head vma_list;
	unsigned long flags;
};

void debug_mm_struct(struct mm_struct *mm);

struct vm_area_struct {
	vaddr_t vm_start;
	vaddr_t vm_end;

	struct list_head list;

	const struct vm_operation_struct *vm_ops;

	struct mm_struct *vm_mm; /* The address space we belongs to. */
	unsigned long vm_page_prot; /* Access permissions of this VMA. */
	unsigned long vm_flags;

	const char *name; /* Optional field for the VMA. */
};

/**
 * Returns the length of a VMA in bytes.
 */
static inline size_t vmem_vma_len(struct vm_area_struct *vma)
{
	ASSERT(vma->vm_end > vma->vm_start);

	return vma->vm_end - vma->vm_start;
}

/*
 * vm_fault is filled by the pagefault handler and passed to the vma's
 * ->fault function. The vma's ->fault is responsible for returning a bitmask
 * of VM_FAULT_xxx flags that give details about how the fault was handled.
 *
 * MM layer fills up gfp_mask for page allocations but fault handler might
 * alter it if its implementation requires a different allocation context.
 *
 * pgoff should be used in favour of virtual_address, if possible.
 */
struct vm_fault {
	vaddr_t vaddr;
	size_t len;
	unsigned int flags;
	struct mm_struct *mm;
};

struct vm_operation_struct {
	void (*open)(struct vm_area_struct *area);
	/**
	 * @close: Called when the VMA is being removed from the MM.
	 */
	void (*close)(struct vm_area_struct *vma);
	/**
	 * @mmap: Called when the VMA is being mapped to the MM.
	 *
	 * @param vma
	 *	 The VMA in which a range should be mapped. 
	*/
	int (*mmap)(struct vm_area_struct *vma);
	/**
	 * Unmaps a range of the VMA. It is the handler's responsibily to
	 * perform the actual unmap in the page table and potentially release
	 * the physical memory.
	 *
	 * @param vma
	 *   The VMA in which a range should be unmapped
	 * @param vaddr
	 *   The base address of the range which should be unmapped
	 * @param len
	 *   The length of the range in bytes
	 *
	 * @return
	 *   0 on success, a negative errno error otherwise
	 */
	int (*unmap)(struct vm_area_struct *vma, vaddr_t vaddr, size_t len);
	/*
	 * Called by mprotect() to make driver-specific permission
	 * checks before mprotect() is finalised.   The VMA must not
	 * be modified.  Returns 0 if mprotect() can proceed.
	 */
	int (*mprotect)(struct vm_area_struct *vma, unsigned long newflags);
	int (*fault)(struct vm_area_struct *vma, struct vm_fault *vmf);
	/* Called by the /proc/PID/maps code to ask the vma whether it
	 * has a special name.  Returning non-NULL will also cause this
	 * vma to be dumped unconditionally. */
	const char *(*name)(struct vm_area_struct *vma);
};

/* Platform vm_operation_struct handlers */
int vma_op_plat_unmap(struct vm_area_struct *vma, vaddr_t vaddr, size_t len);
int vma_op_plat_mmap(struct vm_area_struct *vma);
int vma_op_plat_set_prot(struct vm_area_struct *vma, unsigned long prot);

/* defined at the vmops_anon.c */
extern const struct vm_operation_struct vma_anon_ops;

int vma_map(struct mm_struct *mm, vaddr_t *vaddr, size_t len, unsigned long prot, unsigned long flags,
	    const char *name);
int vma_unmap(struct mm_struct *mm, vaddr_t vaddr, size_t len);
int vma_set_prot(struct mm_struct *mm, vaddr_t vaddr, size_t len, unsigned long prot);
int vma_advise(struct mm_struct *mm, vaddr_t vaddr, size_t len, unsigned long advice);
struct mm_struct *mm_get_active(void);
int mm_init(struct mm_struct *vas);
void mm_destory(struct mm_struct *mm);