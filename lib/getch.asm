[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GETCH EQU 20

global getch
getch:
	mov eax, _NR_GETCH
	int INT_VECTOR_SYS_CALL
	ret