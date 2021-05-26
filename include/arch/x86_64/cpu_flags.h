#pragma once

// Carry flag (1 Carry, 0 No Carry)
#define FLAG_CF (1 << 0)
// Parity flag
#define FLAG_PF (1 << 2)
// Auxiliary Carry Flag
#define FLAG_AF (1 << 4)
// Zero Flag
#define FLAG_ZF (1 << 6)
// Sign Flag
#define FLAG_SF (1 << 7)
// Trap Flag
#define FLAG_TF (1 << 8)
// Interrupt Enable Flag
#define FLAG_IF (1 << 9)
// Direction Flag
#define FLAG_DF (1 << 10)
// Overflow Flag
#define FLAG_OF (1 << 11)
// I/O Privilege Level 1
#define FLAG_IOPL1 (1 << 12)
// I/O Privilege Level 2
#define FLAG_IOPL2 (1 << 13)
// Nested Task
#define FLAG_NT (1 << 14)
// Resume Flag
#define FLAG_RF (1 << 16)
// Virtual-8086 Mode
#define FLAG_VM (1 << 17)
// Alignment Check
#define FLAG_AC (1 << 18)
// Virtual Interrupt Flag
#define FLAG_VIF (1 << 19)
// Virtual Interrupt Pending
#define FLAG_VIP (1 << 20)
// ID Flag
#define FLAG_ID (1 << 21)