#pragma once
#include <stdint.h>
#include <stddef.h>
#include "drivers/fat32/fat32.h"

#define ALIGN4(x) (((x) + 3) & ~3);

void sleep_busy(uint32_t ticks);
char* itoa(int num, char* str, int base);
void load_and_run_binary(Fat32Context* fs, const char* filename);
void srand(uint64_t seed);
uint32_t rand(void);
