#include <stdint.h>

#pragma once

void psf_init();

void psf_putchar(int x, int y, uint32_t codepoint, uint32_t fg, uint32_t bg);

void psf_puts(int x, int y, const char *str, uint32_t fg, uint32_t bg);
