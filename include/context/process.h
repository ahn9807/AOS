#pragma once

#include <stdint.h>
#include <stddef.h>
#include "thread.h"

int process_exec();
tid_t process_fork();
tid_t process_wait();
tid_t process_exit();