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

static int setup_stack(struct intr_frame *_if, int argc, char** argv, char** envp);
static int argv_length(char **path);
static char** parse_argv(char *f_name);

void panic_() {
	*(uint16_t *)(0x123123123123) = 0;
	panic("halt");
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

	if(elf_load(parsed_input[0], &_if)) {
		return -1;
	}
	setup_stack(&_if, argv_length(parsed_input), parsed_input, NULL);
	vmm_activate(thread_current()->p4);
	kfree(parsed_input);
	printf("start process %s\n", parsed_input[0]);
	do_iret(&_if);
	NOT_REACHED();
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

static int setup_stack(struct intr_frame *_if, int argc, char** argv, char** envp) {
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

	// PUSH STACK _AUX[]
	push_aux(NULL, 0);

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