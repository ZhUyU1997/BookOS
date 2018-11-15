[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GETICKS EQU 30

global getticks
getticks:
	mov eax, _NR_GETICKS
	int INT_VECTOR_SYS_CALL
	ret