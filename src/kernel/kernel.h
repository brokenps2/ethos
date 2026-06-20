#include <stdint.h>
#include <stddef.h>
#pragma once

typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, error_code;
    uint32_t eip, cs, eflags, useresp, ss;
} cpu_state_t;

void* kmalloc(size_t size);
