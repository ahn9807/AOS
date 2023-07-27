#include "cmos.h"
#include "lib/debug.h"
#include "lib/time.h"
#include "port.h"
#include "vga_text.h"

// Have to parse from ACPI
int century_register = 0x00;

static struct rtc_tm boot_real_time;

static inline uint8_t read_from_cmos(uint16_t reg)
{
	outb(CMOS_ADDR, reg);
	return inb(CMOS_DATA);
}

static inline uint8_t write_to_cmos(uint16_t reg, uint8_t data)
{
	outb(CMOS_ADDR, reg);
	outb(CMOS_DATA, data);
}

static inline uint8_t in_progress()
{
	outb(CMOS_ADDR, 0x0a);
	return (inb(CMOS_DATA) & 0x80);
}

static inline void wait_progress()
{
	while (in_progress())
		;
}

static void read_rtc_time(rtc_tm_t *rtc_)
{
	wait_progress();

	uint8_t registerB;

	rtc_->second = read_from_cmos(0x00);
	rtc_->minute = read_from_cmos(0x02);
	rtc_->hour = read_from_cmos(0x04);
	rtc_->day = read_from_cmos(0x07);
	rtc_->month = read_from_cmos(0x08);
	rtc_->year = read_from_cmos(0x09);
	if (century_register) {
		rtc_->century = read_from_cmos(century_register);
	}

	// check cmos data type bit 1 is BCD and bit 2 is binary
	// In BCD format, 0x59 = 89 is 59(s/m/h)
	registerB = read_from_cmos(0x0B);

	if (!(registerB & 0x04)) {
		rtc_->second = (rtc_->second & 0x0f) + ((rtc_->second / 16) * 10);
		rtc_->minute = (rtc_->minute & 0x0f) + ((rtc_->minute / 16) * 10);
		rtc_->hour = (rtc_->hour & 0x0f) + ((rtc_->hour / 16) * 10) | (rtc_->hour & 0x80);
		rtc_->day = (rtc_->day & 0x0f) + ((rtc_->day / 16) * 10);
		rtc_->month = (rtc_->month & 0x0f) + ((rtc_->month / 16) * 10);
		rtc_->year = (rtc_->year & 0x0f) + ((rtc_->year / 16) * 10);
		rtc_->century = (rtc_->century & 0x0f) + ((rtc_->century / 16) * 10);
	}

	if (!century_register) {
		rtc_->century = CMOS_DEFAULT_CENTURY;
	}

	if (!(registerB & 0x02) && (rtc_->hour & 0x80)) {
		rtc_->hour = ((rtc_->hour & 0x7f) + 12) % 24;
	}
}

static void write_rtc_time(rtc_tm_t *rtc_)
{
	wait_progress();

	write_to_cmos(0x00, rtc_->second);
	write_to_cmos(0x02, rtc_->minute);
	write_to_cmos(0x04, rtc_->hour);
	write_to_cmos(0x07, rtc_->day);
	write_to_cmos(0x08, rtc_->month);
	write_to_cmos(0x09, rtc_->year);
	if (century_register) {
		write_to_cmos(century_register, rtc_->century);
	}
}

void cmos_init()
{
	read_rtc_time(&boot_real_time);
}

void cmos_get_real_time(struct tm *real_time)
{
	// Later have to add passed time since booted.
	boot_real_time.second += 0;
	boot_real_time.minute += 0;
	boot_real_time.hour += 0;
	/// ...

	read_rtc_time(&boot_real_time);

	real_time->tm_sec = boot_real_time.second;
	real_time->tm_min = boot_real_time.minute;
	real_time->tm_hour = boot_real_time.hour;
	real_time->tm_mday = boot_real_time.day;
	real_time->tm_mon = boot_real_time.month - 1;
	real_time->tm_year = boot_real_time.year + boot_real_time.century * 100 - 1900;
}

void cmos_set_real_time(struct tm *real_time)
{
	PANIC("NOT IMPLEMENTED");
}