[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_PUTCH EQU 21
global 	putch
putch:
	push ebx
	mov eax, _NR_PUTCH
	mov ebx, [esp + 4 + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
