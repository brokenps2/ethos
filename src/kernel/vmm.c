#include <stdint.h>
#include "vmm.h"

__attribute__((aligned(4096))) uint32_t page_dir[1024];
__attribute__((aligned(4096))) uint32_t page_table[1024];

extern void vmm_enable();

void vmm_init() {

	for(int i = 0; i < 1024; i++) {
		page_dir[i] = 0 | PAGE_RW;
	}

	for(uint32_t i = 0; i < 1024; i++) {
		uint32_t phys_addr = i * 4096;
		page_table[i] = phys_addr | PAGE_PRESENT | PAGE_RW;
	}

	page_dir[0] = ((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW;

	vmm_enable();
}


