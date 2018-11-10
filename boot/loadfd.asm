KERNEL_SECTOR_OFFSET EQU 9		;kernel is on the 9th sector
KERNEL_SECTORS EQU 348	;now kernel only use 256 sector(256*512/1024 = 128KB)
;0x7c00~0x7e00	boot.bin	512byte 1sector
;0x70000~0x71000	load.bin	1kb 8sector	
;0x71000~0x80000	kernel.bin	60kb 120sector	

;0x111 640*480*16bit 
;0x114 800*600*16bit 
;0x117 1024*768*16bit 
;0x11a 1280*1024*16bit 

;16 bits
MODE_640_480_16 EQU 0x111
MODE_800_600_16 EQU 0x114
MODE_10240_768_16 EQU 0x117
MODE_1280_1024_16 EQU 0x11A

;24 bits
MODE_640_480_24 EQU 0x112
MODE_800_600_24 EQU 0x115
MODE_10240_768_24 EQU 0x118
MODE_1280_1024_24 EQU 0x11B

VBEMODE	EQU	MODE_800_600_24			; 800*600*16bit

;其实是0x600*16+0x0000，因为偏移地址
ARDS_BUF	EQU		0x0004	;ards buf 地址（0x6004）
ARDS_NR		EQU		0x0000	;ards 的数量（0x6000）
;画面信息16字节
VCOLOR	EQU		0			; 颜色数
XWIDTH	EQU		2			; x的分辨率	宽度
YHEIGHT	EQU		4			; y的分辨率	高度
VRAM	EQU		6			; VRAM 地址

	org 0x70000		;load was loaded at 0x10000

	mov ax, cs
	mov ds, ax 
	mov es, ax 
	mov ss, ax 
	mov sp, 0	;ss=0x1000, sp=0
	
	;put 'L'
	mov ax, 0xb800
	mov es, ax 
	mov al, [char]
	mov byte [es:2],al
	mov byte [es:3],0X06
	
;加载内核
	mov ax, 0x1000
	mov si, KERNEL_SECTOR_OFFSET
	mov cx, 128
	call load_file
	
	mov ax, 0x2000
	mov si, KERNEL_SECTOR_OFFSET+128
	mov cx, 128
	call load_file
	
	mov ax, 0x3000
	mov si, KERNEL_SECTOR_OFFSET+256
	mov cx, 128
	call load_file
	
;加载文件
	mov ax, 0x5000
	mov si, 400
	mov cx, 25
	call load_file
	
;检测内存
check_memory:
	xor ebx, ebx 
	mov edx, 0x534d4150
	mov di, ARDS_BUF
	mov ax, 0x600 
	mov es, ax
	mov word [es:ARDS_NR], 0
.e820_mem_get_loop:
	mov eax, 0x0000e820
	mov ecx, 20
	int 0x15
	jc .e820_check_failed
	add di, cx 
	add word [es:ARDS_NR], 1
	cmp ebx, 0
	jnz .e820_mem_get_loop
	jmp init_vbe
.e820_check_failed:
	jmp $

	
init_vbe:

	;检查VBE是否存在
	;缓冲区 0x90000开始
	mov	ax,0x9000	
	mov	es,ax
	mov	di,0
	mov	ax,0x4f00	;检查VBE存在功能，指定ax=0x4f00
	int	0x10
	cmp	ax,0x004f	;ax=0x004f 存在
	jne	screen_default
	
	;检查VBE版本，必须是VBE 2.0及其以上
	mov	ax,[es:di+4]
	cmp	ax,0x0200
	jb	screen_default			; if (ax < 0x0200) goto screen_default

	;获取画面信息， 256字节
	;cx=输入检查的模式
	;[es:di+0x00]	模式属性	bit7是1就能加上0x4000，便于操作
	;[es:di+0x12]	x的分辨率	宽度
	;[es:di+0x14]	y的分辨率	高度
	;[es:di+0x19]	颜色数		8位，16位，24位，32位
	;[es:di+0x1b]	颜色的指定方法 	调色板等
	;[es:di+0x28]	VRAM 地址
	
	mov	cx,VBEMODE	;cx=模式号
	mov	ax,0x4f01	;获取画面模式功能，指定ax=0x4f01
	int	0x10
	cmp	ax,0x004f	;ax=0x004f 指定的这种模式可以使用
	jne	screen_default

	;切换到指定的模式
	mov	BX,VBEMODE+0x4000	;bx=模式号
	mov	ax,0x4f02	;切换模式模式功能，指定ax=0x4f01
	;int	0x10
	
	mov ax, 0x610
	mov ds, ax
	
	xor ax, ax
	mov	al,[es:di+0x19]
	mov	[VCOLOR],ax ;保存颜色位数
	mov	ax,[es:di+0x12]
	mov	[XWIDTH],ax		;保存x分辨率 宽度
	mov	ax,[es:di+0x14]
	mov	[YHEIGHT],ax		;保存y分辨率 高度
	mov	eax,[es:di+0x28]
	mov	[VRAM],eax		;保存VRAM地址
	
	JMP	protect_set

	;切换失败切换到默认的模式
screen_default:
	;jmp $
	mov ax, 0x610
	mov ds, ax
	
	mov		al,0x13			;320*200*8彩色
	mov		ah,0x00
	int		0x10
	mov		BYTE [VCOLOR],8
	mov		WORD [XWIDTH],320
	mov		WORD [YHEIGHT],200
	mov		DWORD [VRAM],0x000a0000

protect_set:
	
	mov ax, cs
	mov ds, ax
	
	;close the interruption
	cli
	;load GDTR
	lgdt	[gdt48]
	lidt	[cs:IDTR]

	;enable A20 line
	in		al,0x92
	or		al,0000_0010B
	out		0x92,al
	;set CR0 bit PE
	mov		eax,cr0
	or		eax,1
	mov		cr0,eax
	
	;far jump:to clean the cs
	jmp		dword 0x08:flush

;1kb 对齐，便于gdt地址得整数	
times 1024-($-$$) db 0	
;Global Descriptor Table,GDT
gdt:
	;0:void
	dd		0x00000000
	dd		0x00000000
	;1:4GB(flat-mode) code segment 0
	dd		0x0000ffff
	dd		0x00cf9A00
	;2:4GB(flat-mode) data segment 0
	dd		0x0000ffff
	dd		0x00cf9200
	
	dd		0x00000000
	dd		0x00000000
	
	dd		0x0000ffff
	dd		0x00cffa00
	;0x00cffa000000ffff
	
	dd		0x0000ffff
	dd		0x00cff200
	;DRIVER CODE
	dd		0x00000000
	dd		0x00000000
	
	dd		0x00000000
	dd		0x00000000
	;DRIVER DATA
	dd		0x00000000
	dd		0x00000000
	
	dd		0x00000000
	dd		0x00000000
	
gdt48:
	.size	dw	(8*8-1)
	.offset	 dd gdt

dd		0x00000000
dd		0x00000000	
ADR_IDT	equ		0x00090000
LIMIT_IDT equ	0x000007ff

IDTR:
	.limit		dw	LIMIT_IDT			;IDT的长度
	.base		dd	ADR_IDT			;IDT的物理地址
		
	
char: db 'L'

;si=LBA address, from 0
;cx=sectors
;es:dx=buffer address	
;this function was borrowed from internet
read_floppy_sector:
	push ax 
	push cx 
	push dx 
	push bx 
	
	mov ax, si 
	xor dx, dx 
	mov bx, 18
	
	div bx 
	inc dx 
	mov cl, dl 
	xor dx, dx 
	mov bx, 2
	
	div bx 
	
	mov dh, dl
	xor dl, dl 
	mov ch, al 
	pop bx 
.rp:
	mov al, 0x01
	mov ah, 0x02 
	int 0x13 
	jc .rp 
	pop dx
	pop cx 
	pop ax
	ret 
;ax = 写入的段偏移
;si = 扇区LBA地址
;cx = 扇区数
load_file:
	mov es, ax
	xor bx, bx 
.loop:
	call read_floppy_sector
	add bx, 512
	inc si 
	loop .loop
	ret	
	
KERNEL_ENTRY EQU 0x10000

PHY_KERNEL_ENTRY EQU 0x10000
VIR_KERNEL_ENTRY EQU 0x80100000

PAGE_DIR_ADDR equ 0x1000
PAGE_TBL_ADDR equ 0x2000

[bits 32]
flush:
	;init all segment registeres
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, stack_top_end
	
	;put 'P'
	mov dword [0xb8000+4], 'P'
	mov dword [0xb8000+5], 0X06
	call step_page
	
	mov dword [0x800b8000+4], 'P'
	mov dword [0x800b8000+5], 0X03
	
	call init_kernel
	
	;jmp inio kernel with protect mode,no interrupt,high memory read and write
	jmp VIR_KERNEL_ENTRY
	
cpy_kernel:
.re_cpy:
	mov al, [esi]
	mov [edi], al
	inc esi 
	inc edi
	loop .re_cpy
	ret	
	
step_page:  
    mov ecx,1024                       ;1024个目录项
    mov ebx,PAGE_DIR_ADDR                 ;页目录的物理地址
    xor esi,esi
.clean_pdt:
    mov dword [ebx+esi],0x00000000  ;页目录表项清零 
    add esi,4
    loop .clean_pdt

    mov edi, PAGE_TBL_ADDR
    mov ebx, PAGE_DIR_ADDR
    mov dword [ebx], PAGE_TBL_ADDR|0x07
    mov dword [ebx+512*4], PAGE_TBL_ADDR|0x07    
    mov dword [ebx+4092], PAGE_DIR_ADDR|0x07
   	
	;低端4M内存直接对应，可以直接访问到
   	mov cx, 1024
    mov esi, 0|0x07
    
.set_pt0:
    mov [edi], esi
    add esi, 0x1000
    add edi,4
    loop .set_pt0
   	
	;jmp .set_cr

;map vram
    mov eax, [0x6100+6]	;ds = 0x7000 phy 0x6108
	shr eax,22
    shl eax,2
	;eax=0xe00
    
    mov edx,(PAGE_TBL_ADDR+0x1000)|0x07
    mov [PAGE_DIR_ADDR+eax], edx
    
    mov edi, PAGE_TBL_ADDR+0x1000	;edi=0x3000
    mov esi, [0x6100+6]	;esi= 0xe0000000
    or esi, 0x07	;esi= 0xe0000007
    
    mov cx, 1024	;map 4M for vram
    
.set_vram:
    mov dword [edi], esi
	add edi, 4
	add esi, 0x1000
	loop .set_vram

.set_cr:
   	mov eax , PAGE_DIR_ADDR
    mov cr3,eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
.f:
    ret

; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
init_kernel:
	xor	esi, esi
	mov	cx, word [PHY_KERNEL_ENTRY + 2Ch]; ┓ ecx <- pELFHdr->e_phnum
	movzx	ecx, cx					;
	mov	esi, [PHY_KERNEL_ENTRY + 1Ch]	; esi <- pELFHdr->e_phoff
	add	esi, PHY_KERNEL_ENTRY		; esi <- OffsetOfKernel + pELFHdr->e_phoff
.begin:
	mov	eax, [esi + 0]
	cmp	eax, 0				; PT_NULL
	jz	.unaction
	push	dword [esi + 010h]		; size	┓
	mov	eax, [esi + 04h]		;	┃
	add	eax, PHY_KERNEL_ENTRY	;	┣ ::memcpy(	(void*)(pPHdr->p_vaddr),
	push	eax				; src	┃		uchCode + pPHdr->p_offset,
	push	dword [esi + 08h]		; dst	┃		pPHdr->p_filesz;
	call	memcpy				;	┃
	add	esp, 12				;	┛
.unaction:
	add	esi, 020h			; esi += pELFHdr->e_phentsize
	dec	ecx
	jnz	.begin
	ret
	
memcpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回		
stack_top:
	times 256 db 0
stack_top_end equ $-stack_top
;fill it with 1kb
times (4096-($-$$)) db 0
