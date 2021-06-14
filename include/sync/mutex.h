#pragma once

#include "list.h"
#include "stdbool.h"
#include "semaphore.h"


struct mutex {
    struct semaphore sema;
};

void mutex_init(struct mutex *);
void mutex_down(struct mutex *);
bool mutex_try_down(struct mutex *);
void mutex_up(struct mutex *);