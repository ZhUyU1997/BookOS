[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_PS EQU 13

global 	ps
ps:
	push edx
	push ebx
	mov eax, _NR_PS
	mov ebx, [esp + 8 + 4]		;第一个参数
	mov edx, [esp + 8 + 4*2]		;第一个参数
	int INT_VECTOR_SYS_CALL
	pop ebx
	pop edx
	ret
