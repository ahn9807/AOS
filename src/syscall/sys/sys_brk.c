#include "syscalls.h"
#include "memory.h"
#include "vmm.h"
#include "pmm.h"

SYSCALL_DEFINE1(12, brk, unsigned long, brk) {
	// BRK can has NULL as an brk to access start_brk
	if(!mm_is_user((void *)brk)) {
		return -EINVAL;
	}

	uintptr_t cur_brk = thread_current_s()->owner->brk_end;

	// free
	if(cur_brk >= brk) {
		return thread_current()->owner->brk_end;
	} else if(brk > cur_brk) {
		uintptr_t expend_size = (uintptr_t)brk - cur_brk;
		size_t page_num = expend_size / PAGE_SIZE + 1;
		uintptr_t hpage = (uintptr_t)pmm_alloc_pages(page_num);

		vmm_set_pages(thread_current()->p4,
		 			  thread_current()->owner->brk_end, 
					  hpage, 
					  PAGE_USER_ACCESSIBLE | PAGE_WRITE | PAGE_PRESENT, page_num);
		thread_current()->owner->brk_end = cur_brk + (uintptr_t)PAGE_SIZE * page_num;
	}

	return (long)thread_current()->owner->brk_end;
}