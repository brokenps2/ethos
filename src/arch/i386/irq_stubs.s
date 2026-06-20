global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

irq0:
	cli
	push byte 0
	push byte 32
	jmp irqCommonStub
irq1:
	cli
	push byte 0
	push byte 33
	jmp irqCommonStub
irq2:
	cli
	push byte 0
	push byte 34
	jmp irqCommonStub
irq3:
	cli
	push byte 0
	push byte 35
	jmp irqCommonStub
irq4:
	cli
	push byte 0
	push byte 36
	jmp irqCommonStub
irq5:
	cli
	push byte 0
	push byte 37
	jmp irqCommonStub
irq6:
	cli
	push byte 0
	push byte 38
	jmp irqCommonStub
irq7:
	cli
	push byte 0
	push byte 39
	jmp irqCommonStub
irq8:
	cli
	push byte 0
	push byte 40
	jmp irqCommonStub
irq9:
	cli
	push byte 0
	push byte 41
	jmp irqCommonStub
irq10:
	cli
	push byte 0
	push byte 42
	jmp irqCommonStub
irq11:
	cli
	push byte 0
	push byte 43
	jmp irqCommonStub
irq12:
	cli
	push byte 0
	push byte 44
	jmp irqCommonStub
irq13:
	cli
	push byte 0
	push byte 45
	jmp irqCommonStub
irq14:
	cli
	push byte 0
	push byte 46
	jmp irqCommonStub
irq15:
	cli
	push byte 0
	push byte 47
	jmp irqCommonStub

extern irq_handler

irqCommonStub:
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10
	mov dx, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp
	push eax
	mov eax, irq_handler
	call eax
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret
