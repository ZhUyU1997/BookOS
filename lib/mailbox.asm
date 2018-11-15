[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_PUTMAILBOX  EQU 1
global putmailbox
putmailbox:
	push ebx
	mov eax, _NR_PUTMAILBOX
	mov ebx, [esp + 4 + 4]		;put mailbox
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret

_NR_GETMAILBOX  EQU 2
global getmailbox
getmailbox:
	push ebx
	mov eax, _NR_GETMAILBOX
	mov ebx, [esp + 4 + 4]		;get mailbox
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret	
	