#pragma once

#include <stdint.h>
#define MAX_CPU_NUM 128

typedef struct cpu_info
{
	int cpuid;
	int lapicid;
	int cpu_model;
	int family;
	uint32_t cpu_model_name[12];
	const char *cpu_manufacturer;
} cpu_info_t;

extern struct cpu_info cpu_info_table[MAX_CPU_NUM];
extern size_t num_of_cpu;
extern uintptr_t lapic_base_addr;
extern uintptr_t ioapic_base_addr;

static struct cpu_info __seg_gs *current_cpu_ = 0;
#define current_cpu() (&cpu_info_table[current_cpu_->cpuid])
#define current_cpuid() (current_cpu_->cpuid)

void cpu_init();
void cpu_tlb_shootdown();
uint64_t cpu_read_tsc();
void debug_cpu();