#pragma once

// 0 	SCE (System Call Extensions)
#define MSR_SCE (1<<0)
// 1 	DPE (AMD K6 only: Data Prefetch Enable)
#define MSR_DPE (1<<1)
// 2 	SEWBED (AMD K6 only: Speculative EWBE# Disable)
#define MSR_SEWBED (1<<2)
// 3 	GEWBED (AMD K6 only: Global EWBE# Disable)
#define MSR_GEWBED (1<<3)
// 4 	L2D (AMD K6 only: L2 Cache Disable)
#define MSR_L2D (1<<4)
// 8 	LME (Long Mode Enable)
#define MSR_LME (1<<8)
// 10 	LMA (Long Mode Active)
#define MSR_LMA (1<<10)
// 11 	NXE (No-Execute Enable)
#define MSR_NXE (1<<11)
// 12 	SVME (Secure Virtual Machine Enable)
#define MSR_SVME (1<<12)
// 13 	LMSLE (Long Mode Segment Limit Enable)
#define MSR_LMSLE (1<<13)
// 14 	FFXSR (Fast FXSAVE/FXRSTOR)
#define MSR_FFXSR (1<<14)
// 15 	TCE (Translation Cache Extension)
#define MSR_TCE (1<<15)

/// Ring 0 and Ring 3 Segment bases, as well as SYSCALL EIP.
/// Low 32 bits = SYSCALL EIP, bits 32-47 are kernel segment base, bits 48-63
/// are user segment base.
#define MSR_STAR = 0xc0000081
/// The kernel's RIP SYSCALL entry for 64 bit software.
#define MSR_LSTAR = 0xc0000082
/// The kernel's RIP for SYSCALL in compatibility mode.
#define MSR_CSTAR = 0xc0000083
/// The low 32 bits are the SYSCALL flag mask.
/// If a bit in this is set, the corresponding bit in rFLAGS is cleared.
#define MSR_SF_MASK = 0xc0000084
#define MSR_FS_BASE = 0xc0000100
#define MSR_GS_BASE = 0xc0000101
#define MSR_KERNEL_GS_BASE = 0xc0000102
#define MSR_TSC_DEADLINE = 0x6e0