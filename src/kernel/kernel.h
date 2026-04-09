#include <stdint.h>
#include <stddef.h>
#pragma once

typedef struct CPUState {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t intNo, errCode;
    uint32_t eip, cs, eflags, useresp, ss;
} CPUState;

void* kmalloc(size_t size);
