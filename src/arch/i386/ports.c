#include <stdint.h>

void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %b0, %w1": : "a"(val), "Nd"(port) : "memory");
}

void outw(uint16_t port, uint16_t val) {
	asm volatile("outw %0, %1" :: "a"(val), "Nd"(port));
}

char inb(uint16_t port) {
    char rv;
    asm volatile("inb %1, %0" : "=a"(rv):"dN"(port));
    return rv;
}

uint16_t inw(uint16_t port) {
    uint16_t rv;
    asm volatile("inw %1, %0" : "=a"(rv):"Nd"(port));
    return rv;
}

void outl(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %1" :: "a"(val), "Nd"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t val;
    asm volatile("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void ioWait() {
    outb(0x80, 0);
}

void end_of_int() {
    outb(0x20, 0x20);
}
