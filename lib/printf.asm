[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_PRINTF EQU 22

global printf
printf:
   	push ebx
	push edx
	mov eax, _NR_PRINTF
	mov ebx, [esp + 8 + 4]		;第一个参数
	mov edx, [esp + 8 + 8]		;第一个参数
	int INT_VECTOR_SYS_CALL
	pop edx
	pop ebx
	ret