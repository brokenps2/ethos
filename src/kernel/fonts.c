#include <stdint.h>
#include "drivers/vga.h"
#include "string.h"
#include "kernel/kernel.h"

extern uint8_t _binary_src_terminus_psf_start[];
extern uint8_t _binary_src_terminus_psf_end[];

PSFFont *font;

uint16_t *unicode;

void psf_init() {
    font = (PSFFont*)&_binary_src_terminus_psf_start;

    if (font->magic != PSF_FONT_MAGIC) {
        unicode = NULL;
        return;
    }

    if (font->flags == 0) {
        unicode = NULL;
        return;
    }

    unsigned char* s = (unsigned char*)&_binary_src_terminus_psf_start + font->headerSize + font->glyphCount * font->bytesPerGlyph;
    unsigned char* end = (unsigned char*)&_binary_src_terminus_psf_end;

    unicode = kmalloc(sizeof(uint16_t) * 65536);
    memset(unicode, 0, sizeof(uint16_t) * 65536);

    uint16_t glyph = 0;
    while (s < end) {
        uint16_t uc;

        if (*s == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if (*s & 0x80) {
            if ((*s & 0xE0) == 0xC0) {
                uc = ((*s & 0x1F) << 6) | (s[1] & 0x3F);
                s += 2;
            } else if ((*s & 0xF0) == 0xE0) {
                uc = ((*s & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
                s += 3;
            } else if ((*s & 0xF8) == 0xF0) {
                uc = 0;
                s += 4;
            } else {
                s++;
                continue;
            }
        } else {
            uc = *s;
            s++;
        }

        unicode[uc] = glyph;
    }
}

void psf_putchar(int x, int y, uint32_t codepoint, uint32_t fg, uint32_t bg) {
    uint16_t glyph_index = unicode ? unicode[codepoint] : codepoint;

    unsigned char *glyph = (unsigned char*)&_binary_src_terminus_psf_start
                           + font->headerSize
                           + glyph_index * font->bytesPerGlyph;

    int bytes_per_row = (font->width + 7) / 8;

    for (uint32_t row = 0; row < font->height; row++) {
        for (uint32_t col = 0; col < font->width; col++) {
            int byte = col / 8;
            int bit  = 7 - (col % 8);
            uint32_t color = (glyph[row * bytes_per_row + byte] >> bit) & 1
                             ? fg : bg;
            fb_put_pixel(x + col, y + row, color);
        }
    }
}

void psf_puts(int x, int y, const char *str, uint32_t fg, uint32_t bg) {
    int cx = x, cy = y;
    while (*str) {
        if (*str == '\n') {
            cy += font->height;
            cx = x;
        } else {
            psf_putchar(cx, cy, (unsigned char)*str, fg, bg);
            cx += font->width;
        }
        str++;
    }
}

