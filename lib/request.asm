[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_REQUEST EQU 0

global request_put
request_put:
   	push ebx
	mov eax, _NR_REQUEST
	mov ebx, [esp + 4 + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
