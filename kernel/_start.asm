KERNEL_STACK_TOP EQU 0x8009fc00

[bits 32]
extern main	

global gdt_limit
global gdt_addr

global _start
_start:
	;put 'K'
	mov dword [0x000b8000+6], 'K'
	mov dword [0x000b8000+7], 0X06
	;init all segment registeres
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, KERNEL_STACK_TOP
	
	sgdt [gdtr_ptr]
	
	call main					;into c mian

stop_run:
	hlt
	jmp stop_run
jmp $	

[section .data]
gdtr_ptr:
gdt_limit: dw 0
gdt_addr: dd 0
