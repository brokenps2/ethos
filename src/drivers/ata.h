#include <stdint.h>
#pragma once

#define DISK_SECTOR_CONFIG 1
#define DISK_SECTOR_PROGRAM 64

void ata_read_sector(uint32_t lba, uint8_t* buffer);
int ata_detect();
