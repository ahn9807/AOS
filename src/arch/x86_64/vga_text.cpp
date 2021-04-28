#include "vga_text.h"

size_t terminal_row = 0;
size_t terminal_column = 0;
uint8_t terminal_color = 0;
uint16_t *terminal_buffer = 0;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t)uc | (uint16_t)color << 8;
}

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t *)VIDEO_ADDR;

	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void putchar(char c)
{
	if(terminal_row == VGA_HEIGHT) {
		cls();
	}
	if (c == '\n' || c == '\r')
	{
		terminal_column = 0;
		terminal_row++;
		if (terminal_row >= VGA_HEIGHT)
			terminal_column = 0;
		return;
	}
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH)
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void write(const char *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		putchar(data[i]);
}

void writestring(const char *data)
{
	write(data, strlen(data));
}

/*  Clear the screen and initialize VIDEO, XPOS and YPOS. */
void cls(void)
{
	int i;

	terminal_buffer = (uint16_t *)VIDEO_ADDR;

	for (i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
		*(terminal_buffer + i) = (uint16_t)0;

	terminal_row = 0;
	terminal_column = 0;
}

/*  Convert the integer D to a string and save the string in BUF. If
   BASE is equal to ’d’, interpret that D is decimal, and if BASE is
   equal to ’x’, interpret that D is hexadecimal. */
static void itoa(char *buf, int base, int d)
{
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;

	/*  If %d is specified and D is minus, put ‘-’ in the head. */
	if (base == 'd' && d < 0)
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if (base == 'x')
		divisor = 16;

	/*  Divide UD by DIVISOR until UD == 0. */
	do
	{
		int remainder = ud % divisor;

		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	} while (ud /= divisor);

	/*  Terminate BUF. */
	*p = 0;

	/*  Reverse BUF. */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}

/*  Format a string and print it on the screen, just like the libc
   function printf. */
void printf(const char *format, ...)
{
	va_list argument_list;
	va_start(argument_list, format);
	int c;
	char buf[20];
	char *arg;
	int int_arg;

	while ((c = *format++) != 0)
	{
		if (c != '%')
			putchar(c);
		else
		{
			char *p, *p2;
			int pad0 = 0, pad = 0;

			c = *format++;
			if (c == '0')
			{
				pad0 = 1;
				c = *format++;
			}

			if (c >= '0' && c <= '9')
			{
				pad = c - '0';
				c = *format++;
			}

			switch (c)
			{
				case 'd':
				case 'u':
				case 'x':
					int_arg = va_arg(argument_list, int);
					itoa(buf, c, int_arg);
					p = buf;
					goto string;
					break;

				case 's':
					p = va_arg(argument_list, char*);
					if (!p)
						p = (char *)("null");

				string:
					for (p2 = p; *p2; p2++)
						;
					for (; p2 < p + pad; p2++)
						putchar(pad0 ? '0' : ' ');
					while (*p)
						putchar(*p++);
					break;

				default:
					arg = va_arg(argument_list, char*);
					putchar(*((int *)arg++));
					break;
			}
		}
	}
}

void panic(const char *panic_message)
{
    printf("[Kernel Panic!]%s\n", panic_message);
    while (1)
    {
    };
}