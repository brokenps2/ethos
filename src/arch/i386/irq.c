#include <stdint.h>
#include <stdio.h>
#include "idt.h"
#include "drivers/keyboard.h"
#include "kernel/kernel.h"
#include "ports.h"
#include "drivers/pit.h"

void* irq_routines[16] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void irq_set_handler(uint8_t irq, void (*handler)(CpuState state)) {
	irq_routines[irq] = handler;
}

void irq_uninstall_handler(uint8_t irq) {
	irq_routines[irq] = 0;
}

void irq_remap() {
	outb(0x20, 0x11);
	ioWait();
	outb(0xA0, 0x11);
	ioWait();
	outb(0x21, 0x20);
	ioWait();
	outb(0xA1, 0x28);
	ioWait();
	outb(0x21, 0x04);
	ioWait();
	outb(0xA1, 0x02);
	ioWait();
	outb(0x21, 0x01);
	ioWait();
	outb(0xA1, 0x01);
	ioWait();
	outb(0x21, 0x00);
	ioWait();
	outb(0xA1, 0x00);
	ioWait();
}

void irq_install() {
	irq_remap();
	idt_set_entry(32, irq0, 0x8E);
	idt_set_entry(33, irq1, 0x8E);
	idt_set_entry(34, irq2, 0x8E);
	idt_set_entry(35, irq3, 0x8E);
	idt_set_entry(36, irq4, 0x8E);
	idt_set_entry(37, irq5, 0x8E);
	idt_set_entry(38, irq6, 0x8E);
	idt_set_entry(39, irq7, 0x8E);
	idt_set_entry(40, irq8, 0x8E);
	idt_set_entry(41, irq9, 0x8E);
	idt_set_entry(42, irq10, 0x8E);
	idt_set_entry(43, irq11, 0x8E);
	idt_set_entry(44, irq12, 0x8E);
	idt_set_entry(45, irq13, 0x8E);
	idt_set_entry(46, irq14, 0x8E);
	idt_set_entry(47, irq15, 0x8E);
	irq_set_handler(0, pit_handler);
	irq_set_handler(1, keyboard_handler);
}

void irq_handler(CpuState* state) {
	void (*handler)(CpuState* state);

	handler = irq_routines[state->int_no - 32];
	if(handler) {
		handler(state);
	}

	if(state->int_no >= 40) {
		outb(0xA0, 0x20);
	}

	outb(0x20, 0x20);
}
