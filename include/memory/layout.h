#pragma once

/* GDT selectors defined by loader.
   More selectors are defined by userprog/gdt.h. */
#define SEL_NULL        0x00    /* Null selector. */
#define SEL_KCSEG       0x08    /* Kernel code selector. */
#define SEL_KDSEG       0x10    /* Kernel data selector. */
#define SEL_UDSEG       0x1B    /* User data selector. */
#define SEL_UCSEG       0x23    /* User code selector. */
#define SEL_TSS         0x28    /* Task-state segment. */
#define SEL_CNT         8       /* Number of segments. */
#define USER_STACK 0x47480000