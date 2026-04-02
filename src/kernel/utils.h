#pragma once
#include <stdint.h>
#include <stddef.h>

#define ALIGN4(x) (((x) + 3) & ~3);

void sleep_busy(uint32_t ticks);
void* kmalloc(size_t size);
