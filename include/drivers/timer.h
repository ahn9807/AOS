#include "lib/time.h"

#define TIMER_FREQ 100

#if TIMER_FREQ < 19
#error 8254 timer requires TIMER_FREQ >= 19
#endif
#if TIMER_FREQ > 1000
#error TIMER_FREQ <= 1000 recommended
#endif

void timer_init();
time_t timer_ticks();
time_t timer_real_time();