#include "drivers/terminal.h"
#include <stdint.h>
#include "kernel/fonts.h"
#include "vga.h"
#include <stdbool.h>
#include "arch/i386/ports.h"
#include "kernel/multiboot.h"
#include "string.h"
#include <stddef.h>

size_t term_width;      // in characters
size_t term_height;     // in characters
size_t term_row;
size_t term_column;
uint32_t term_fg;
uint32_t term_bg;

uint16_t* vga_buffer = (uint16_t*)0xB8000;

bool cursor_visible = false;


void textmode_term_enable_cursor(uint8_t start, uint8_t end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}
void textmode_term_disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}
void textmode_term_set_cursor_pos(int row, int column) {
    uint16_t pos = row * term_width + column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void textmode_term_clear() {
    for(int y = 0; y < term_height; y++) {
        for(int x = 0; x < term_width; x++) {
            int cpos = (y * term_width) + x;
            vga_buffer[cpos] = ' ';
        }
    }
    term_row = 0;
    term_column = 0;
    textmode_term_set_cursor_pos(1, 1);
    textmode_term_enable_cursor(0, 15);
}

void textmode_term_scroll() {
    for (size_t y = 1; y < term_height; y++) {
        for (size_t x = 0; x < term_width; x++) {
            vga_buffer[(y - 1) * term_width + x] = vga_buffer[y * term_width + x];
        }
    }

    for (size_t x = 0; x < term_width; x++) {
        vga_buffer[(term_height - 1) * term_width + x] = vga_entry(' ', term_fg | term_bg << 4);
    }
}

void textmode_term_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * term_width + x;
    vga_buffer[index] = vga_entry(c, color);
}

extern uint8_t _binary_src_terminus_psf_start[];
extern uint8_t _binary_src_terminus_psf_end[];

extern multiboot_info_t* mbi;

extern psf_font_t *font;

extern bool supports_vbe;

void term_create(size_t width, size_t height, uint32_t fg, uint32_t bg) {
    
    if(supports_vbe) {
        font = (psf_font_t*)&_binary_src_terminus_psf_start;
        term_width  = width  / font->width;   // convert px to char cells
        term_height = height / font->height;
        term_fg = fg;
        term_bg = bg;
        term_row = 0;
        term_column = 0;
        fb_clear(bg);
    } else {
        term_width  = width;
        term_height = height;
        term_fg = fg;
        term_bg = bg;
        term_row = 0;
        term_column = 0;
        textmode_term_clear();
        textmode_term_enable_cursor(0, 15);
    }
}

void term_draw_cursor() {
    int x = term_column * font->width;
    int y = term_row * font->height + font->height - 2;
    fb_draw_rect(x, y, font->width, 2, term_fg);
    cursor_visible = true;
}

void term_erase_cursor() {
    int x = term_column * font->width;
    int y = term_row * font->height + font->height - 2;
    fb_draw_rect(x, y, font->width, 2, term_bg);
    cursor_visible = false;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while(str[len]) {
        len++;
    }
    return len;
}

void term_clear() {
    fb_clear(term_bg);   
    term_row = 0;
    term_column = 0;
}

void term_set_color(uint32_t fg, uint32_t bg) {
    term_fg = fg;
    term_bg = bg;
}

void term_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    psf_putchar(x, y, c, term_fg, term_bg);
}

void term_scroll() {
    uint32_t char_h = font->height;
    uint32_t char_w = font->width;
    uint32_t pitch  = mbi->framebuffer_pitch;
    uint32_t bpp    = mbi->framebuffer_bpp / 8;

    volatile uint8_t *fb = (volatile uint8_t*)(uintptr_t)mbi->framebuffer_addr;

    size_t row_bytes = pitch * char_h;
    size_t total = row_bytes * (term_height - 1);
    memmove((void*)fb, (void*)(fb + row_bytes), total);

    volatile uint8_t *last_row = fb + row_bytes * (term_height - 1);
    for (uint32_t y = 0; y < char_h; y++) {
        for (uint32_t x = 0; x < term_width * char_w; x++) {
            uint32_t *px = (uint32_t*)(last_row + y * pitch + x * bpp);
            *px = term_bg;
        }
    }
}

void term_put_char(char c) {
    if(supports_vbe) term_erase_cursor();

    if (c == '\b') {
        if (term_column == 0 && term_row > 0) {
            term_row--;
            term_column = term_width;
        }
        if (term_column > 0) {
            term_column--;
        }

        if(supports_vbe) {
            psf_putchar(term_column * font->width, term_row * font->height, ' ', term_fg, term_bg);
        } else {
            textmode_term_put_entry_at(' ', term_fg | term_bg << 4, term_column, term_row);
            textmode_term_set_cursor_pos(term_row, term_column);
        } 
        return;
    }

    if (c == '\n') {
        term_column = 0;
        term_row++;
        if(!supports_vbe) textmode_term_set_cursor_pos(term_row, term_column);
    } else {
        if(supports_vbe) {
            psf_putchar(term_column * font->width, term_row * font->height, (unsigned char)c, term_fg, term_bg);
        } else {
            textmode_term_put_entry_at(c, term_fg | term_bg << 4, term_column, term_row);
            textmode_term_set_cursor_pos(term_row, term_column+1);
        }

        if (++term_column >= term_width) {
            term_column = 0;
            term_row++;
        }
    }

    if (term_row >= term_height) {
        if(supports_vbe) {
            term_scroll();
        } else {
            textmode_term_scroll();
        }
        term_row = term_height - 1;
    }

    if(supports_vbe) {
        term_draw_cursor();
    } else {
        textmode_term_set_cursor_pos(term_row, term_column+1);
    }
}

void term_write(const char* data, size_t size) {
    for(size_t i = 0; i < size; i++) {
        term_put_char(data[i]);
    }
}

void term_write_string(const char* data) {
    term_write(data, strlen(data));
}

void term_set_uppercase(bool up) {

}

void kprint(const char *str) {
    while (*str) {
        term_put_char((int)*str);
        ++str;
    }
}

int kputs(const char *str) {
    kprint(str);
    term_put_char((int)'\n');
    return 0;
}


