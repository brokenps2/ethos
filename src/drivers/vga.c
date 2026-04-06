#include <stdint.h>
#include "vga.h"
#include "kernel/multiboot.h"
#include <stddef.h>

uint8_t vga_entry_color(VGAColor fg, VGAColor bg) { return fg | bg << 4; }

uint16_t vga_entry(unsigned char uc, uint8_t color) { 
	return (uint16_t)uc | (uint16_t)color << 8;
}


extern multiboot_info_t* mbi;

uint32_t get_framebuffer_address() {
	uint32_t fb = (uint32_t)mbi->framebuffer_addr;

	return fb;
}

void put_pixel(int x, int y, uint32_t color) {
    uint32_t* fb = (uint32_t*)mbi->framebuffer_addr;
    uint32_t pixels_per_row = mbi->framebuffer_pitch / (mbi->framebuffer_bpp / 8);
    fb[y * pixels_per_row + x] = color;
}
