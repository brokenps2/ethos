#include "drivers/terminal.h"
#include "drivers/vga.h"
#include "kernel/kernel.h"
#include "stdio.h"
#include <stdbool.h>

const char* exception_msgs[] = {
    "EX0: Division by 0 Exception",
    "EX1: Debug Exception",
    "EX2: Non Maskable Interrupt Exception",
    "EX3: Breakpoint Exception",
    "EX4: Into Detected Overflow Exception",
    "EX5: Out of Bounds Exception",
    "EX6: Invalid Opcode Exception",
    "EX7: No Co-Processor Exception",
    "EX8: Double Fault",
    "EX9: Co-Processor Segment Overrun Exception",
    "EX10: Bad TSS Exception",
    "EX11: Segment Not Present Exception",
    "EX12: Stack Fault",
    "EX13: General Protection Fault",
    "EX14: Page Fault Exception",
    "EX15: Unknown Interrupt Exception",
    "EX16: Co-Processor Fault Exception",
    "EX17: Alignment Check Exception",
    "EX18: Machine Check Exception",
    "EX19-31: Reserved Exception"
};

extern bool supports_vbe;

void exception_handler(CpuState* state) {

    term_write_string("\n\n");
    if(supports_vbe) {
        term_set_color(0x00EE0000, 0x00000000);
    } else {
        term_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
    }
    printf("%s, Error Code: %d\n", exception_msgs[state->int_no], state->error_code);
    printf("EAX: %x  EBK: %x  ECX: %x  EDX: %x\n", state->eax, state->ebx, state->ecx, state->edx);
    printf("EIP: %x  CS:  %x  EFLAGS: %x\n", state->eip, state->cs, state->eflags);
    asm volatile("hlt");
}
