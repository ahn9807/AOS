#pragma once

#include "lib/list.h"
#include "lib/stddef.h"
#include "semaphore.h"

struct mutex {
	struct semaphore sema;
};

void mutex_init(struct mutex *);
void mutex_down(struct mutex *);
bool mutex_try_down(struct mutex *);
void mutex_up(struct mutex *);