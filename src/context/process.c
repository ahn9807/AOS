#include "process.h"
#include "thread.h"
#include "interrupt.h"
#include "layout.h"
#include "cpu_flags.h"
#include "elf.h"
#include "debug.h"
#include "vmm.h"
#include "tss.h"
#include "auxv.h"
#include "kmalloc.h"
#include "vfs.h"
#include "elf.h"
#include "msr_flags.h"

#define ROUND_UP(X, STEP) (((X) + (STEP) - 1) / (STEP) * (STEP))

static int setup_stack(struct process_info *proc, struct intr_frame *_if, int argc, char** argv, char** envp);
static int setup_heap(struct process_info *proc, size_t log_heap_size);
static int argv_length(char **path);
static char** parse_argv(char *f_name);
static inline pid_t allocate_pid();

static pid_t current_pid = 0;

void panic_() {
	*(uint16_t *)(0x123123123123) = 0;
	panic("halt");
}

int process_run_idle() {

}

int process_exec(char *f_name) {
	struct intr_frame _if;
	char **parsed_input = parse_argv(f_name);
	if(parsed_input == NULL) {
		return -1;
	}

	memset(&_if, 0, sizeof(struct intr_frame));
	_if.ds = _if.es = _if.ss = SEL_UDSEG;
	_if.cs = SEL_UCSEG;
	_if.eflags = FLAG_IF | 0x2;

	process_info_t *owner = kcalloc(1, sizeof(process_info_t));

	if(elf_load(owner, parsed_input[0], &_if)) {
		return -1;
	}

	tss_update(thread_current_s());
	vmm_activate(thread_current_s()->p4);
	
	owner->pid = allocate_pid();
	owner->uid = 100;
	owner->euid = 100;
	owner->status = -1;
	list_init(&owner->threads);
	list_init(&owner->fdescs);
	list_push_back(&owner->threads, &thread_current()->elem);

	thread_current()->owner = owner;

	setup_stack(owner, &_if, argv_length(parsed_input), parsed_input, NULL);
	setup_heap(owner, PAGE_SIZE);
	
	kfree(parsed_input);

	do_iret(&_if);
	NOT_REACHED();
}

static inline pid_t allocate_pid() {
	return current_pid++;
}

static char** parse_argv(char *path) {
    if(path == NULL || *path == '\0')
        return NULL;

    while (*path == ' ')
        ++path;

    char *tokens = strdup(path);

    if (tokens == NULL)
        return NULL;

    int len = strlen(path);

    if (!len) {
        char **ret = kmalloc(sizeof(char *));
        *ret = NULL;
        return ret;
    }

    int i, count = 0;
    for (i = 0; i < len; ++i) {
        if (tokens[i] == ' ') {
            tokens[i] = 0;
            ++count;
        }
    }

    if (path[len-1] != ' ')
        ++count;

    char **ret = kmalloc(sizeof(char *) * (count + 1));

    int j = 0;
    ret[j++] = tokens;

    for (i = 0; i < strlen(path) - 1; ++i)
        if (tokens[i] == 0)
            ret[j++] = &tokens[i+1];

    ret[j] = NULL;

    return ret;
}

static int argv_length(char **path) {
    int size = 0;
	if(path == NULL) {
		return size;
	}
    for(int i=0;path[i] != NULL; i++) {
        size++;
    }
    return size;
}

static int setup_heap(struct process_info *proc, size_t heap_size) {
	ASSERT(proc->ehdr != NULL);
	ASSERT(proc->phdrs != NULL);

	uintptr_t elf_end_addr = 0;
	size_t page_num = heap_size / 4096 + 1;

	for(int i = 0; i< proc->ehdr->e_phnum;i++) {
		if(proc->phdrs[i].p_type != PT_LOAD) {
			continue;
		}

		if(elf_end_addr < proc->phdrs[i].p_vaddr + proc->phdrs[i].p_memsz) {
			elf_end_addr = ROUND_UP(proc->phdrs[i].p_vaddr + proc->phdrs[i].p_memsz, PAGE_SIZE);
		}
	}

	ASSERT(elf_end_addr != 0);
	proc->brk_start = elf_end_addr;
	proc->brk_end = elf_end_addr + PAGE_SIZE * page_num;

	uintptr_t hpage = pmm_alloc_pages(page_num);

	vmm_set_pages(thread_current()->p4, proc->brk_start, hpage, PAGE_USER_ACCESSIBLE | PAGE_WRITE | PAGE_PRESENT, page_num);

	return 0;
}

static int setup_stack(struct process_info *proc, struct intr_frame *_if, int argc, char** argv, char** envp) {
	ASSERT(proc->ehdr != NULL);
	ASSERT(proc->phdrs != NULL);

	uint8_t* spage = pmm_alloc_pages(USER_STACK_SIZE);
	
	if(spage != NULL) {
		vmm_set_pages(thread_current_s()->p4, USER_STACK - PAGE_SIZE, spage, PAGE_USER_ACCESSIBLE | PAGE_WRITE | PAGE_PRESENT, USER_STACK_SIZE);
		_if->rsp = USER_STACK;
	} else {
		printf("failed to alloc stack\n");
		return -1;
	}

#define push8(x) \
	_if->rsp -= sizeof(uint8_t); \
	*(uint8_t *)_if->rsp = (uint8_t)x;
#define push64(x) \
	_if->rsp -= sizeof(uint64_t); \
	*(uint64_t *)_if->rsp = (uint64_t)x;
#define push_aux(id, val) \
	do { \
		push64(val); \
		push64(id); \
	} while(0)
	
	ASSERT(argc == argv_length(argv));
	
	size_t len = 0;
	int envpc = argv_length(envp);
	uintptr_t *argv_ptr = kmalloc(sizeof(uintptr_t) * strlen(argv));
	uintptr_t *envp_ptr = kmalloc(sizeof(uintptr_t) * strlen(envp));

	if(argv_ptr == NULL || envp_ptr == NULL) {
		return -1;
	}

	// Bottom of stack push NULL
	unsigned char k_rand_bytes[16];
	for(int i=0;i<16;i++) {
		k_rand_bytes[i] = 171717 * i / 255;
	}
	uintptr_t u_random_byte = _if->rsp;
	for(int i=0;i<16;i++) {
		push64(k_rand_bytes[i]);
	}
	push64(NULL);

	// PUSH STACK _AUX[]
	push_aux(AT_NULL, 0);
	push_aux(AT_PHDR, proc->phdrs[0].p_vaddr + sizeof(struct ELF64_Ehdr));
	push_aux(AT_PHENT, proc->ehdr->e_phentsize);
	push_aux(AT_PHNUM, proc->ehdr->e_phnum);
	push_aux(AT_PAGESZ, PAGE_SIZE);
	push_aux(AT_BASE, 0x0);
	push_aux(AT_ENTRY, (uintptr_t)proc->ehdr->e_entry);
	push_aux(AT_NOTELF, 0x0);
	push_aux(AT_UID, 0);
	push_aux(AT_EUID, 0);
	push_aux(AT_GID, 0);
	push_aux(AT_EGID, 0);
	push_aux(AT_RANDOM, u_random_byte);
	push_aux(AT_SECURE, 0);

	// PUSH STACK _ ENVP[][]
	for(int i=envpc -1; i >= 0; i--) {
		envp_ptr = _if->rsp;
		len = strlen(envp[i]);

		for(int j=0;j<len;j++) {
			push8(envp[i][j]);
		}
		push8(NULL);
	}

	// PUSH STACK _ ARGUMENT[][]
	for(int i=argc -1; i >= 0; i--) {
		argv_ptr = _if->rsp;
		len = strlen(argv[i]);

		for(int j=0;j<len;j++) {
			push8(argv[i][j]);
		}
		push8(NULL);
	}

	// PUSH STACK _ PADDING
	while(_if->rsp & 7) {
		_if->rsp--;
	}

	// PUSH STACK _ ENVP[]
	for(int i=envpc-1; i>=0; i--){
		push64(envp_ptr[i]);
	}

	// PUSH STACK _ARGUMENT[]
	for(int i=argc-1; i>0; i--) {
		push64(argv_ptr[i]);
	}

	// PUSH STACK _ ARGC
	push64(argc);

	kfree(argv_ptr);
	kfree(envp_ptr);

	return 0;
}