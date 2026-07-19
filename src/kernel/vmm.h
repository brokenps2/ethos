#pragma once

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4 //I don't think I'll actually end up using ring 3 but this is here just in case

void vmm_init();
