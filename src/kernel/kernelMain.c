#include "arch/i386/gdt.h"
#include "arch/i386/idt.h"
#include "arch/i386/irq.h"
#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include <stdint.h>

int kernelMain() {

	gdt_init();
	idt_init();
	irq_install();

	term_create(80, 25, (uint16_t*)0xB8000, 0, 0, VGA_COLOR_LIGHT_GRAY, VGA_COLOR_BLACK);

	asm volatile("sti");

	
	while (1) {
		process_key_input();
		asm volatile("hlt");
	}
	return 0;
}
