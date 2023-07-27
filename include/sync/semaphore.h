#pragma once

#include "lib/list.h"

struct semaphore {
    unsigned int value;
    struct list_head waiters;
};

void sema_init(struct semaphore *, unsigned int value);
void sema_down(struct semaphore *);
bool sema_try_down(struct semaphore *);
void sema_up(struct semaphore *);