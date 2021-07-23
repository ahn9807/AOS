#pragma once

#include <stdint.h>
#define MAX_CPU_NUM 128

typedef struct cpu_info {
	int cpuid;
	int lapicid;
	int cpu_model;
	int cpu_familty;
	int cpu_model_name[48];
	const char *cpu_manufacturer;
} cpu_info_t;

extern struct cpu_info cpu_info_table[MAX_CPU_NUM];
extern size_t num_of_cpu;
extern uintptr_t lapic_base_addr;
extern uintptr_t ioapic_base_addr;