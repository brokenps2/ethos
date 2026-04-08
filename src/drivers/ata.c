#include "arch/i386/ports.h"
#include "stdio.h"
#include <stdint.h>

#define ATA_PRIMARY_DATA 0x1f0
#define ATA_PRIMARY_STATUS 0x1f7
#define ATA_PRIMARY_COMMAND 0x1f7

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    int timeout = 100000;
    while ((inb(0x1F7) & 0x80) && --timeout);
    if (!timeout) { 
        printf("ATA: BSY timeout\n"); 
        return; 
    }

    while(inb(0x1f7) & 0x80);

    outb(0x1f6, 0xe0 | ((lba >> 24) & 0x0f));
    outb(0x1f2, 1);
    outb(0x1f3, (uint8_t)lba);
    outb(0x1f4, (uint8_t)(lba >> 8));
    outb(0x1f5, (uint8_t)(lba >> 16));
    outb(0x1f7, 0x20);

    while (!(inb(0x1F7) & 0x08));

    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(0x1F0);
        buffer[i * 2]     = (uint8_t)(data);
        buffer[i * 2 + 1] = (uint8_t)(data >> 8);
    }
}


void ata_read_sectors(uint32_t lba, uint32_t count, uint8_t *buf) {
    for (uint32_t i = 0; i < count; i++)
        ata_read_sector(lba + i, buf + i * 512);
}

int ata_detect() {
    outb(0x1F6, 0xA0);
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    outb(0x1F7, 0xEC);

    uint8_t status = inb(0x1F7);
    if (status == 0) {
        printf("ATA: no drive detected\n");
        return 0;
    }

    while (inb(0x1F7) & 0x80);

    if (inb(0x1F4) != 0 || inb(0x1F5) != 0) {
        printf("ATA: not an ATA device\n");
        return 0;
    }

    while (1) {
        status = inb(0x1F7);
        if (status & 0x08) break;
        if (status & 0x01) {
            printf("ATA: identify error\n");
            return 0;
        }
    }

    uint8_t buf[512];
    for (int i = 0; i < 256; i++) {
        uint16_t w = inw(0x1F0);
        buf[i*2]   = w & 0xFF;
        buf[i*2+1] = w >> 8;
    }

    printf("ATA: drive detected\n");
    return 1;
}
