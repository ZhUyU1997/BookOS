[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_MALLOC EQU 3
_NR_FREE EQU 4

global malloc
malloc:
   	push ebx
	mov eax, _NR_MALLOC
	mov ebx, [esp + 4 + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
global free
free:
   	push ebx
	mov eax, _NR_FREE
	mov ebx, [esp + 4 + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
