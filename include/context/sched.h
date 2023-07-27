#pragma once

#include "interrupt.h"
#include "lib/list.h"
#include "lib/time.h"
#include "thread.h"

// Helper functions for scheduler (pintos!)
#define F 16384
#define REAL_TO_INT(x) ((x) / F)
#define INT_TO_REAL(n) ((n)*F)
#define REAL_TO_INT_ROUND(x) ((x) >= 0 ? (((x) + F / 2)) / F : (((x)-F / 2)) / F)
#define ADD_REAL(x, y) ((x) + (y))
#define SUBTRACT_REAL(x, y) ((x) - (y))
#define ADD_REAL_INT(x, n) ((x) + (n)*F)
#define SUBTRACT_REAL_INT(x, n) ((x) - (n)*F)
#define MUL_REAL(x, y) (((int64_t)(x)) * (y) / F)
#define MUL_REAL_INT(x, n) ((x) * (n))
#define DIV_REAL(x, y) (((int64_t)(x)) * F / (y))
#define DIV_REAL_INT(x, n) ((x) / n)

#define PRIORITY_MAX 61
#define PRIORITY_MIN 0
#define PRIORITY_DEFAULT

#define NICE_MAX 31
#define NICE_MIN -31
#define NICE_DEFAULT 0

#define RECENT_CPU_DEFAULT 0

#define SCHED_TIMER_TICK_INTVAL 4
#define SCHED_TIMER_MLFQ_INTVAL 100

void sched_init();
struct thread_info *sched_next();
void sched_push_ready(struct thread_info *th);
void sched_do();
enum irq_handler_result sched_tick();
void sched_set_idle(struct thread_info *idle);
void sched_sleep(timespec_t tm);

void sched_set_nice(int nice);
int sched_get_nice();