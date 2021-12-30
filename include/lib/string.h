#pragma once

#include <stddef.h>
#include <stdint.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memchr(const void *buf, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *str);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

char *strcpy_s(char *dst, const char *src, size_t dstSize);

char * strtok_r(char * str, const char * delim, char ** saveptr);
char * strtok(char * str, const char * delim);
char * strcat(char *dest, const char *src);
char * strncat(char *dest, const char *src, size_t n);
char *strdup(const char *s);

void itoa(char *buf, int64_t base, int64_t d);