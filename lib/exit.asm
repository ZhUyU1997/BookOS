[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_EXIT EQU 10

global sub_exit
sub_exit:
	push ebx
	mov eax, _NR_EXIT
	mov ebx, [esp + 4 + 4]
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret
