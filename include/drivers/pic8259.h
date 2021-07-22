#pragma once

void pic_init();
void pic_disable();
void pic_end_of_interrupt (int irq);

#include <time.h>