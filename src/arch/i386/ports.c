#include <stdint.h>

void outb(uint16_t port, uint8_t val) {
	asm volatile("outb %b0, %w1": : "a"(val), "Nd"(port) : "memory");
}

char inb(uint16_t port) {
	char rv;
	asm volatile("inb %1, %0" : "=a"(rv):"dN"(port));
	return rv;
}

uint16_t inw(uint16_t port) {
	uint16_t rv;
	asm volatile("inw %1, %0" : "=a"(rv):"dN"(port));
	return rv;
}


void ioWait() {
	outb(0x80, 0);
}

void end_of_int() {
    outb(0x20, 0x20);
}
