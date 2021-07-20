#include <stddef.h>
#include <stdint.h>
#include "layout.h"
#include "tss.h"

struct segment_desc
{
	unsigned lim_15_0 : 16;
	unsigned base_15_0 : 16;
	unsigned base_23_16 : 8;
	unsigned type : 4;
	unsigned s : 1;
	unsigned dpl : 2;
	unsigned p : 1;
	unsigned lim_19_16 : 4;
	unsigned avl : 1;
	unsigned l : 1;
	unsigned db : 1;
	unsigned g : 1;
	unsigned base_31_24 : 8;
};

struct segment_descriptor64
{
	unsigned lim_15_0 : 16;
	unsigned base_15_0 : 16;
	unsigned base_23_16 : 8;
	unsigned type : 4;
	unsigned s : 1;
	unsigned dpl : 2;
	unsigned p : 1;
	unsigned lim_19_16 : 4;
	unsigned avl : 1;
	unsigned rsv1 : 2;
	unsigned g : 1;
	unsigned base_31_24 : 8;
	uint32_t base_63_32;
	unsigned res1 : 8;
	unsigned clear : 8;
	unsigned res2 : 16;
};

#define SEG64(type, base, lim, dpl)                                   \
	(struct segment_desc)                                             \
	{                                                                 \
		((lim) >> 12) & 0xffff, (base)&0xffff, ((base) >> 16) & 0xff, \
			type, 1, dpl, 1, (unsigned)(lim) >> 28, 0, 1, 0, 1,       \
			(unsigned)(base) >> 24                                    \
	}

static struct segment_desc gdt[SEL_CNT] = {
	[SEL_NULL >> 3] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	[SEL_KCSEG >> 3] = SEG64(0xa, 0x0, 0xffffffff, 0),
	[SEL_KDSEG >> 3] = SEG64(0x2, 0x0, 0xffffffff, 0),
	[SEL_UDSEG >> 3] = SEG64(0x2, 0x0, 0xffffffff, 3),
	[SEL_UCSEG >> 3] = SEG64(0xa, 0x0, 0xffffffff, 3),
	[SEL_TSS >> 3] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	[6] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	[7] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

struct {
	uint16_t len;
	struct segment_desc* addr;
} __attribute__ ((packed)) gdtr;

void gdt_init(void)
{
	/* Initialize GDT. */
	struct segment_descriptor64 *tss_desc =
		(struct segment_descriptor64 *)&gdt[SEL_TSS >> 3];
	struct task_state *tss = tss_get();

	*tss_desc = (struct segment_descriptor64){
		.lim_15_0 = (uint64_t)(sizeof(struct task_state)) & 0xffff,
		.base_15_0 = (uint64_t)(tss)&0xffff,
		.base_23_16 = ((uint64_t)(tss) >> 16) & 0xff,
		.type = 0x9,
		.s = 0,
		.dpl = 0,
		.p = 1,
		.lim_19_16 = ((uint64_t)(sizeof(struct task_state)) >> 16) & 0xf,
		.avl = 0,
		.rsv1 = 0,
		.g = 0,
		.base_31_24 = ((uint64_t)(tss) >> 24) & 0xff,
		.base_63_32 = ((uint64_t)(tss) >> 32) & 0xffffffff,
		.res1 = 0,
		.clear = 0,
		.res2 = 0};

	gdtr.addr = gdt;
	gdtr.len = sizeof(gdt) - 1;

	lgdt(&gdtr);
	/* reload segment registers */
	asm volatile("movw %%ax, %%gs" ::"a"(SEL_UDSEG));
	asm volatile("movw %%ax, %%fs" ::"a"(0));
	asm volatile("movw %%ax, %%es" ::"a"(SEL_KDSEG));
	asm volatile("movw %%ax, %%ds" ::"a"(SEL_KDSEG));
	asm volatile("movw %%ax, %%ss" ::"a"(SEL_KDSEG));
	asm volatile("pushq %%rbx\n"
				 "movabs $1f, %%rax\n"
				 "pushq %%rax\n"
				 "lretq\n"
				 "1:\n" ::"b"(SEL_KCSEG)
				 : "cc", "memory");
	/* Kill the local descriptor table */
	ltr(SEL_TSS);
	lldt(0);
}