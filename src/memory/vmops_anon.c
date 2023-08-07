#include "lib/debug.h"
#include "memory.h"
#include "vmem.h"

int vma_op_mmap(struct vm_area_struct *vma)
{
	ASSERT(vma != NULL);

	return 0;
}

int vma_op_unmap(struct vm_area_struct *vma, vaddr_t vaddr, size_t len)
{
	ASSERT(vma != NULL);
	ASSERT(vaddr == vma->vm_start);
	ASSERT(vaddr + len == vma->vm_end);
	ASSERT(PAGE_ALIGNED(len));

	PANIC("NOT IMPLEMENTED");
}

int vma_op_set_prot(struct vm_area_struct *vma, unsigned long prot)
{
	PANIC("NOT IMPLEMENTED");
}

const struct vm_operation_struct vma_anon_ops = {
	.open = NULL,
	.close = NULL,
	.mmap = vma_op_mmap,
	.unmap = vma_op_unmap,
	.mprotect = vma_op_set_prot,
};
