[BITS 32]

extern page_dir
global vmm_enable

vmm_enable:
	mov eax, page_dir
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	ret
