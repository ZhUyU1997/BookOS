#ifndef _VGA_H
#define _VGA_H
#include "kernel/types.h"
#include "kernel/lock.h"

#define VGA_VRAM 0x800b8000

/* VGA */
#define	CRTC_ADDR_REG	0x3D4	/* CRT Controller Registers - Addr Register */
#define	CRTC_DATA_REG	0x3D5	/* CRT Controller Registers - Data Register */
#define	START_ADDR_H	0xC	/* reg index of video mem start addr (MSB) */
#define	START_ADDR_L	0xD	/* reg index of video mem start addr (LSB) */
#define	CURSOR_H	0xE	/* reg index of cursor position (MSB) */
#define	CURSOR_L	0xF	/* reg index of cursor position (LSB) */
#define	V_MEM_BASE	VGA_VRAM	/* base of color video memory */
#define	V_MEM_SIZE	0x8000	/* 32K: B8000H -> BFFFFH */


#define VIDEO_WIDTH		80
#define VIDEO_HEIGHT		25

#define VIDEO_SIZE		(VIDEO_WIDTH * VIDEO_HEIGHT)
/*
color set

MAKE_COLOR(BLUE, RED)
MAKE_COLOR(BLACK, RED) | BRIGHT
MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH

*/
#define BLACK   0x0     /* 0000 */
#define WHITE   0x7     /* 0111 */
#define RED     0x4     /* 0100 */
#define GREEN   0x2     /* 0010 */
#define BLUE    0x1     /* 0001 */
#define FLASH   0x80    /* 1000 0000 */
#define BRIGHT  0x08    /* 0000 1000 */
#define	MAKE_COLOR(x,y)	((x<<4) | y) /* MAKE_COLOR(Background,Foreground) */

//#define COLOR_DEFAULT	(MAKE_COLOR(BLACK, WHITE))
#define COLOR_DEFAULT	(MAKE_COLOR(BLACK, RED))
#define COLOR_GRAY		(MAKE_COLOR(BLACK, BLACK))


struct vga {
	int8_t *vram;
	uint16_t cursor_pos;
	int8_t color;
	struct semaphore sema;
};

extern struct vga VGA;
void init_vga();
void put_char(uint8_t ch);
void put_str(uint8_t *str);
void put_int(uint32_t num);

uint16_t get_cursor();
uint16_t set_cursor(uint16_t cursor_pos);
void set_video_start_addr(uint16_t addr);


uint16_t line_feed(uint16_t cursor_pos);
uint16_t backspace(uint16_t cursor_pos);
void set_color(uint8_t color);
void reset_color();

void roll_screen();
void clean_screen();
int print_buf(char* buf, int len);
int printk(const char *fmt, ...);
int memory_printf(const char *fmt, ...);

void vga_putchar(uint8_t ch);

void vga_clear();


#endif

