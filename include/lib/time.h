#pragma once

#include <stdint.h>

/* Identifier for system-wide realtime clock.  */
# define CLOCK_REALTIME			0
/* Monotonic system-wide clock.  */
# define CLOCK_MONOTONIC		1
/* High-resolution timer from the CPU.  */
# define CLOCK_PROCESS_CPUTIME_ID	2
/* Thread-specific CPU-time clock.  */
# define CLOCK_THREAD_CPUTIME_ID	3
/* Monotonic system-wide clock, not adjusted for frequency scaling.  */
# define CLOCK_MONOTONIC_RAW		4
/* Identifier for system-wide realtime clock, updated only on ticks.  */
# define CLOCK_REALTIME_COARSE		5
/* Monotonic system-wide clock, updated only on ticks.  */
# define CLOCK_MONOTONIC_COARSE		6
/* Monotonic system-wide clock that includes time spent in suspension.  */
# define CLOCK_BOOTTIME			7
/* Like CLOCK_REALTIME but also wakes suspended system.  */
# define CLOCK_REALTIME_ALARM		8
/* Like CLOCK_BOOTTIME but also wakes suspended system.  */
# define CLOCK_BOOTTIME_ALARM		9
/* Like CLOCK_REALTIME but in International Atomic Time.  */
# define CLOCK_TAI			11

/* Flag to indicate time is absolute.  */
# define TIMER_ABSTIME			1

typedef struct rtc_tm {
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t century;
} rtc_tm_t;

struct tm
{
    int  tm_sec;    /* seconds after the minute [0-60] */
    int  tm_min;    /* minutes after the hour [0-59] */
    int  tm_hour;   /* hours since midnight [0-23] */
    int  tm_mday;   /* day of the month [1-31] */
    int  tm_mon;    /* months since January [0-11] */
    int  tm_year;   /* years since 1900 */
    int  tm_wday;   /* days since Sunday [0-6] */
    int  tm_yday;   /* days since January 1 [0-365] */
    int  tm_isdst;  /* Daylight Savings Time flag */
    long tm_gmtoff; /* offset from UTC in seconds */
    char *tm_zone;  /* timezone abbreviation */
};

typedef long time_t;
typedef long clock_t;

typedef struct timespec
{
    time_t tv_sec;     /* seconds */
    long   tv_nsec;    /* nanosecond */
} timespec_t;