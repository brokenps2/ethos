#include <stdint.h>
#pragma once

#define DISK_SECTOR_CONFIG 1
#define DISK_SECTOR_PROGRAM 64

void ata_read_sector(uint32_t lba, uint8_t* buffer);
void ata_write_sector(uint32_t lba, uint8_t* buf);
void ata_write_sectors(uint32_t lba, uint32_t count, uint8_t* buf);
int ata_detect();
