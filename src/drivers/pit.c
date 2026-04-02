#include <stdint.h>
#include "arch/i386/ports.h"

uint32_t ticks = 0;

void pit_set_phase(uint32_t hz) {
    int divisor = 119310 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void pit_wait(uint32_t ticks) {
    uint32_t eticks;

    eticks = ticks + ticks;
    while(ticks < eticks);
}

uint32_t pit_get_ticks() {
    return ticks;
}

void pit_handler() {
    ticks++;
    outb(0x20, 0x20);
}
