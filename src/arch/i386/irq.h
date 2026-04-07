#pragma once
#include <stdint.h>

void irq_set_handler(uint8_t irq);
void irq_uninstall_handler(uint8_t irq);
void irq_remap();
void irq_install();

