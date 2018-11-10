#include "driver/vga.h"
#include "lib/string.h"
#include "kernel/io.h"
#include "lib/stdarg.h"

struct vga VGA;
void init_vga()
{
	VGA.vram = (int8_t *)VGA_VRAM;
	VGA.cursor_pos = 0;
	VGA.color = COLOR_DEFAULT;
	
	sema_init(&VGA.sema, 1);
	
	clean_screen();
	printk("Init vga done!\n");
}

void put_char(uint8_t ch)
{
	VGA.cursor_pos = get_cursor();
	if(ch == 0xd){				//回车CR(0x0d)
		VGA.cursor_pos = line_feed(VGA.cursor_pos);
	}else if(ch == 0xa){		//换行LF(0x0a)
		VGA.cursor_pos = line_feed(VGA.cursor_pos);
	}else if(ch == 0x8){		//退格BS(backspace)
		VGA.cursor_pos = backspace(VGA.cursor_pos);
	}else{						//一般字符
		VGA.vram[VGA.cursor_pos*2] = ch;
		VGA.vram[VGA.cursor_pos*2+1] = VGA.color;
		VGA.cursor_pos++;
	}
	if(VGA.cursor_pos >= VIDEO_WIDTH*25){
		roll_screen();
	}
	set_cursor(VGA.cursor_pos);
}

void put_str(uint8_t *str)
{
	while(*str != 0x00){
		put_char(*str);
		str++;
	}
}

void put_int(uint32_t num)
{
	int8_t str_ptr[16];
	itoa(str_ptr, num);
	put_str(str_ptr);
}

uint16_t get_cursor()
{
	uint16_t pos_low, pos_high;		//设置光标位置的高位的低位
	//取得光标位置
	io_out8(CRTC_ADDR_REG, CURSOR_H);			//光标高位
	pos_high = io_in8(CRTC_DATA_REG);
	io_out8(CRTC_ADDR_REG, CURSOR_L);			//光标低位
	pos_low = io_in8(CRTC_DATA_REG);
	
	return (pos_high<<8 | pos_low);	//返回合成后的值
}

uint16_t set_cursor(uint16_t cursor_pos)
{
	//设置光标位置 0-2000
	io_cli();
	io_out8(CRTC_ADDR_REG, CURSOR_H);			//光标高位
	io_out8(CRTC_DATA_REG, (cursor_pos >> 8) & 0xFF);
	io_out8(CRTC_ADDR_REG, CURSOR_L);			//光标低位
	io_out8(CRTC_DATA_REG, cursor_pos & 0xFF);
	io_sti();
}

void set_video_start_addr(uint16_t addr)
{
	io_cli();
	io_out8(CRTC_ADDR_REG, START_ADDR_H);
	io_out8(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	io_out8(CRTC_ADDR_REG, START_ADDR_L);
	io_out8(CRTC_DATA_REG, addr & 0xFF);
	io_sti();
}

void roll_screen()
{
	
	int x, y;
	for(y = 0; y < 24; y++){
		for(x = 0; x < VIDEO_WIDTH; x++){
			VGA.vram[(y*VIDEO_WIDTH + x)*2] = VGA.vram[((y+1)*VIDEO_WIDTH + x)*2];
			VGA.vram[(y*VIDEO_WIDTH + x)*2 + 1] = VGA.vram[((y+1)*VIDEO_WIDTH + x)*2 + 1];
		}
	}
	for(x = 0; x < VIDEO_WIDTH; x++){
		VGA.vram[(24*VIDEO_WIDTH + x)*2] = ' ';
		VGA.vram[(24*VIDEO_WIDTH + x)*2 + 1] = 0x07;
	}
	VGA.cursor_pos = VIDEO_WIDTH*24;
	//sema_V(&VGA.sema);
}

void clean_screen()
{
	int x, y;
	for(y = 0; y < 24; y++){
		for(x = 0; x < VIDEO_WIDTH; x++){
			VGA.vram[(y*VIDEO_WIDTH + x)*2] = ' ';
			VGA.vram[(y*VIDEO_WIDTH + x)*2 + 1] = 0x07;
		}
	}
	VGA.cursor_pos = 0;
	set_cursor(0);
}

uint16_t line_feed(uint16_t cursor_pos)
{
	//换行
	uint16_t words_line = cursor_pos%VIDEO_WIDTH;
	cursor_pos -= words_line;
	cursor_pos += VIDEO_WIDTH;
	return cursor_pos;
}

uint16_t backspace(uint16_t cursor_pos)
{
	uint16_t old_cursor_pos = cursor_pos;
	
	old_cursor_pos--;
	VGA.vram[old_cursor_pos*2] = ' ';
	VGA.vram[old_cursor_pos*2+1] = 0X07;
	
	return old_cursor_pos;
}

void set_color(uint8_t color)
{
	VGA.color = color;
}

void reset_color()
{
	VGA.color = 0x07;
}

int printk(const char *fmt, ...)
{
	//lock_acquire(&VGA.lock);
	
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	print_buf(buf, i);
	//lock_release(&VGA.lock);
	
	return i;
}

int memory_printf(const char *fmt, ...)
{
	//lock_acquire(&VGA.lock);
	
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	print_buf(buf, i);

	return i;
}


int print_buf(char* buf, int len)
{
	char* p = buf;
	int i = len;
	
	while (i) {
		put_char( *p++);
		i--;
	}
	
	return 0;
}

void vga_putchar(uint8_t ch)
{
	put_char(ch);
}


void vga_clear()
{
	clean_screen();
}
