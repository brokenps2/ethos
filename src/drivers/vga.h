#pragma once
#include <stddef.h>
#include <stdint.h>

#define PSF1_FONT_MAGIC 0x0436

#define PSF_FONT_MAGIC 0x864ab572

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t headerSize;
	uint32_t flags;
	uint32_t glyphCount;
	uint32_t bytesPerGlyph;
	uint32_t height;
	uint32_t width;
} PSFFont;

typedef enum VGAColor {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GRAY = 7,
	VGA_COLOR_DARK_GRAY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
} VGAColor;


uint8_t vga_entry_color(VGAColor fg, VGAColor bg);
uint16_t vga_entry(unsigned char uc, uint8_t color);
uint32_t fb_get_address();
void fb_put_pixel(int x, int y, uint32_t color);
void fb_draw_rect(int x, int y, int w, int h, uint32_t color);
void fb_clear(uint32_t color);
