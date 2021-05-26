#pragma once

/*
█▀▀ █▀█ █▄░█ ▀█▀ █▀█ █▀█ █░░ █▀▀ █▀█   █▀█ █▀▀ █▀▀ █ █▀ ▀█▀ █▀▀ █▀█   ▄█
█▄▄ █▄█ █░▀█ ░█░ █▀▄ █▄█ █▄▄ ██▄ █▀▄   █▀▄ ██▄ █▄█ █ ▄█ ░█░ ██▄ █▀▄   ░█
*/

// 0 	PE 	Protected Mode Enable 	If 1, system is in protected mode, else system is in real mode
#define CR0_PE (1<<0)
// 1 	MP 	Monitor co-processor 	Controls interaction of WAIT/FWAIT instructions with TS flag in CR0
#define CR0_MP (1<<1)
// 2 	EM 	Emulation 	If set, no x87 floating-point unit present, if clear, x87 FPU present
#define CR0_EM (1<<2)
// 3 	TS 	Task switched 	Allows saving x87 task context upon a task switch only after x87 instruction used
#define CR0_TS (1<<3)
// 4 	ET 	Extension type 	On the 386, it allowed to specify whether the external math coprocessor was an 80287 or 80387
#define CR0_ET (1<<4)
// 5 	NE 	Numeric error 	Enable internal x87 floating point error reporting when set, else enables PC style x87 error detection
#define CR0_NE (1<<5)
// 16 	WP 	Write protect 	When set, the CPU can't write to read-only pages when privilege level is 0
#define CR0_WP (1<<16)
// 18 	AM 	Alignment mask 	Alignment check enabled if AM set, AC flag (in EFLAGS register) set, and privilege level is 3
#define CR0_AM (1<<18)
// 29 	NW 	Not-write through 	Globally enables/disable write-through caching
#define CR0_NW (1<<29)
// 30 	CD 	Cache disable 	Globally enables/disable the memory cache
#define CR0_CD (1<<30)
// 31 	PG 	Paging 	If 1, enable paging and use the § CR3 register, else disable paging.
#define CR0_PG (1<<31)

/*
█▀▀ █▀█ █▄░█ ▀█▀ █▀█ █▀█ █░░ █▀▀ █▀█   █▀█ █▀▀ █▀▀ █ █▀ ▀█▀ █▀▀ █▀█   █░█
█▄▄ █▄█ █░▀█ ░█░ █▀▄ █▄█ █▄▄ ██▄ █▀▄   █▀▄ ██▄ █▄█ █ ▄█ ░█░ ██▄ █▀▄   ▀▀█
*/

// 0 	VME 	Virtual 8086 Mode Extensions 	If set, enables support for the virtual interrupt flag (VIF) in virtual-8086 mode.
#define CR4_VME (1<<0)
// 1 	PVI 	Protected-mode Virtual Interrupts 	If set, enables support for the virtual interrupt flag (VIF) in protected mode.
#define CR4_PVI (1<<1)
// 2 	TSD 	Time Stamp Disable 	If set, RDTSC instruction can only be executed when in ring 0, otherwise RDTSC can be used at any privilege level.
#define CR4_TSD (1<<2)
// 3 	DE 	Debugging Extensions 	If set, enables debug register based breaks on I/O space access.
#define CR4_DE (1<<3)
// 4 	PSE 	Page Size Extension 	If unset, page size is 4 KiB, else page size is increased to 4 MiB, If PAE is enabled or the processor is in x86-64 long mode this bit is ignored.
#define CR4_PSE (1<<4)
// 5 	PAE 	Physical Address Extension 	If set, changes page table layout to translate 32-bit virtual addresses into extended 36-bit physical addresses.
#define CR4_PAE (1<<5)
// 6 	MCE 	Machine Check Exception 	If set, enables machine check interrupts to occur.
#define CR4_MCE (1<<6)
// 7 	PGE 	Page Global Enabled 	If set, address translations (PDE or PTE records) may be shared between address spaces.
#define CR4_PGE (1<<7)
// 8 	PCE 	Performance-Monitoring Counter enable 	If set, RDPMC can be executed at any privilege level, else RDPMC can only be used in ring 0.
#define CR4_PCE (1<<8)
// 9 	OSFXSR 	Operating system support for FXSAVE and FXRSTOR instructions 	If set, enables Streaming SIMD Extensions (SSE) instructions and fast FPU save & restore.
#define CR4_OXFXSR (1<<9)
// 10 	OSXMMEXCPT 	Operating System Support for Unmasked SIMD Floating-Point Exceptions 	If set, enables unmasked SSE exceptions.
#define CR4_OSXMMEXCPT (1<<10)
// 11 	UMIP 	User-Mode Instruction Prevention 	If set, the SGDT, SIDT, SLDT, SMSW and STR instructions cannot be executed if CPL > 0.[1]
#define CR4_UMIP (1<<11)
// 12 	LA57 	(none specified) 	If set, enables 5-Level Paging.[3]
#define CR4_LA57 (1<<12)
// 13 	VMXE 	Virtual Machine Extensions Enable 	see Intel VT-x x86 virtualization.
#define CR4_VMXE (1<<13)
// 14 	SMXE 	Safer Mode Extensions Enable 	see Trusted Execution Technology (TXT)
#define CR4_SMXE (1<<14)
// 16 	FSGSBASE 	Enables the instructions RDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE.
#define CR4_FSGBASE (1<<16)
// 17 	PCIDE 	PCID Enable 	If set, enables process-context identifiers (PCIDs).
#define CR4_PCIDE (1<<17)
// 18 	OSXSAVE 	XSAVE and Processor Extended States Enable
#define CR4_OXSAVE (1<<18)
// 20 	SMEP 	Supervisor Mode Execution Protection Enable 	If set, execution of code in a higher ring generates a fault.
#define CR4_SMEP (1<<20)
// 21 	SMAP 	Supervisor Mode Access Prevention Enable 	If set, access of data in a higher ring generates a fault.[5]
#define CR4_SMAP (1<<21)
// 22 	PKE 	Protection Key Enable 	See Intel 64 and IA-32 Architectures Software Developer’s Manual.
#define CR4_PKE (1<<22)

/*
█▀▀ ▀▄▀ ▀█▀ █▀▀ █▄░█ █▀▄ █▀▀ █▀▄   █▀▀ █▀█ █▄░█ ▀█▀ █▀█ █▀█ █░░   █▀█ █▀▀ █▀▀ █ █▀ ▀█▀ █▀▀ █▀█   █▀█
██▄ █░█ ░█░ ██▄ █░▀█ █▄▀ ██▄ █▄▀   █▄▄ █▄█ █░▀█ ░█░ █▀▄ █▄█ █▄▄   █▀▄ ██▄ █▄█ █ ▄█ ░█░ ██▄ █▀▄   █▄█
*/

// 0 	X87 (x87 FPU/MMX State, note, must be '1')
#define XCR0_X87 (1<<0)
// 1 	SSE (XSAVE feature set enable for MXCSR and XMM regs)
#define XCR0_SSE (1<<1)
// 2 	AVX (AVX enable, and XSAVE feature set can be used to manage YMM regs)
#define XCR0_AVX (1<<2)
// 3 	BNDREG (MPX enable, and XSAVE feature set can be used for BND regs)
#define XCR0_BNDREG (1<<3)
// 4 	BNDCSR (MPX enable, and XSAVE feature set can be used for BNDCFGU and BNDSTATUS regs)
#define XCR0_BNDCSR (1<<4)
// 5 	opmask (AVX-512 enable, and XSAVE feature set can be used for AVX opmask, AKA k-mask, regs)
#define XCR0_OPMASK (1<<5)
// 6 	ZMM_hi256 (AVX-512 enable, and XSAVE feature set can be used for upper-halves of the lower ZMM regs)
#define XCR0_ZMM_HI256 (1<<6)
// 7 	Hi16_ZMM (AVX-512 enable, and XSAVE feature set can be used for the upper ZMM regs)
#define XCR0_ZMM_HI16 (1<<7)
// 9 	PKRU (XSAVE feature set can be used for PKRU register, which is part of the protection keys mechanism.)
#define XCR0_PKRU (1<<9)
// 11 	Control-flow Enforcement Technology (CET) User State
#define XCR0_USER_STATE (1<<11)
// 12 	Control-flow Enforcement Technology (CET) Supervisor State
#define XCR0_SUPERVISOR_STATE (1<<12)