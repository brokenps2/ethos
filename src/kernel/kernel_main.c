#include "arch/i386/gdt.h"
#include "arch/i386/idt.h"
#include "drivers/terminal.h"
#include "drivers/vga.h"
#include "kernel/multiboot.h"
#include "kernel/fonts.h"
#include "kernel/commandline.h"
#include <stdint.h>
#include <stdbool.h>

bool supportsVBEFramebuffer = false;

mb2_tag_framebuffer* fb_tag;

int kernel_main(uint32_t magic, uint32_t* mbi_addr) {

	if(magic != 0x36D76289) {
		while(1);
	}

	mb2_tag* tag = (mb2_tag*)((uint8_t*)mbi_addr + 8);

	while(tag->type != 0) {
		if(tag->type == 8) { //set fb_tag to the tag with tag type framebuffer
			fb_tag = (mb2_tag_framebuffer*)tag;
			if(fb_tag->fb_type == 1) {
				supportsVBEFramebuffer = true;
			}
			break;
		}
		tag = (mb2_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7));
	}

	if(supportsVBEFramebuffer) {
		term_create(fb_tag->fb_width, fb_tag->fb_height, 0x00FFFFFF, 0x00000000);
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
