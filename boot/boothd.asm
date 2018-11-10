;boot.asm

;we use LBA address for floppy
LOAD_SECTOR_OFFSET EQU 1		;load is on the 1st sector
LOAD_SECTORS EQU 8	;this time we use 8 sectors for load.it's 1kb(4096 bytes)
;0x7c00~0x7e00	boot.bin	512byte 1sector
;0x70000~0x71000	load.bin	1kb 8sector	
;0x71000~0x80000	kernel.bin	60kb 120sector	

	org 0x7c00
	[bits 16]
	
	jmp entry
	

SECT_BUF_SIZE		equ	LOAD_SECTORS * 512

LOADER_SEG equ 0x7000

disk_address_packet:
	db	0x10		; [ 0] Packet size in bytes.
	db	0		; [ 1] Reserved, must be 0.
	db	LOAD_SECTORS	; [ 2] Nr of blocks to transfer.
	db	0		; [ 3] Reserved, must be 0.
	dw	0		; [ 4] Addr of transfer - Offset
	dw	LOADER_SEG	; [ 6] buffer.          - Seg
	dd	0		; [ 8] LBA. Low  32-bits.
	dd	0		; [12] LBA. High 32-bits.

entry:
	mov ax, cs 
	mov ds, ax 
	mov es, ax 
	xor ax, ax
	mov ss, ax
	mov sp, 0x7c00
	
	;clean screan
	mov ax, 0x02
	int 0x10
	
	;put 'B'
	mov ax, 0xb800
	mov es, ax 
	mov al, [char]
	mov byte [es:0],al
	mov byte [es:1],0X04
	
	mov	dword [disk_address_packet +  8], LOAD_SECTOR_OFFSET
	call	read_sector
	
	;jmp $

	;cs=0x1000 ip=0
	;now we can use above 1 sector
	jmp 0x7000:0
	
;----------------------------------------------------------------------------
; read_sector
;----------------------------------------------------------------------------
; Entry:
;     - fields disk_address_packet should have been filled
;       before invoking the routine
; Exit:
;     - es:bx -> data read
; registers changed:
;     - eax, ebx, dl, si, es
read_sector:
	xor	ebx, ebx

	mov	ah, 0x42
	mov	dl, 0x80
	mov	si, disk_address_packet
	int	0x13

	mov	ax, [disk_address_packet + 6]
	mov	es, ax
	mov	bx, [disk_address_packet + 4]

	ret
	
	
;char B
char: db 'B'

;make boot to be 512 bytes
times 510-($-$$) db 0
dw 	0xaa55
