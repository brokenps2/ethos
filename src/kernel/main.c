#include "arch/i386/gdt.h"
#include "arch/i386/idt.h"
#include "drivers/terminal.h"
#include "drivers/vga.h"
#include "kernel/multiboot.h"
#include "kernel/fonts.h"
#include "kernel/cmdline.h"
#include <stdint.h>
#include <stdbool.h>

multiboot_info_t* mbi;
bool supports_vbe = false;

int kernel_main(uint32_t magic, uint32_t* mbi_addr) {

	if(magic != 0x2BADB002) {
		while(1);
	}

	mbi = (multiboot_info_t*)mbi_addr;

	supports_vbe = ((mbi->flags & (1 << 12)) && mbi->framebuffer_type == 1) ? true : false;


	if(supports_vbe) {
		term_create(mbi->framebuffer_width, mbi->framebuffer_height, 0x00FFFFFF, 0x00000000);
		psf_init();
	} else {
		term_create(80, 25, VGA_COLOR_LIGHT_GRAY, VGA_COLOR_BLACK);
	}

	gdt_init();
	idt_init();

	asm volatile("sti");

	print_prompt();


	while (1) {
		scan_kernel_cmdline();
		asm volatile("hlt");
	}
	return 0;
}
