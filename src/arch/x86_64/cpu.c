#include <stdint.h>
#include <stddef.h>
#include "debug.h"
#include "vga_text.h"
#include "multiboot2.h"
#include "memory.h"
#include "cpu.h"
#include "intrinsic.h"
#include "msr_flags.h"
#include "thread.h"
#include "register_flags.h"
#include "apic.h"
#include "vmm.h"

#define cpuid(in,a,b,c,d) do { asm volatile ("cpuid" : "=a"(a),"=b"(b),"=c"(c),"=d"(d) : "a"(in)); } while(0)

extern uintptr_t ap_trampoline;
extern uintptr_t ap_trampoline_end;

struct cpu_info cpu_info_table[MAX_CPU_NUM];
size_t num_of_cpu;

static uint64_t ap_end_init = 0;

static volatile int current_ap_index = 0;

static inline void load_cpu_info() {
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

	memcpy(cpu_info_table[current_cpu()->cpuid].cpu_model_name, "Undefined", 10);
	cpuid(0x80000000, a, unused, unused, unused);
	if(a >= 0x80000000) {
		uint32_t brand[12];
		cpuid(0x80000002, brand[0], brand[1], brand[2], brand[3]);
		cpuid(0x80000003, brand[4], brand[5], brand[6], brand[7]);
		cpuid(0x80000004, brand[8], brand[9], brand[10], brand[11]);
		memcpy(cpu_info_table[current_cpu()->cpuid].cpu_model_name, brand, sizeof(uint32_t) * 12);	
	}
}

static inline void set_gs_segment(uintptr_t base) {
	write_msr(MSR_FS_BASE, 0);
	write_msr(MSR_GS_BASE, base);
	write_msr(MSR_KERNEL_GS_BASE, base);
	asm volatile ("swapgs");
}

static inline void enable_sse() {
	uint64_t prev_cr4 = rcr4();
	prev_cr4 |= CR4_OXFXSR | CR4_OXSAVE | CR4_OSXMMEXCPT;
	lcr4(prev_cr4);

	uint64_t prev_xcr0 = rcr4();
	prev_xcr0 = XCR0_X87 | XCR0_SSE | XCR0_AVX;
	lxcr0(prev_xcr0);
}

void debug_cpu() {
	printf("[CPU %d] %s, %s\n", current_cpu()->cpuid, current_cpu()->cpu_manufacturer, current_cpu()->cpu_model_name);
}

void ap_main() {
	// set gs segment's cpu id to current_ap_index
	set_gs_segment((uintptr_t)&cpu_info_table[current_ap_index]);
	current_cpu()->cpuid = current_ap_index;
	load_cpu_info();
	enable_sse();

	vmm_activate(kernel_P4);
	ap_end_init = 1;
	while(1);
}

void bsp_main() {

}

void cpu_init() {
	set_gs_segment((uintptr_t)&cpu_info_table[current_ap_index]);
	current_cpu()->cpuid = current_ap_index;
	load_cpu_info();
	debug_cpu();
	enable_sse();

	for(int i=0;i<num_of_cpu;i++) {
		memcpy((void*)P2V(0x8000), &ap_trampoline, (size_t)&ap_trampoline_end - (size_t)&ap_trampoline);
		if(i == 0) continue;
		ap_end_init = 0;
		ASSERT(i == cpu_info_table[i].cpuid);
		current_ap_index = i;
		// SEND INIT
		lapic_send_ipi(cpu_info_table[i].lapicid, 0x4500);
		// SEND SIPI
		lapic_send_ipi(cpu_info_table[i].lapicid, 0x4600 | (0x8000 >> 12));
		uint64_t prev_clk = read_tsc();
		// This line must be changed!!!! Now hard coded about 0.1 sec in my machine (200ns is required typically....)
		while(read_tsc() - prev_clk < 1000315000 * 0.1) {};
		lapic_send_ipi(cpu_info_table[i].lapicid, 0x4600 | (0x8000 >> 12));
		// WAIT
		do { asm volatile ("pause" : : : "memory"); } while (!ap_end_init);
	}
}

void cpu_tlb_shootdown() {
	
}

uint64_t cpu_read_tsc() {
}