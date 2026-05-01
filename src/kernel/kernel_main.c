#include "arch/i386/gdt.h"
#include "arch/i386/idt.h"
#include "drivers/terminal.h"
#include "kernel/multiboot.h"
#include "kernel/fonts.h"
#include "kernel/commandline.h"
#include "stdio.h"
#include <stdint.h>

multiboot_info_t* mbi;

int kernel_main(uint32_t magic, uint32_t* mbi_addr) {

	if(magic != 0x2BADB002) {
		while(1);
	}

	mbi = (multiboot_info_t*)mbi_addr;

	term_create(mbi->framebuffer_width, mbi->framebuffer_height, 0x00FFFFFF, 0x00090815);
	psf_init();

	gdt_init();
	idt_init();

	//pit_wait(30);
	
	asm volatile("sti");

	printf("\nethos kernel console -- build date %s\n\n", __DATE__);
	print_prompt();


	while (1) {
		scan_kernel_cmdline();
		asm volatile("hlt");
	}
	return 0;
}
