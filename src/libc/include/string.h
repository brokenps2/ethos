#include <stddef.h>

#pragma once

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char*);
char* strtok(char* str, const char* delim);
char* strcpy(char* s1, const char* s2);
