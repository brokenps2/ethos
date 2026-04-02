#include <stddef.h>
#include "vga.h"
#pragma once

void term_create(size_t width, size_t height, uint16_t* address, size_t row, size_t column, VGAColor fg, VGAColor bg);
size_t strlen(const char* str);
void term_set_color(VGAColor fg, VGAColor bg);
void term_put_entry_at(char c, uint8_t color, size_t x, size_t y);
void term_put_char(char c);
void term_put_char_before(char c);
void term_write(const char* data, size_t size);
void term_write_string(const char* data);
void kprint(const char *str);
int kputs(const char *str);
