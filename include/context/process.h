#pragma once

#include <stdint.h>
#include <stddef.h>
#include "thread.h"

typedef uint32_t pid_t;

typedef struct process_info {
	pid_t pid;
	pid_t group;
	pid_t job;
	int status;
} process_info_t;

int process_exec();
tid_t process_fork();
tid_t process_wait();
tid_t process_exit();