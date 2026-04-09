#include <stdint.h>
#pragma once

#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

/*
* bit 31 = 1
* bits 23-16 = bus number
* bits 15-11 = slot number
* bits 10-8 = function number
* bits 7-0 = register offset (must be 4-byte aligned)
*/

uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);
uint32_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, int bar_num);
void pci_scan(void);
