#include <stdint.h>
#include "arch/i386/ports.h"
#include "stdio.h"


uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = (1U << 31) 
        | ((uint32_t)bus  << 16) | ((uint32_t)slot << 11) 
        | ((uint32_t)func <<  8) | (offset & 0xFC);
    outl(0xCF8, addr);
    return inl(0xCFC);
}

void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t addr = (1U << 31)
        | ((uint32_t)bus  << 16) | ((uint32_t)slot << 11)
        | ((uint32_t)func <<  8) | (offset & 0xFC);
    outl(0xCF8, addr);
    outl(0xCFC, val);
}

uint32_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, int bar_num) {
    uint8_t offset = 0x10 + bar_num * 4;
    uint32_t bar = pci_read(bus, slot, func, offset);

    if (bar & 1) {
        return bar & 0xFFFFFFFC;
    } else {
        return bar & 0xFFFFFFF0;
    }
}

void pci_scan(void) {
    for (uint16_t bus = 0; bus < 256; bus++) {

        for (uint8_t slot = 0; slot < 32; slot++) {

            uint32_t id = pci_read(bus, slot, 0, 0x00);
            if ((id & 0xFFFF) == 0xFFFF) continue;

            uint16_t vendor = id & 0xFFFF;
            uint16_t device = id >> 16;
            uint32_t class  = pci_read(bus, slot, 0, 0x08);
            uint8_t  classcode = (class >> 24) & 0xFF;
            uint8_t  subclass  = (class >> 16) & 0xFF;
            uint8_t  progif    = (class >>  8) & 0xFF;

            printf("PCI %d:%d vendor=%x device=%x class=%x:%x:%x\n", bus, slot, vendor, device, classcode, subclass, progif);

            uint8_t header_type = (pci_read(bus, slot, 0, 0x0C) >> 16) & 0xFF;

            if (header_type & 0x80) {
                for (uint8_t func = 1; func < 8; func++) {
                    uint32_t fid = pci_read(bus, slot, func, 0x00);
                    if ((fid & 0xFFFF) == 0xFFFF) continue;
                    printf("  func %d vendor=%x device=%x\n", func, fid & 0xFFFF, fid >> 16);
                }
            }
        }

    }
}

