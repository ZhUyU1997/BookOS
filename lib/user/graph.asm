[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_CREATE_WINDOW EQU 31
_NR_CLOSE_WINDOW EQU 32

global create_window2
create_window2:
	push ebx
   	push ebx
	push ecx
	mov eax, _NR_CREATE_WINDOW
	mov ebx, [esp + 4*3 + 4]		;第一个参数
	mov edx, [esp + 4*3 + 8]		;第一个参数
	mov ecx, [esp + 4*3 + 12]		;第一个参数
	int INT_VECTOR_SYS_CALL
	pop ecx
	pop edx
	pop ebx
	ret
	
global window_close2
window_close2:
	mov eax, _NR_CLOSE_WINDOW
	int INT_VECTOR_SYS_CALL
	ret	
