#include <stdint.h>

#pragma once

void ata_read_sector(uint32_t lba, uint8_t* buffer);
