#include "msr_flags.h"
#include "cpu_flags.h"
#include "register_flags.h"
#include "syscalls.h"
#include "intrinsic.h"
#include "layout.h"
#include "thread.h"
#include "debug.h"
#include "errno.h"
#include "kmalloc.h"

static void debug_syscall(struct intr_frame *if_);
const syscall_ptr_t syscall_table[MAX_SYSCALL_NR + 1];
void syscall_entry();

void syscall_init() {
	write_msr(MSR_EFER, MSR_SCE | MSR_LME);
	write_msr(MSR_STAR, (((uint64_t)SEL_UCSEG - 0x10) << 48) | ((uint64_t)SEL_KCSEG << 32));
	write_msr(MSR_LSTAR, (uint64_t)syscall_entry);
	write_msr(MSR_SF_MASK, FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL1 | FLAG_AC | FLAG_NT);
}

void syscall_handler(struct intr_frame *if_) {
	long ret =syscall_table[if_->reg.rax](if_->reg.rdi, if_->reg.rsi, if_->reg.rdx, if_->reg.r10, if_->reg.r8, if_->reg.r9);
	if_->reg.rax = ret;
}

SYSCALL_DEFINE3(write, uint64_t, fd, char *, buf, size_t, size) {
	char *buffer = kmalloc(size + 1);
	memcpy(buffer, buf, size);
	buffer[size] = '\0';
	printf("%s", buffer);
	kfree(buffer);

	return 0;
}

SYSCALL_DEFINE0(ni) {
	PANIC("UNREGISTERED SYSTEM CALL!");
	return -ENOSYS;	
}

const syscall_ptr_t syscall_table[MAX_SYSCALL_NR + 1] = {
	[0 ... MAX_SYSCALL_NR] = &sys_ni,
	#include "syscalls_64.h"
};

static void debug_syscall(struct intr_frame *if_) {
	printf("syscall nr: %d, name: %s\n", if_->reg.rax, "NULL");
	printf("rax: %d rdi: 0x%x, rsi: 0x%x,\nrdx: 0x%x, r10: 0x%x, r8: 0x%x, r9: 0x%x\n",
		if_->reg.rax, if_->reg.rdi, if_->reg.rsi, if_->reg.rdx, if_->reg.r10, if_->reg.r8, if_->reg.r9
	);
}