#include <stdint.h>
#include "vga.h"
#include "kernel/multiboot.h"
#include <stddef.h>

uint8_t vga_entry_color(VGAColor fg, VGAColor bg) { return fg | bg << 4; }

uint16_t vga_entry(unsigned char uc, uint8_t color) { 
	return (uint16_t)uc | (uint16_t)color << 8;
}


extern multiboot_info_t* mbi;

uint32_t fb_get_address() {
	uint32_t fb = (uint32_t)mbi->framebuffer_addr;

	return fb;
}

void fb_put_pixel(int x, int y, uint32_t color) {
    uint8_t* fb = (uint8_t*)(uintptr_t)mbi->framebuffer_addr;

    uint32_t bpp = mbi->framebuffer_bpp;
    uint32_t bytespp = bpp / 8;

    uint8_t* pixel = fb
        + y * mbi->framebuffer_pitch
        + x * bytespp;

    switch (bpp) {
        case 32:
            *(uint32_t*)pixel = color;
            break;

        case 24:
            pixel[0] = color & 0xFF;
            pixel[1] = (color >> 8) & 0xFF;
            pixel[2] = (color >> 16) & 0xFF;
            break;

        case 16: {
            uint16_t c =
                ((color >> 19) << 11) |
                (((color >> 10) & 0x3F) << 5) |
                ((color >> 3) & 0x1F);

            *(uint16_t*)pixel = c;
            break;
        }
    }
}

void fb_clear(uint32_t color) {
    volatile uint32_t *fb = (volatile uint32_t*)(uintptr_t)mbi->framebuffer_addr;
    uint32_t pixels_per_row = mbi->framebuffer_pitch / (mbi->framebuffer_bpp / 8);
    for (uint32_t y = 0; y < mbi->framebuffer_height; y++) {
	for (uint32_t x = 0; x < mbi->framebuffer_width; x++) {
	    fb[y * pixels_per_row + x] = color;
	}
    }
}

void fb_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
	    fb_put_pixel(x + col, y + row, color);
	}
    }
}
