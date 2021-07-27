#include <stdint.h>
#include <stddef.h>
#include "debug.h"
#include "vga_text.h"
#include "multiboot2.h"
#include "memory.h"
#include "cpu.h"

#define cpuid(in,a,b,c,d) do { asm volatile ("cpuid" : "=a"(a),"=b"(b),"=c"(c),"=d"(d) : "a"(in)); } while(0)

extern uintptr_t ap_trampoline;
extern uintptr_t ap_trampoline_end;

struct cpu_info cpu_info_table[MAX_CPU_NUM];
size_t num_of_cpu;

static uint64_t ap_end_init = 0;

static volatile int current_ap_index = 0;

static inline load_cpu_info() {
	uint64_t a, b, unused;
	cpuid(0, unused, b, unused, unused);

	current_cpu()->cpu_manufacturer = "Unknown";

	if(b == 0x756e6547) {
		cpuid(1, a, unused, unused, unused);
		current_cpu()->cpu_manufacturer = "Intel";
		current_cpu()->cpu_model = (a >> 4) & 0x0f;
		current_cpu()->family = (a >> 8) & 0x0f;
	} else {
		cpuid(1, a, b, unused, unused);
		current_cpu()->cpu_manufacturer = "AMD";
		current_cpu()->cpu_model = (a >> 4) & 0x0f;
		current_cpu()->family = (a >> 8) & 0x0f;
	}

	cpuid(0x80000000, a, unused, unused, unused);
	*current_cpu()->cpu_model_name = "Unknown";
	if(a >= 0x80000000) {
		uint32_t brand[12];
		cpuid(0x80000002, brand[0], brand[1], brand[2], brand[3]);
		cpuid(0x80000003, brand[4], brand[5], brand[6], brand[7]);
		cpuid(0x80000004, brand[8], brand[9], brand[10], brand[11]);
		memcpy(current_cpu()->cpu_model_name, brand, sizeof(uint32_t) * 12);	
	}
}

void debug_cpu() {
	printf("[CPU %d] %s, %s\n", current_cpu()->cpuid, current_cpu()->cpu_manufacturer, current_cpu()->cpu_model_name);
}

void ap_main(int lapic_id) {
	// set gs segment's cpu id to current_ap_index
	
	current_cpu()->cpuid = current_ap_index;
	load_cpu_info();
	debug_cpu();
	vmm_activate(V2P(kernel_P4));
	ap_end_init = 1;
	while(1);
}

void bsp_main() {

}

void cpu_init() {
	current_cpu()->cpuid = 0;
	load_cpu_info();
	debug_cpu();

	memcpy((void*)P2V(0x8000), &ap_trampoline, (size_t)&ap_trampoline_end - (size_t)&ap_trampoline);
	printf("length: 0x%x\n", (size_t)&ap_trampoline_end - (size_t)&ap_trampoline);
	printf("start addr: 0x%x\n", &ap_trampoline);
	printf("end addr: 0x%x\n", &ap_trampoline_end);
	printf("test: 0x%x\n", 8000 >> 12);
	for(int i=0;i<num_of_cpu;i++) {
		if(i == 0) continue;
		ap_end_init = 0;
		ASSERT(i == cpu_info_table[i].cpuid);
		current_ap_index = i;
		// SEND INIT
		lapic_send_ipi(cpu_info_table[i].lapicid, 0x4500);
		// SEND SIPI
		lapic_send_ipi(cpu_info_table[i].lapicid, 0x4600 | (0x8000 >> 12));
		lapic_send_ipi(cpu_info_table[i].lapicid, 0x4600 | (0x8000 >> 12));
		// WAIT
		for(int i=0;i<1000000;i++);
		// do { asm volatile ("pause" : : : "memory"); } while (!ap_end_init);
	}
}

void cpu_tlb_shootdown() {
	
}

uint64_t cpu_read_tsc() {
}