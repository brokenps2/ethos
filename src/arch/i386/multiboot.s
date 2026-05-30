MAGIC equ 0xE85250D6
ARCH equ 0

section .multiboot
align 8

header_start:
    dd MAGIC
    dd ARCH
    dd header_end - header_start
    dd -(MAGIC + ARCH + (header_end - header_start))
    
    ; tag 1 - video
    align 8
    dw 5 ; tag type (5 - framebuffer related)
    dw 1 ; flags (1 - optional)
    dd 20 ; tag size (2 words (4 bytes) + 4 dwords (16 bytes))
    dd 640 ; width
    dd 480 ; height
    dd 32 ; bpp

    ; tag 2 - end tag
    align 8
    dw 0 ; tag type (0 - end of tags)
    dw 0 ; flags
    dd 8 ; tag size (8)

header_end:


section .bss
align 16
stack_bottom:
resb 16384
stack_top:


section .text
global _start:function (_start.end = _start)
_start:
    mov esp, stack_top

    push ebx
    push eax

    extern kernel_main
    call kernel_main

.hang:
    hlt
    jmp .hang

.end:

