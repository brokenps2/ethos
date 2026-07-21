#include <stdint.h>
#pragma once

#define PMM_ERROR 0xFFFFFFFF

void pmm_set_bit(uint32_t frame_idx);
void pmm_clear_bit(uint32_t frame_idx);
int pmm_test_bit(uint32_t frame_idx);
void pmm_init();
uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t phys_addr);

