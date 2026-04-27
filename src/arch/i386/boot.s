MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
VIDEO     equ  1 << 2            ; enable video info
MBFLAGS  equ  MBALIGN | MEMINFO | VIDEO; | VIDEO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + MBFLAGS) ; checksum of above, to prove we are multiboot
                                ; CHECKSUM + MAGIC + MBFLAGS should be Zero (0)
VMODE   equ 0
COLUMNS equ 1024
ROWS    equ 768
DEPTH   equ 0

section .multiboot
align 4
    dd MAGIC        ; use the EQU constants, not lowercase names
    dd MBFLAGS
    dd CHECKSUM
    dd 0            ; header_addr   (0 = not used)
    dd 0            ; load_addr     (0 = not used)
    dd 0            ; load_end_addr (0 = not used)
    dd 0            ; bss_end_addr  (0 = not used)
    dd 0            ; entry_addr    (0 = not used)
    dd VMODE        ; mode_type
    dd COLUMNS      ; width
    dd ROWS         ; height
    dd DEPTH        ; depth

section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB is reserved for stack
stack_top:

section .text
global _start:function (_start.end - _start)
_start:
    
    mov esp, stack_top

    push ebx
    push eax

    extern kernel_main
    call kernel_main

    cli
.hang:    hlt
    jmp .hang
.end:
