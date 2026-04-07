#include "arch/i386/ports.h"
#include "keyboard.h"
#include "drivers/terminal.h"
#include <stdbool.h>

char keybuf[KEYBUF_SIZE];
int head = 0, tail = 0;


void keyboard_handler() {
    unsigned char scancode = inb(0x60);

    int next = (head + 1) % KEYBUF_SIZE;
    if (next != tail) {
        keybuf[head] = scancode;
        head = next;
    }

    end_of_int();
}

bool get_key(char* out) {
    if (tail == head)
        return false;

    *out = keybuf[tail];
    tail = (tail + 1) % KEYBUF_SIZE;
    return true;
}

extern char line[128];
extern int len;

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
