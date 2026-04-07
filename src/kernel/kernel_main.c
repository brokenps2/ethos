#include "arch/i386/gdt.h"
#include "arch/i386/idt.h"
#include "arch/i386/irq.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "drivers/terminal.h"
#include "kernel/multiboot.h"
#include "kernel/fonts.h"
#include "kernel/utils.h"
#include <stdint.h>


multiboot_info_t* mbi;

int kernel_main(uint32_t magic, uint32_t mbi_addr) {

	if(magic != 0x2BADB002) {
		while(1);
	}

	mbi = (multiboot_info_t*)mbi_addr;


	gdt_init();
	idt_init();
	irq_install();
	
	//term_create(80, 25, (uint16_t*)0xB8000, 0, 0, VGA_COLOR_LIGHT_GRAY, VGA_COLOR_BLACK);

	asm volatile("sti");

	psf_init();
	psf_puts(10, 10, "test", 0x00FFFFFF, 0x00000000);

	while (1) {
		process_key_input();
		asm volatile("hlt");
	}
	return 0;
}
