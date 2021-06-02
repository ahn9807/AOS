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
    char *ret = kcalloc((len + 1) * sizeof(char), 1);
    (void) memcpy(ret, s, len + 1);
    return ret;
}