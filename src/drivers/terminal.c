#include "drivers/terminal.h"
#include <stdint.h>
#include "drivers/ata.h"
#include "drivers/fat32.h"
#include "kernel/fonts.h"
#include "vga.h"
#include <stdbool.h>
#include "arch/i386/ports.h"
#include "kernel/multiboot.h"
#include "stdio.h"
#include "string.h"
#include <stddef.h>

size_t termWidth;      // in characters
size_t termHeight;     // in characters
size_t termRow;
size_t termColumn;
uint32_t termFG;
uint32_t termBG;

bool cursor_visible = false;

char line[128];
int len = 0;


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
    uint16_t pos = row * termWidth + column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}


extern uint8_t _binary_src_terminus_psf_start[];
extern uint8_t _binary_src_terminus_psf_end[];

extern multiboot_info_t* mbi;

extern PSFFont *font;

void term_create(size_t width, size_t height, uint32_t fg, uint32_t bg) {
    font = (PSFFont*)&_binary_src_terminus_psf_start;

    termWidth  = width  / font->width;   // convert px to char cells
    termHeight = height / font->height;
    termFG = fg;
    termBG = bg;
    termRow = 0;
    termColumn = 0;
    fb_clear(bg);
}

void term_draw_cursor() {
    int x = termColumn * font->width;
    int y = termRow * font->height + font->height - 2;
    fb_draw_rect(x, y, font->width, 2, termFG);
    cursor_visible = true;
}

void term_erase_cursor() {
    int x = termColumn * font->width;
    int y = termRow * font->height + font->height - 2;
    fb_draw_rect(x, y, font->width, 2, termBG);
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
    fb_clear(termBG);   
    termRow = 0;
    termColumn = 0;
}

void term_set_color(uint32_t fg, uint32_t bg) {
    termFG = fg;
    termBG = bg;
}

void term_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    psf_putchar(x, y, c, termFG, termBG);
}

void term_scroll() {
    uint32_t char_h = font->height;
    uint32_t char_w = font->width;
    uint32_t pitch  = mbi->framebuffer_pitch;
    uint32_t bpp    = mbi->framebuffer_bpp / 8;

    volatile uint8_t *fb = (volatile uint8_t*)(uintptr_t)mbi->framebuffer_addr;

    size_t row_bytes = pitch * char_h;
    size_t total = row_bytes * (termHeight - 1);
    memmove((void*)fb, (void*)(fb + row_bytes), total);

    volatile uint8_t *last_row = fb + row_bytes * (termHeight - 1);
    for (uint32_t y = 0; y < char_h; y++) {
        for (uint32_t x = 0; x < termWidth * char_w; x++) {
            uint32_t *px = (uint32_t*)(last_row + y * pitch + x * bpp);
            *px = termBG;
        }
    }
}

void term_put_char(char c) {
    term_erase_cursor();

    if (c == '\b') {
        if (termColumn == 0 && termRow > 0) {
            termRow--;
            termColumn = termWidth;
        }
        if (termColumn > 0) {
            termColumn--;
        }

        psf_putchar(termColumn * font->width, termRow * font->height, ' ', termFG, termBG);
        return;
    }

    if (c == '\n') {
        termColumn = 0;
        termRow++;
    } else {
        psf_putchar(termColumn * font->width, termRow * font->height, (unsigned char)c, termFG, termBG);
        if (++termColumn >= termWidth) {
            termColumn = 0;
            termRow++;
        }
    }

    if (termRow >= termHeight) {
        term_scroll();
        termRow = termHeight - 1;
    }

    term_draw_cursor();
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


