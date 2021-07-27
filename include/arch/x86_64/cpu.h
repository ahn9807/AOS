#pragma once

#include <stdint.h>
#define MAX_CPU_NUM 128

static struct cpu_info __seg_gs * const current_cpu_ = 0;

typedef struct cpu_info {
	int cpuid;
	int lapicid;
	int cpu_model;
	int family;
	int cpu_model_name[48];
	const char *cpu_manufacturer;
} cpu_info_t;

extern struct cpu_info cpu_info_table[MAX_CPU_NUM];
extern size_t num_of_cpu;
extern uintptr_t lapic_base_addr;
extern uintptr_t ioapic_base_addr;

#define current_cpu() ((struct cpu_info *)current_cpu_)

void cpu_init();
void cpu_tlb_shootdown();
uint64_t cpu_read_tsc();
void debug_cpu();