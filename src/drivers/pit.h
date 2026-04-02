#pragma once
#include <stdint.h>

void pit_set_phase(uint32_t hz);
void pit_wait(uint32_t ticks);
void pit_handler();
uint32_t pit_get_ticks();
