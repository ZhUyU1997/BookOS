#include "gui/gui.h"
#include "gui/layer.h"
#include "gui/graphic.h"
#include "kernel/memory.h"
#include "driver/vga.h"
#include "driver/video.h"
#include "driver/console.h"
#include "kernel/ards.h"
#include "driver/mouse.h"
#include "gui/desktop.h"
#include "lib/math.h"
#include "lib/bmp.h"
#include "fs/fs.h"
#include "kernel/message.h"
#include "lib/gl.h"
struct layer_manage *layer_manage;

struct layer *layer_desktop, *layer_mouse, *layer_console, *layer_taskbar_up, *layer_taskbar_down;
uint8_t *buf_desktop, *buf_mouse, *buf_console, *buf_taskbar_up, *buf_taskbar_down;

struct layer *layer_pic;
//uint16_t *buf_pic;

int is_open_interface;
void image_cursor(uint8_t *buf, int32_t bc);

void draw_open_interface(struct layer *layer);

void init_gui()
{
	//初始化图形管理
	layer_manage = layer_manage_init((uint8_t *)video_info.vram, video_info.wide, video_info.high);
	
	is_open_interface = 1;
	
	layer_desktop = layer_alloc(LAYER_TYPE_NOTMOVE);
	layer_mouse = layer_alloc(LAYER_TYPE_MOVE);
	//layer_console = layer_alloc(LAYER_TYPE_WINDOW);
	//layer_pic = layer_alloc(LAYER_TYPE_NOTMOVE);
	
	//layer_taskbar_up = layer_alloc(LAYER_TYPE_NOTMOVE);
	//layer_taskbar_down = layer_alloc(LAYER_TYPE_NOTMOVE);
	
	buf_desktop = sys_malloc(video_info.wide * video_info.high * 3);
	buf_mouse = sys_malloc(16*16*3);
	//buf_console = sys_malloc(CONSOLE_WIN_WIDE*CONSOLE_WIN_HIGH*3);
	//buf_pic = sys_malloc(320*240*2);
	
	//buf_taskbar_up = sys_malloc(TASKBAR_UP_HIGH*video_info.wide*2);
	//buf_taskbar_down = sys_malloc(TASKBAR_DOWN_HIGH*video_info.wide*2);
	
	
	layer_init(layer_desktop, buf_desktop, video_info.wide, video_info.high, LAYER_NOINVISIBLE);
	layer_init(layer_mouse, buf_mouse, 16, 16, LAYER_INVISIBLE);
	//layer_init(layer_console, buf_console, CONSOLE_WIN_WIDE, CONSOLE_WIN_HIGH ,LAYER_NOINVISIBLE);
	//初始化图层信息，LAYER_NOINVISIBLE说这个图层没有透明部分 LAYER_INVISIBLE有透明部分
	//layer_init(layer_pic, buf_pic, 320, 240 ,LAYER_NOINVISIBLE);
	
	init_desktop();
	//BC
	//
	
	//draw_block(layer_console->buf,layer_console->wide, layer_console->high, 0);
	//window_draw(layer_console->buf,layer_console->wide, layer_console->high, "Console", 1);
	//在里面绘制内容
	//draw_square16(layer_pic->buf,layer_pic->wide, 0, 0, layer_pic->wide,layer_pic->high, COLOR_RED);
	/*int px = 0;
	for(px = 0; px < 255; px++){
		draw_pix16(buf_desktop,layer_desktop->wide, px, 50, RGB16(px,0,0));
	}*/
	
	image_cursor(layer_mouse->buf, LAYER_INVISIBLE);

	layer_shift(layer_desktop,0,0);
	//layer_shift(layer_console,layer_desktop->wide/2-layer_console->wide/2,layer_desktop->high/2-layer_console->high/2);
	layer_shift(layer_mouse,layer_desktop->wide/2,layer_desktop->high/2);
	//设置在屏幕中的位置
	//layer_shift(layer_pic,200,100);
	
	layer_height(layer_desktop,0);
	//layer_height(layer_taskbar_up,-1);
	//layer_height(layer_taskbar_down,2);
	//layer_height(layer_console,1);
	//图层在多个图层中的高度
	//layer_height(layer_pic,-1);
	layer_height(layer_mouse,1);
	
	/*
	uint32_t *a = sys_malloc(16);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 0, COLOR_WHITE, (uint32_t )a);
	sys_mfree(a);
	uint32_t *b = sys_malloc(16);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*1, COLOR_WHITE, (uint32_t )b);
	uint32_t *c = sys_malloc(32);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*2, COLOR_WHITE, (uint32_t )c);
	
	uint32_t *d = sys_malloc(64);
	sys_mfree(c);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*3, COLOR_WHITE, (uint32_t )d);
	
	uint32_t *e = sys_malloc(64);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*4, COLOR_WHITE, (uint32_t )e);
	sys_mfree(e);
	uint32_t *f = sys_malloc(64);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*5, COLOR_WHITE, (uint32_t )f);
	
	
	uint32_t *g = sys_malloc(128);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*6, COLOR_WHITE, (uint32_t )g);
	sys_mfree(g);
	uint32_t *h = sys_malloc(128);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*7, COLOR_WHITE, (uint32_t )h);
	uint32_t *i = sys_malloc(256);
	uint32_t *j = sys_malloc(256);
	uint32_t *k = sys_malloc(512);
	uint32_t *o = sys_malloc(1024);
	
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*8, COLOR_WHITE, (uint32_t )i);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*9, COLOR_WHITE,(uint32_t )j);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*10, COLOR_WHITE, (uint32_t )k);
	draw_hex16(layer_desktop->buf,layer_desktop->wide, 0, 16*11, COLOR_WHITE, (uint32_t )o);
	
	*/
	
	//
	
	//draw_open_interface(layer_desktop);
	/*draw_square16(layer_desktop->buf,layer_desktop->wide, 0, 0, layer_desktop->wide,layer_desktop->high, COLOR_RED);
	layer_refresh(layer_desktop, 0, 0, layer_desktop->wide, layer_desktop->high);*/
	//layer_refresh(layer_console, 0, 0, layer_console->wide, layer_console->high);
	//layer_refresh(layer_taskbar_up, 0, 0, layer_taskbar_up->wide, layer_taskbar_up->high);
	//layer_refresh(layer_taskbar_down, 0, 0, layer_taskbar_down->wide, layer_taskbar_down->high);
	//刷新图层
	//layer_refresh(layer_pic, 0, 0, layer_pic->wide, layer_pic->high);
	layer_refresh(layer_mouse, 0, 0, layer_mouse->wide, layer_mouse->high);
	
	init_windows();
	
	draw_square24(layer_desktop->buf,layer_desktop->wide, 0, 0, layer_desktop->wide,layer_desktop->high, DEFAULT_DESKTOP_COLOR);
	layer_refresh(layer_desktop, 0, 0, layer_desktop->wide, layer_desktop->high);
	//把窗口显示
	
	//sys_create_window(100,36,320,240,"File Manager");
	
	//测试
	/*
	layer_integer(layer_desktop, 0,100, 16, 10, COLOR_GREEN);
	*/
	//gprintk("###%x ",100);
	/*void *a,*b,*c,*d,*e;
	a = kmalloc(32);
	//kfree(a);
	b = kmalloc(32);
	c = kmalloc(64);
	//kfree(c);
	d = kmalloc(1024);
	d = kmalloc(1024);
	e = kmalloc(512);
	
	draw_hex16(layer_desktop->buf, layer_desktop->wide, 0,30,COLOR_GREEN, a);
	draw_hex16(layer_desktop->buf, layer_desktop->wide, 0,30+16,COLOR_GREEN, b);
	draw_hex16(layer_desktop->buf, layer_desktop->wide, 0,30+16*2,COLOR_GREEN, c);
	draw_hex16(layer_desktop->buf, layer_desktop->wide, 0,30+16*3,COLOR_GREEN, d);
	draw_hex16(layer_desktop->buf, layer_desktop->wide, 0,30+16*4,COLOR_GREEN, e);
	layer_refresh(layer_desktop, 0, 30, 8*10, 100+16*5);
	
	*/
	
	/*char *app = (char *)0x80060000;
	
	int t;
	for(t = 0; t < 32; t++){
		//layer_integer(layer_desktop, 0,32, 16, app[t], COLOR_BLACK);
		gprintk("%x ",app[t]);
	}*/
	
	
	//draw_3d();
	/**内核加载
	int i;
	char *buf = (char *)0x80100000;
	gprintk("######");
	for(i = 0; i < 32; i++){
		gprintk("%x ", buf[i]);
	}
	buf = (char *)0x80010000;
	gprintk("###");
	
	for(i = 0; i < 32; i++){
		gprintk("%x ", buf[i]);
	}
	*/
	//显示一个ico
	
	
	
}

void draw_open_interface(struct layer *layer)
{
	/*char *edition = "Dragon OS v0.0.05";
	char *company = "Dragon Coporation";
	char *starting = "Starting Dragon...";
	char *power = "Powered By Dragon";
	
	//底层
	draw_square16(layer->buf,layer->wide, 0, 0, layer->wide,layer->high, COLOR_BLACK);
	
	draw_string16(layer->buf,layer->wide, 8, 8, COLOR_WHITE, edition);
	
	draw_string16(layer->buf,layer->wide, layer->wide/2 - 18/2*8, layer->high/2, COLOR_WHITE, starting);
	draw_square16(layer->buf,layer->wide, layer->wide/2-70, layer->high -128-16, 140,10, RGB16(200,200,200));
	
	draw_string16(layer->buf,layer->wide, layer->wide/2 - 18/2*8, layer->high -128, COLOR_WHITE, company);
	draw_string16(layer->buf,layer->wide, layer->wide/2 - 18/2*8, layer->high -128 + 16+4, RGB16(180,180,180), power);
	
	
	draw_square16(layer->buf,layer->wide, layer->wide/2 - 32,layer->high/2 -32 - 48, 64,64, COLOR_RED);
	draw_square16(layer->buf,layer->wide, layer->wide/2 - 32,layer->high/2 -32 - 48, 48,48, RGB16(180,0,0));
	draw_square16(layer->buf,layer->wide, layer->wide/2 - 32,layer->high/2 -32 - 48, 32,32, RGB16(120,0,0));
	draw_square16(layer->buf,layer->wide, layer->wide/2 - 32,layer->high/2 -32 - 48, 16,16, RGB16(80,0,0));
	
	layer_refresh(layer, 0, 0, layer->wide, layer->high);*/
}

static char cursor[16][16] = {
		{1,0,0,0,0,0,0,0,0,0,0,0},
		{1,1,0,0,0,0,0,0,0,0,0,0},
		{1,2,1,0,0,0,0,0,0,0,0,0},
		{1,2,2,1,0,0,0,0,0,0,0,0},
		{1,2,2,2,1,0,0,0,0,0,0,0},
		{1,2,2,2,2,1,0,0,0,0,0,0},
		{1,2,2,2,2,2,1,0,0,0,0,0},
		{1,2,2,2,2,2,2,1,0,0,0,0},
		{1,2,2,2,2,2,2,2,1,0,0,0},
		{1,2,2,2,2,2,2,2,2,1,0,0},
		{1,2,2,2,2,1,1,1,1,1,1,0},
		{1,2,2,2,1,0,0,0,0,0,0,0},
		{1,2,2,1,0,0,0,0,0,0,0,0},
		{1,2,1,0,0,0,0,0,0,0,0,0},
		{1,1,0,0,0,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0,0,0,0,0},
};
	
void image_cursor(uint8_t *buf, int32_t bc)
{
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == 1) {
				//buf[(y * 16 + x)*3] = COLOR_BLACK;
				draw_pix24(buf,16, x, y, COLOR_BLACK);
			}
			if (cursor[y][x] == 2) {
				//buf[(y * 16 + x)*3] = COLOR_WHITE;
				draw_pix24(buf,16, x, y, COLOR_WHITE);
			}
			if (cursor[y][x] == 0) {
				draw_pix24(buf,16, x, y, bc);
				//buf[(y * 16 + x)*3] = bc;
			}
		}
	}
}
