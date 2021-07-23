#include "string.h"
#include "vga_text.h"
#include "kmalloc.h"
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t n)
{
	char *dp = dst;
	const char *sp = src;
	while (n--)
		*dp++ = *sp++;
	return dst;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *p = s;
	while (n--)
    {
		*p++ = (unsigned char)c;
    }
	return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
	if (src == dst)
		return dst;

	const void *src_end = (const void *)((uintptr_t)src + n);
	if (src < dst && dst < src_end)
	{
		char *dp = dst;
		const char *sp = src;
		while (n--)
			dp[n] = sp[n];
		return dst;
	}

	memcpy(dst, src, n);
	return dst;
}

void *memchr(const void *buf, int c, size_t n) {
    char *dp = (char *)buf;
    char *dp_end = dp + n;

    while(dp != dp_end) {
        if(*dp == c) {
            return dp;
        }

        dp++;
    }

    return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *p1 = s1, *p2 = s2;
	for (; n--; p1++, p2++)
	{
		if (*p1 != *p2)
			return *p1 - *p2;
	}
	return 0;
}

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

char *strcpy(char *dst, const char *src) {
    char c;
    char *p = dst;

    while ((c = *src++))
    {
        *p++ = c;
    }

    *p = '\0';
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    size_t i;

    for (i = 0 ; i < n && src[i] != '\0' ; i++)
    {
        dst[i] = src[i];
    }

    for ( ; i < n ; i++)
    {
        dst[i] = '\0';
    }

    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == '\0')
        {
            return 0;
        }

        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	int cnt = 0;
    while (*s1 == *s2 && cnt < n)
    {
        if (*s1 == '\0')
        {
            return 0;
        }

        ++s1;
        ++s2;
	
		cnt++;
    }

	if(cnt == n) {
		return 0;
	}

    return *s1 - *s2;
}

char *strcpy_s(char *dst, const char *src, size_t dstSize)
{
    char *p = dst;
    char *end = dst + dstSize - 1;
    char c;

    while ((c = *src++) && dst < end)
    {
        *p++ = c;
    }

    if (p < end + 1)
    {
        *p = '\0';
    }

    return dst;
}

char *strdup(const char *s)
{
    int len = strlen(s);
    char *ret = kcalloc(1, (len + 1) * sizeof(char));
    (void) memcpy(ret, s, len + 1);
    return ret;
}

/*  Convert the integer D to a string and save the string in BUF. If
   BASE is equal to ’d’, interpret that D is decimal, and if BASE is
   equal to ’x’, interpret that D is hexadecimal. */
void itoa(char *buf, int64_t base, int64_t d)
{
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;

	/*  If %d is specified and D is minus, put ‘-’ in the head. */
	if ((base == 'd' || base == 'i') && d < 0)
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
		uint64_t remainder = ud % divisor;

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

// char * strtok_r(char * str, const char * delim, char ** saveptr) {
// 	char * token;
// 	if (str == NULL) {
// 		str = *saveptr;
// 	}
// 	str += strspn(str, delim);
// 	if (*str == '\0') {
// 		*saveptr = str;
// 		return NULL;
// 	}
// 	token = str;
// 	str = strpbrk(token, delim);
// 	if (str == NULL) {
// 		*saveptr = (char *)lfind(token, '\0');
// 	} else {
// 		*str = '\0';
// 		*saveptr = str + 1;
// 	}
// 	return token;
// }

// char * strtok(char * str, const char * delim) {
// 	static char * saveptr = NULL;
// 	if (str) {
// 			saveptr = NULL;
// 	}
// 	return strtok_r(str, delim, &saveptr);
// }

char * strcat(char *dest, const char *src) {
	char * end = dest;
	while (*end != '\0') {
		++end;
	}
	while (*src) {
		*end = *src;
		end++;
		src++;
	}
	*end = '\0';
	return dest;
}

char * strncat(char *dest, const char *src, size_t n) {
	char * end = dest;
	while (*end != '\0') {
		++end;
	}
	size_t i = 0;
	while (*src && i < n) {
		*end = *src;
		end++;
		src++;
		i++;
	}
	*end = '\0';
	return dest;
}