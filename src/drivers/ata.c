#include "arch/i386/ports.h"
#include <stdint.h>

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    while(inb(0x1f7) & 0x80);

    outb(0x1f6, 0xe0 | ((lba >> 24) & 0x0f));
    outb(0x1f2, 1);
    outb(0x1f3, (uint8_t)lba);
    outb(0x1f4, (uint8_t)lba >> 8);
    outb(0x1f5, (uint8_t)lba >> 16);
    outb(0x1f7, 0x20);

    while(!(inb(0x1f7) & 0x80));

    for(int i = 0; i < 256; i++) {
        ((uint16_t*)buffer)[i] = inw(0x1f0);
    }
}
