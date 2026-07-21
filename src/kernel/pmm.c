#include "kernel/multiboot.h"
#include "kernel/pmm.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

extern void* _kernel_end;

uint8_t* pmm_bitmap = NULL;
size_t bitmap_size = 0;

extern multiboot_info_t* mbi;
multiboot_memory_map_entry_t* mmap;

uint32_t max_addr = 0;

void pmm_set_bit(uint32_t frame_idx) {
	pmm_bitmap[frame_idx / 8] |= (1 << (frame_idx % 8));
}

void pmm_clear_bit(uint32_t frame_idx) {
	pmm_bitmap[frame_idx / 8] &= ~(1 << (frame_idx % 8));

}

int pmm_test_bit(uint32_t frame_idx) {
	return (pmm_bitmap[frame_idx / 8] & (1 << (frame_idx % 8))) != 0;
}

void pmm_init() {

	mmap = (multiboot_memory_map_entry_t*)mbi->mmap_addr;
	
	uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;
	uintptr_t current = mbi->mmap_addr;

	while(current < mmap_end) {
		multiboot_memory_map_entry_t* entry = (multiboot_memory_map_entry_t*)current;

		//find end of physical mem
		if(entry->type == 1) {
			uint32_t region_end = (uint32_t)entry->base_addr + (uint32_t)entry->length;
			if(region_end > max_addr) {
				max_addr = region_end;
			}
		}
		current += entry->size + sizeof(entry->size);
	}

	//calc size of bmp from phys mem size/end
	uint32_t frame_count = max_addr / 4096;
	bitmap_size = frame_count / 8;
	if(frame_count % 8 != 0) {
		bitmap_size++;
	}

	pmm_bitmap = (uint8_t*)&_kernel_end;

	memset(pmm_bitmap, 0xFF, bitmap_size);

	current = mbi->mmap_addr;

	//second loop to check for usable ram
	while(current < mmap_end) {
		multiboot_memory_map_entry_t* entry = (multiboot_memory_map_entry_t*)current;

		if(entry->type == 1) {
    		uint32_t base_frame = (entry->base_addr + 4095) / 4096;
		    uint32_t end_frame = (entry->base_addr + entry->length) / 4096;

		    if (end_frame > base_frame) {
        		for(uint32_t f = base_frame; f < end_frame; f++) {
		            pmm_clear_bit(f);
        		}
    		}
			current += entry->size + sizeof(entry->size);
		}
	}


	//mask out kernel (bmp addr+size is end of all kernel ram since bmp is mapped directly after it)
	uint32_t kernel_end_frame = ((uint32_t)pmm_bitmap + bitmap_size) / 4096;
	for(uint32_t frame = 0; frame <= kernel_end_frame; frame++) {
		pmm_set_bit(frame);
	}
}


uint32_t pmm_alloc_frame() {
	for(uint32_t i = 0; i < bitmap_size; i++) {
		if(pmm_bitmap[i] == 0xFF) continue;

		uint8_t first_zero_bit_idx = __builtin_ctz(~pmm_bitmap[i]);

		uint32_t frame_idx = (i * 8) + first_zero_bit_idx;
		if(frame_idx >= (max_addr / 4096)) return PMM_ERROR; //out of ram bounds
		
		pmm_set_bit(frame_idx);

		return frame_idx * 4096;
	}
	return PMM_ERROR;
}

void pmm_free_frame(uint32_t phys_addr) {
	uint32_t frame_idx = phys_addr / 4096;
	pmm_clear_bit(frame_idx);
}
