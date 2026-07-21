#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "vmm.h"
#include "kernel/multiboot.h"
#include "kernel/pmm.h"

__attribute__((aligned(4096))) uint32_t page_dir[1024];
__attribute__((aligned(4096))) uint32_t page_table[1024];

extern void vmm_enable();

extern multiboot_info_t* mbi;

void vmm_map_page(uint32_t* page_dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
	uint32_t dir_idx = virt_addr >> 22;
	uint32_t table_idx = (virt_addr >> 12) & 0x3FF;
	
	uint32_t* table = NULL;

	if((page_dir[dir_idx] & PAGE_PRESENT)) {
		table = (uint32_t*)(page_dir[dir_idx] & ~0x3FF);
	} else {
		uint32_t table_phys = pmm_alloc_frame();
		if(table_phys == PMM_ERROR) return; //this should be a panic() at some point
		table = (uint32_t*)table_phys;
		
		memset(table, 0, 4096);

		page_dir[dir_idx] = table_phys | PAGE_PRESENT | PAGE_RW | flags;
	}
	
	table[table_idx] = (phys_addr & ~0xFFF) | PAGE_PRESENT | flags;

	asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
}

void vmm_init() {

	for(int i = 0; i < 1024; i++) {
		page_dir[i] = 0 | PAGE_RW;
	}

	for(uint32_t i = 0; i < 1024; i++) {
		uint32_t phys_addr = i * 4096;
		page_table[i] = phys_addr | PAGE_PRESENT | PAGE_RW;
	}

	page_table[0xB8000 / 4096] = 0xb8000 | PAGE_PRESENT | PAGE_RW;

	page_dir[0] = ((uintptr_t)page_table) | PAGE_PRESENT | PAGE_RW;







	//none of this works but that's okay

	uint32_t mbi_page = (uint32_t)mbi & ~0xFFF;
	vmm_map_page(page_dir, mbi_page, mbi_page, PAGE_RW);

	if (mbi->flags & (1 << 12)) {
		uint32_t fb_size = mbi->framebuffer_pitch * mbi->framebuffer_height;
		for(uint32_t offset = 0; offset < fb_size; offset += 4096) {
			uint32_t page_addr = mbi->framebuffer_addr + offset;
			vmm_map_page(page_dir, page_addr, page_addr, PAGE_RW);
		}
	}

	vmm_enable();
}


