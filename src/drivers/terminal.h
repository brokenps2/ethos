#include <stddef.h>
#include "vga.h"
#pragma once

void term_create(size_t width, size_t height, uint32_t fg, uint32_t bg);
size_t strlen(const char* str);
void term_set_color(uint32_t fg, uint32_t bg);
void term_put_entry_at(char c, uint8_t color, size_t x, size_t y);
void term_put_char(char c);
void term_put_char_before(char c);
void term_write(const char* data, size_t size);
void term_write_string(const char* data);
void handle_command(char* cmd);
void kprint(const char *str);
int kputs(const char *str);
void term_clear();
