#pragma once

#include <stdint.h>
#include <stddef.h>
#include "list.h"
#include "elf.h"

typedef uint32_t pid_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint32_t tid_t;

typedef struct process_info {
	// process id
	pid_t pid;

	// exist status
	int status;

	// Associated stat information
	uid_t uid;
	uid_t euid;
	gid_t gid;

	// Opened file descriptors
	struct list fdescs;

	// Holding threads
	struct list threads;

	// Elf Informations
	struct ELF64_Phdr *phdrs;
	struct ELF64_Ehdr *ehdr;

	// User Heap area (brk)
	uintptr_t brk_start;
	uintptr_t brk_end;
} process_info_t;

int process_exec();
tid_t process_fork();
tid_t process_wait();
tid_t process_exit();