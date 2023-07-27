#pragma once

#include "gdt.h"
#include "lib/types.h"

__attribute__((always_inline)) static __inline void lcr0(uint64_t val)
{
	__asm __volatile("movq %0, %%cr0" : : "r"(val));
}

/* Store the physical address of the page directory into CR3
   aka PDBR (page directory base register).  This activates our
   new page tables immediately.  See [IA32-v2a] "MOV--Move
   to/from Control Registers" and [IA32-v3a] 3.7.5 "Base Address
   of the Page Directory". */
__attribute__((always_inline)) static __inline void lcr3(uint64_t val)
{
	__asm __volatile("movq %0, %%cr3" : : "r"(val));
}

__attribute__((always_inline)) static __inline void lcr4(uint64_t val)
{
	__asm __volatile("movq %0, %%cr4" : : "r"(val));
}

__attribute__((always_inline)) static __inline void lgdt(const void *dtr)
{
	__asm __volatile("lgdt %0" : : "m"(*(uintptr_t *)dtr));
}

__attribute__((always_inline)) static __inline void lldt(uint16_t sel)
{
	__asm __volatile("lldt %0" : : "r"(sel));
}

__attribute__((always_inline)) static __inline void ltr(uint16_t sel)
{
	__asm __volatile("ltr %0" : : "r"(sel));
}

__attribute__((always_inline)) static __inline void lidt(const void *idt)
{
	__asm __volatile("lidt %0" : : "m"(*(uintptr_t *)idt));
}

__attribute__((always_inline)) static __inline void invlpg(uint64_t addr)
{
	__asm __volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

__attribute__((always_inline)) static __inline uint64_t read_eflags(void)
{
	uint64_t rflags;
	__asm __volatile("pushfq; popq %0" : "=r"(rflags));
	return rflags;
}

__attribute__((always_inline)) static __inline uint64_t rcr0(void)
{
	uint64_t val;
	__asm __volatile("movq %%cr3,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rcr2(void)
{
	uint64_t val;
	__asm __volatile("movq %%cr2,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rcr3(void)
{
	uint64_t val;
	__asm __volatile("movq %%cr3,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rcr4(void)
{
	uint64_t val;
	__asm __volatile("movq %%cr4,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rrax(void)
{
	uint64_t val;
	__asm __volatile("movq %%rax,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rrdi(void)
{
	uint64_t val;
	__asm __volatile("movq %%rdi,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rrsi(void)
{
	uint64_t val;
	__asm __volatile("movq %%rsi,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rrdx(void)
{
	uint64_t val;
	__asm __volatile("movq %%rdx,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rr10(void)
{
	uint64_t val;
	__asm __volatile("movq %%r10,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rr8(void)
{
	uint64_t val;
	__asm __volatile("movq %%r8,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rr9(void)
{
	uint64_t val;
	__asm __volatile("movq %%r9,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rrcx(void)
{
	uint64_t val;
	__asm __volatile("movq %%rcx,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline uint64_t rrsp(void)
{
	uint64_t val;
	__asm __volatile("movq %%rsp,%0" : "=r"(val));
	return val;
}

__attribute__((always_inline)) static __inline void write_msr(uint32_t ecx, uint64_t val)
{
	uint32_t edx, eax;
	eax = (uint32_t)val;
	edx = (uint32_t)(val >> 32);
	__asm __volatile("wrmsr" ::"c"(ecx), "d"(edx), "a"(eax));
}

__attribute__((always_inline)) static __inline uint64_t read_tsc()
{
	uint32_t lo, hi;
	asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | (uint64_t)lo;
}

__attribute__((always_inline)) static __inline uint64_t rxcr0()
{
	uint32_t lo, hi;
	asm volatile("xgetbv" : "=a"(lo), "=d"(hi) : "c"(0));
	return ((uint64_t)hi << 32) | (uint64_t)lo;
}

__attribute__((always_inline)) static __inline void lxcr0(uint64_t val)
{
	uint32_t edx, eax;
	eax = (uint32_t)val;
	edx = (uint32_t)(val >> 32);
	asm volatile("xsetbv" ::"c"(0), "d"(edx), "a"(eax));
}