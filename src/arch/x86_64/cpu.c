#include <stdint.h>
#include <stddef.h>
#include "cpu.h"

#define cpuid(in,a,b,c,d) do { asm volatile ("cpuid" : "=a"(a),"=b"(b),"=c"(c),"=d"(d) : "a"(in)); } while(0)

struct cpu_info cpu_info_table[MAX_CPU_NUM];
size_t num_of_cpu;
uintptr_t lapic_base_addr;
uintptr_t ioapic_base_addr;

void send_ipi();
void get_cpu_info();
void per_cpu();