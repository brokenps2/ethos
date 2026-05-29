#include "arch/i386/ports.h"
#include "keyboard.h"
#include <stdbool.h>

char keybuf[KEYBUF_SIZE];
int head = 0, tail = 0;

bool uppercase = false;
bool extended = false;

void keyboard_handler() {
    unsigned char scancode = inb(0x60);

    if (scancode == 0xE0) {
        extended = true;
        end_of_int();
        return;
    }

    int next = (head + 1) % KEYBUF_SIZE;
    if(next != tail) {
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
