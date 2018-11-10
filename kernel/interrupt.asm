
%include "kernel/const.inc"

extern exception_handler
extern irq_table
extern task_ready 
extern sys_call_table
extern tss 

global switch_to
global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error

global intr_exit_clock

CLOCK_IRQ equ 0
KEYBOARD_IRQ equ 1
MOUSE_IRQ equ 12
PRIMARY_IRQ equ 14
SECONDARY_IRQ equ 15

[section .text]
[bits 32]
; 中断和异常 -- 异常
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	push esp
	call	exception_handler
	add	esp, 4*3	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt

global IRQ_clock
IRQ_clock:
	pushad
	push ds
	push es
	push fs
	push gs

	mov dx,ss
	mov ds, dx
	mov es, dx
	
	mov esp, INTERRUPT_STACK_TOP

	in	al, INT_M_CTLMASK	; `.
	or	al, (1 << CLOCK_IRQ)		;  | 屏蔽当前中断
	out	INT_M_CTLMASK, al	; /

	mov	al, EOI			; `. 置EOI位
	out	INT_M_CTL, al		; /

	push CLOCK_IRQ
	call [irq_table + CLOCK_IRQ*4]
	add esp, 4
	
	in	al, INT_M_CTLMASK	; `.
	and	al, ~(1 << CLOCK_IRQ)		;  | 恢复接受当前中断
	out	INT_M_CTLMASK, al	; /
	
	jmp intr_exit_clock
	
global IRQ_keyboard	
IRQ_keyboard:
	pushad
	push ds
	push es
	push fs
	push gs

	mov dx,ss
	mov ds, dx
	mov es, dx
	
	mov esp, INTERRUPT_STACK_TOP

	in	al, INT_M_CTLMASK	; `.
	or	al, (1 << KEYBOARD_IRQ)		;  | 屏蔽当前中断
	out	INT_M_CTLMASK, al	; /
	mov	al, EOI			; `. 置EOI位
	out	INT_M_CTL, al		; /
	
	push KEYBOARD_IRQ
	call [irq_table + KEYBOARD_IRQ*4]
	add esp, 4

	in	al, INT_M_CTLMASK	; `.
	and	al, ~(1 << KEYBOARD_IRQ)		;  | 恢复接受当前中断
	out	INT_M_CTLMASK, al	; /
	
	jmp intr_exit
	
global IRQ_mouse	
IRQ_mouse:
	pushad
	push ds
	push es
	push fs
	push gs

	mov dx,ss
	mov ds, dx
	mov es, dx
	
	mov esp, INTERRUPT_STACK_TOP
	
	in	al, INT_S_CTLMASK	; `.
	or	al, (1 << (MOUSE_IRQ - 8))	;  | 屏蔽当前中断
	out	INT_S_CTLMASK, al	; /
	
	push MOUSE_IRQ
	call [irq_table + MOUSE_IRQ*4]
	add esp, 4

	in	al, INT_S_CTLMASK	; `.
	and	al, ~(1 << (MOUSE_IRQ - 8))	;  | 恢复接受当前中断
	out	INT_S_CTLMASK, al	; /
	
	mov	al, EOI			; `. 置EOI位(master)
	out	INT_M_CTL, al		; /
	nop				; `. 置EOI位(slave)
	out	INT_S_CTL, al		; /  一定注意：slave和master都要置EOI

	jmp intr_exit

global IRQ_primary_channel
IRQ_primary_channel:
	pushad
	push ds
	push es
	push fs
	push gs

	mov dx,ss
	mov ds, dx
	mov es, dx
	mov esp, INTERRUPT_STACK_TOP

	in	al, INT_S_CTLMASK	; `.
	or	al, (1 << (PRIMARY_IRQ - 8))	;  | 屏蔽当前中断
	out	INT_S_CTLMASK, al	; /
	
	push PRIMARY_IRQ
	call [irq_table + PRIMARY_IRQ*4]
	add esp, 4

	in	al, INT_S_CTLMASK	; `.
	and	al, ~(1 << (PRIMARY_IRQ - 8))	;  | 恢复接受当前中断
	out	INT_S_CTLMASK, al	; /
	
	mov	al, EOI			; `. 置EOI位(master)
	out	INT_M_CTL, al		; /
	nop				; `. 置EOI位(slave)
	out	INT_S_CTL, al		; /  一定注意：slave和master都要置EOI
	
	jmp intr_exit	;不知道为什么不加个跳转就出错	

global intrrupt_sys_call

intrrupt_sys_call:
	pushad
	push ds
	push es
	push fs
	push gs
	
	mov edi, edx 
	mov dx,ss
	mov ds, dx
	mov es, dx
	mov edx, edi 
	
	mov esp, INTERRUPT_STACK_TOP
	
	;先把后面的参数压栈, 最多支持5个参数
	push esi
	push ecx
	push edx
	push ebx
	call [sys_call_table + eax*4]
	add esp, 4*4
	
	;保存返回值eax
	mov edx, [task_ready]
	mov [edx + EAXREG], eax

	jmp intr_exit
	
global intr_exit_clock
intr_exit_clock:
	mov esp, [task_ready]
	lea eax, [esp + STACKTOP]
	mov dword [tss + TSS_ESP0], eax 
	
	pop gs
	pop fs
	pop es	 
	pop ds
	popad
	iretd
	
global intr_exit
intr_exit:
	mov esp, [task_ready]
	pop gs
	pop fs
	pop es	 
	pop ds
	popad
	iretd	
