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
static syscall_info_t syscall_info_table[MAX_SYSCALL_NR + 1];
const syscall_ptr_t syscall_func_table[MAX_SYSCALL_NR + 1];

void syscall_entry();
extern uintptr_t _start_syscall_info_table;
extern uintptr_t _end_syscall_info_table;

void syscall_init() {
	write_msr(MSR_EFER, MSR_SCE | MSR_LME);
	write_msr(MSR_STAR, (((uint64_t)SEL_UCSEG - 0x10) << 48) | ((uint64_t)SEL_KCSEG << 32));
	write_msr(MSR_LSTAR, (uint64_t)syscall_entry);
	write_msr(MSR_SF_MASK, FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL1 | FLAG_AC | FLAG_NT);

	// fill default settings for syscall	
	for(int i=0;i<MAX_SYSCALL_NR;i++) {
		syscall_info_table[i].syscall_name = "sys_ni";
		syscall_info_table[i].syscall_p = syscall_func_table[i];
		syscall_info_table[i].arg_nr = 0;
		syscall_info_table[i].syscall_nr = i;
	}

	for(syscall_info_t **cursor = (syscall_info_t **)&_start_syscall_info_table; 
		cursor < &_end_syscall_info_table; 
		cursor = (uintptr_t)cursor + (uintptr_t)sizeof(syscall_info_t **))
	{
		// sys_ni
		if((*cursor)->syscall_nr == -1) {
			continue;
		}

		syscall_info_table[(*cursor)->syscall_nr].syscall_p = syscall_func_table[(*cursor)->syscall_nr]; 
		syscall_info_table[(*cursor)->syscall_nr].syscall_nr = (*cursor)->syscall_nr; 
		syscall_info_table[(*cursor)->syscall_nr].syscall_name = (*cursor)->syscall_name; 
		syscall_info_table[(*cursor)->syscall_nr].arg_nr = (*cursor)->arg_nr; 
	}
}

void syscall_handler(struct intr_frame *if_) {
	if(if_->reg.rax > MAX_SYSCALL_NR) {
		if_->reg.rax = -ENOTSUP;
	} else {
		syscall_info_t *syscall_info = &syscall_info_table[if_->reg.rax];
		long ret = 0;

		debug_syscall(if_);
		switch (syscall_info->arg_nr)
		{
			case 0:
				ret = syscall_info->syscall_p.syscall_arg0();
				break;
			case 1:
				ret = syscall_info->syscall_p.syscall_arg1(if_->reg.rdi);
				break;
			case 2:
				ret = syscall_info->syscall_p.syscall_arg2(if_->reg.rdi, if_->reg.rsi);
				break;
			case 3:
				ret = syscall_info->syscall_p.syscall_arg3(if_->reg.rdi, if_->reg.rsi, if_->reg.rdx);
				break;
			case 4:
				ret = syscall_info->syscall_p.syscall_arg4(if_->reg.rdi, if_->reg.rsi, if_->reg.rdx, if_->reg.r10);
				break;
			case 5:
				ret = syscall_info->syscall_p.syscall_arg5(if_->reg.rdi, if_->reg.rsi, if_->reg.rdx, if_->reg.r10, if_->reg.r8);
				break;
			case 6:
				ret = syscall_func_table[if_->reg.rax].syscall_arg6(if_->reg.rdi, if_->reg.rsi, if_->reg.rdx, if_->reg.r10, if_->reg.r8, if_->reg.r9);
				break;
			default:
				panic("UNDEFINED SYSTEM CALL!");
				break;
			
			if_->reg.rax = (uint64_t)ret;
		}
	}
}

SYSCALL_DEFINE3(1, write, uint64_t, fd, const char *, buf, size_t, size) {
	char *buffer = kmalloc(size + 1);
	memcpy(buffer, buf, size);
	buffer[size] = '\0';
	printf("%s", buffer);
	kfree(buffer);

	return 0;
}

SYSCALL_DEFINE1(60, exit, int, error_code) {
	thread_exit();
}

SYSCALL_DEFINE0(-1, ni) {
	panic("UNREGISTERED SYSTEM CALL!");
	return -ENOSYS;	
}

const syscall_ptr_t syscall_func_table[MAX_SYSCALL_NR + 1] = {
	[0 ... MAX_SYSCALL_NR] = (syscall_ptr_t)&sys_ni,
	#include "syscalls_64.h"
};

static void debug_syscall(struct intr_frame *if_) {
	printf("syscall nr: %d, name: %s\n", if_->reg.rax, "NULL");
	printf("rax: %d rdi: 0x%x, rsi: 0x%x,\nrdx: 0x%x, r10: 0x%x, r8: 0x%x, r9: 0x%x\n",
		if_->reg.rax, if_->reg.rdi, if_->reg.rsi, if_->reg.rdx, if_->reg.r10, if_->reg.r8, if_->reg.r9
	);
}