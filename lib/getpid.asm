[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GETPID EQU 12

global getpid
getpid:
	mov eax, _NR_GETPID
	int INT_VECTOR_SYS_CALL
	ret