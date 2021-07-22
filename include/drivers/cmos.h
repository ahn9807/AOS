#pragma once

// Register  Contents            Range
//  0x00      Seconds             0–59
//  0x02      Minutes             0–59
//  0x04      Hours               0–23 in 24-hour mode, 
//                                1–12 in 12-hour mode, highest bit set if pm
//  0x06      Weekday             1–7, Sunday = 1
//  0x07      Day of Month        1–31
//  0x08      Month               1–12
//  0x09      Year                0–99
//  0x32      Century (maybe)     19–20?
//  0x0A      Status Register A
//  0x0B      Status Register B

#include "time.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

#define CMOS_DEFAULT_CENTURY 20

void cmos_init();
void cmos_get_real_time(struct tm *real_time);