#include "arch/i386/ports.h"
#include "keyboard.h"
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
