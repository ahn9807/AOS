#pragma once

#include "list.h"
#include "thread.h"

void sched_init();
struct thread_info *sched_next();
void sched_push(struct thread_info *th);
void sched_do();
void sched_tick();
void sched_set_idle(struct thread_info* idle);