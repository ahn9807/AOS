#pragma once

#include "lib/stddef.h"

typedef struct spinlock {
    // Is locked?
    bool locked;
} spinlock_t;

void spin_lock_init(spinlock_t *spl);
void spin_lock(spinlock_t *spl);
void spin_unlock(spinlock_t *spl);
bool spin_trylock(spinlock_t *spl);
void spin_lock_irq(spinlock_t *spl);
void spin_unlock_irq(spinlock_t *spl);
bool spin_trylock_irq(spinlock_t *spl);