#include <stdint.h>
#include "vga.h"
#include "keyboard.h"
#include "arch/i386/ports.h"
#include "stdio.h"
#include "string.h"
#include "pit.h"
#include <stddef.h>

size_t termWidth;
size_t termHeight;
uint16_t* termBuffer;
size_t termRow;
size_t termColumn;
size_t cursorRow;
size_t cursorColumn;
uint8_t termColor;


char line[128];
int len = 0;


void term_enable_cursor(uint8_t start, uint8_t end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

void term_disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void term_set_cursor_pos(int row, int column) {
    cursorRow = row;
    cursorColumn = column;
    uint16_t pos = cursorRow * termWidth + cursorColumn;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void term_create(size_t width, size_t height, uint16_t* address, size_t row, size_t column, VGAColor fg, VGAColor bg) {
    termWidth = width;
    termHeight = height;
    termBuffer = address;
    termRow = row;
    termColumn = column;
    cursorRow = row;
    cursorColumn = column;
    termColor = vga_entry_color(fg, bg);
    for(size_t y = 0; y < termHeight; y++) {
        for(size_t x = 0; x < termWidth; x++) {
            const size_t index = y * termHeight + x;
            termBuffer[index] = vga_entry(' ', termColor);
        }
    }
    printf(">");
}

size_t strlen(const char* str) {
    size_t len = 0;
    while(str[len]) {
        len++;
    }
    return len;
}

void term_clear() {
    for(int y = 0; y < termHeight; y++) {
        for(int x = 0; x < termWidth; x++) {
            int cpos = (y * termWidth) + x;
            termBuffer[cpos] = ' ';
        }
    }
    termRow = 0;
    termColumn = 0;
    term_set_cursor_pos(1, 1);
    term_enable_cursor(0, 15);
}

void term_set_color(VGAColor fg, VGAColor bg) {
    termColor = vga_entry_color(fg, bg);
}

void term_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * termWidth + x;
    termBuffer[index] = vga_entry(c, color);
}

void term_scroll() {
    for (size_t y = 1; y < termHeight; y++) {
        for (size_t x = 0; x < termWidth; x++) {
            termBuffer[(y - 1) * termWidth + x] = termBuffer[y * termWidth + x];
        }
    }

    for (size_t x = 0; x < termWidth; x++) {
        termBuffer[(termHeight - 1) * termWidth + x] = vga_entry(' ', termColor);
    }
}

void term_put_char(char c) {
    if (c == '\b') {
        if (termColumn == 0 && termRow > 0) {
            termRow--;
            termColumn = termWidth;
        }
        if (termColumn > 0) {
            termColumn--;
        }

        term_put_entry_at(' ', termColor, termColumn, termRow);
        term_set_cursor_pos(termRow, termColumn);
        return;
    }

    if (c == '\n') {
        termColumn = 0;
        termRow++;
    } else {
        term_put_entry_at(c, termColor, termColumn, termRow);

        if (++termColumn >= termWidth) {
            termColumn = 0;
            termRow++;
        }
    }

    if (termRow >= termHeight) {
        term_scroll();
        termRow = termHeight - 1;
    }

    term_set_cursor_pos(termRow, termColumn);
}

void term_write(const char* data, size_t size) {
    for(size_t i = 0; i < size; i++) {
        term_put_char(data[i]);
    }
}

void term_write_string(const char* data) {
    term_write(data, strlen(data));
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

uint8_t min_col = 0;

void print_prompt() {
    printf("> ");
    min_col = termColumn;
}

void handle_command(char* cmd) {
    if(strcmp(cmd, "help") == 0) {
        printf("help clear ticks\n");
    } else if(strcmp(cmd, "clear") == 0) {
        term_clear();
    } else if(strcmp(cmd, "ticks") == 0) {
        printf("%d\n", pit_get_ticks());
    } else {
        printf("unknown command\n");
    }
    print_prompt();
}

extern int tail, head;
extern char keybuf[];

void process_key_input() {
    while (tail != head) {
        unsigned char scancode = keybuf[tail];
        tail = (tail + 1) % KEYBUF_SIZE;

        if (!(scancode & 0x80)) {
            char c = keymapUS[scancode];
            if (c == '\n') {
                line[len] = 0;
                len = 0;
                term_put_char('\n');
                handle_command(line);
	        } else if(c == '\b') {
                if(len > 0) {
                    len--;
                    term_put_char('\b');
                }
            } else {
                line[len++] = c;
                term_put_char(c);
            }
        }
    }
}
