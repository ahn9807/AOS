#include <stdint.h>

static inline void lcr3(uint64_t val)
{
  asm volatile("movq %0,%%cr3" : : "r" (val));
}
