
extern exception_handler

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

isr0:
    cli
    push byte 0
    push byte 0
    jmp isrCommonStub
isr1:
    cli
    push byte 0
    push byte 1
    jmp isrCommonStub
isr2:
    cli
    push byte 0
    push byte 2
    jmp isrCommonStub
isr3:
    cli
    push byte 0
    push byte 3
    jmp isrCommonStub
isr4:
    cli
    push byte 0
    push byte 4
    jmp isrCommonStub
isr5:
    cli
    push byte 0
    push byte 5
    jmp isrCommonStub
isr6:
    cli
    push byte 0
    push byte 6
    jmp isrCommonStub
isr7:
    cli
    push byte 0
    push byte 7
    jmp isrCommonStub
isr8:
    cli
    push byte 8
    jmp isrCommonStub
isr9:
    cli
    push byte 0
    push byte 9
    jmp isrCommonStub
isr10:
    cli
    push byte 10
    jmp isrCommonStub
isr11:
    cli
    push byte 11
    jmp isrCommonStub
isr12:
    cli
    push byte 12
    jmp isrCommonStub
isr13:
    cli
    push byte 13
    jmp isrCommonStub
isr14:
    cli
    push byte 14
    jmp isrCommonStub
isr15:
    cli
    push byte 0
    push byte 15
    jmp isrCommonStub
isr16:
    cli
    push byte 0
    push byte 16
    jmp isrCommonStub
isr17:
    cli
    push byte 0
    push byte 17
    jmp isrCommonStub
isr18:
    cli
    push byte 0
    push byte 18
    jmp isrCommonStub
isr19:
    cli
    push byte 0
    push byte 19
    jmp isrCommonStub
isr20:
    cli
    push byte 0
    push byte 20
    jmp isrCommonStub
isr21:
    cli
    push byte 0
    push byte 21
    jmp isrCommonStub
isr22:
    cli
    push byte 0
    push byte 22
    jmp isrCommonStub
isr23:
    cli
    push byte 0
    push byte 23
    jmp isrCommonStub
isr24:
    cli
    push byte 0
    push byte 24
    jmp isrCommonStub
isr25:
    cli
    push byte 0
    push byte 25
    jmp isrCommonStub
isr26:
    cli
    push byte 0
    push byte 26
    jmp isrCommonStub
isr27:
    cli
    push byte 0
    push byte 27
    jmp isrCommonStub
isr28:
    cli	
    push byte 0
    push byte 28
    jmp isrCommonStub
isr29:
    cli
    push byte 0
    push byte 29
    jmp isrCommonStub
isr30:
    cli
    push byte 0
    push byte 30
    jmp isrCommonStub
isr31:
    cli
    push byte 0
    push byte 31
    jmp isrCommonStub

isrCommonStub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, exception_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
