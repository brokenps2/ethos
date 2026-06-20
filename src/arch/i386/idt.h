#pragma once
#include <stdint.h>

typedef struct {
	uint16_t isr_low;
	uint16_t selector;
	uint8_t reserved;
	uint8_t attribs;
	uint16_t isr_high;
} __attribute__((packed)) IdtEntry;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) IdtPointer;

void idt_set_entry(int i, void* isr, uint8_t flags);
void idt_init();

