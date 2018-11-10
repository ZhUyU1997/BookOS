[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_WAIT EQU 11

global sub_wait
sub_wait:
	push ebx
	mov eax, _NR_WAIT
	mov ebx, [esp + 4 + 4]
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
