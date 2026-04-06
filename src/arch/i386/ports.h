#pragma once
#include <stdint.h>

void outb(uint16_t port, uint8_t val);

char inb(uint16_t port);
uint16_t inw(uint16_t port);

void ioWait();

void end_of_int();
