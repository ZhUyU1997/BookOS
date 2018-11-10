#include "gui/graphic.h"
#include "kernel/types.h"
#include "gui/font.h"
#include "gui/layer.h"
#include "kernel/memory.h"
#include "driver/vga.h"
#include "driver/video.h"
#include "gui/window.h"

uint8_t draw_num_buf[32];
uint8_t draw_num_ptr = 0;
void draw_num(uint32_t value, uint32_t base) ;
void graphic_test()
{
	char *buf = video_info.vram;
	
	draw_pix24(buf,video_info.wide, 10,10, RGB24(255,255,255));
	draw_square24(buf,video_info.wide, 11, 10, 100,50, RGB24(255,0,0));
	draw_square24(buf,video_info.wide, 10, 11, 100,50, RGB24(255,0,0));
	draw_string24(buf,video_info.wide, 5,100,RGB24(0,255,0), "Hello 24 bit mode!");
	
	draw_hex24(buf,video_info.wide,5,120,RGB24(0,0,255), 0x1357acef);
	
	draw_line24(buf,video_info.wide, 100, 5, 200, 55, RGB24(255,0,255));
	draw_circle24(buf,video_info.wide, 200,200, 50,RGB24(255,255,0));
	
}

void draw_pix24(uint8_t *vram,uint32_t wide, uint32_t x, uint32_t y, uint32_t color)
{
	uint8_t *buf = (uint8_t *)(vram + ((y * wide) + x)*3);
	/*判定是否在边界内，如果是就可以直接写数据*/
	if(x >= 0 && x < video_info.wide && y >= 0 && y < video_info.high){
		*buf++ = color&0xff;
		*buf++ = (color>> 8)&0xff;
		*buf = (color>> 16)&0xff;
	}
}

uint32_t get_pix24(uint8_t *vram,uint32_t wide, uint32_t x, uint32_t y)
{
	uint8_t r = 0,g = 0,b = 0;
	uint8_t *buf = (uint8_t *)(vram + ((y * wide) + x)*3);
	/*判定是否在边界内，如果是就可以直接写数据*/
	//if(x >= 0 && x < video_info.wide && y >= 0 && y < video_info.high){
		b = *buf++;
		g = *buf++;
		r = *buf;
	//}
	return RGB24(r,g,b);
}


void draw_square24(uint8_t *vram,uint32_t wide, uint32_t x, uint32_t y, uint32_t w,uint32_t h, uint32_t color)
{
	/*wide-video wide*/
	uint32_t x1, y1;
	for (y1 = 0; y1 < h; y1++) {
		for (x1 = 0; x1 < w; x1++) {
			draw_pix24(vram, wide, x + x1,y + y1, color);
		}
	}
}


void draw_word24(uint8_t *vram, uint32_t wide, uint32_t x, uint32_t y , uint32_t color, uint8_t *ascii)
{
	int i;
	char d /* data */;
	for (i = 0; i < 16; i++) {
		d = ascii[i];
		if ((d & 0x80) != 0) { draw_pix24(vram, wide, x + 0, y + i, color); }
		if ((d & 0x40) != 0) { draw_pix24(vram, wide, x + 1, y + i, color); }
		if ((d & 0x20) != 0) { draw_pix24(vram, wide, x + 2, y + i, color); }
		if ((d & 0x10) != 0) { draw_pix24(vram, wide, x + 3, y + i, color); }
		if ((d & 0x08) != 0) { draw_pix24(vram, wide, x + 4, y + i, color); }
		if ((d & 0x04) != 0) { draw_pix24(vram, wide, x + 5, y + i, color); }
		if ((d & 0x02) != 0) { draw_pix24(vram, wide, x + 6, y + i, color); }
		if ((d & 0x01) != 0) { draw_pix24(vram, wide, x + 7, y + i, color); }
	}
	return;
}


void draw_a_word24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y,uint32_t color, uint8_t *s)
{
	draw_word24(vram, wide, x, y, color, standard_font + *s * 16);
	x += 8;
	return;
}


void draw_string24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y, uint32_t color, uint8_t *s)
{
	for (; *s != 0x00; s++) {
		draw_word24(vram, wide, x, y, color, standard_font + *s * 16);
		x += 8;
	}
	return;
}

void draw_hex24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y,uint32_t color, uint32_t value) 
{
	char ch[8];
	int i = 8;
	/**
	*char *s = "0123456789abcdef";
	*ch[i] = s[(value>>(i*4))&0xf];
	*/
	while (i-- > 0) {
		ch[i] = "0123456789abcdef"[(value>>(i*4))&0xf];
	}
	/*
	draw_string16(vram, wide, x,y,color, "0x");	
	x += 8*2;
	*/
	//循环显示
	for(i = 7; i >= 0; i--){
		draw_a_word24(vram, wide, x,y,color, &ch[i]);	
		x += 8;
	}
}

void draw_num(uint32_t value, uint32_t base) 
{
	uint32_t n = value / base;
	int r = value % base;
	if (r < 0) {
		r += base;
		--n;
	}
	if (value >= base)
		draw_num(n, base);
	draw_num_buf[draw_num_ptr++] = "0123456789"[r];
}

void draw_int24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y,uint32_t color, uint32_t value) 
{
	int i;
	draw_num_ptr = 0;
	
	/*清空缓冲区*/
	for(i = 0; i < 32; i++){
		draw_num_buf[i] = 0;
	}
	
	draw_num(value, 10);
	for(i = 0; i <=7; i++){
		draw_a_word24(vram, wide, x,y,color, &draw_num_buf[i]);	//e
		x += 8;
	}
	draw_num_ptr = 0;
}

int draw_circle24(uint8_t *vram,int wide, uint32_t center_x,uint32_t center_y, uint32_t radius,uint32_t color)  
{
    int x, y, p;  
    x = 0, y = radius, p = 1-radius;  

    while (x < y)
    {
        draw_pix24(vram, wide, center_x + x, center_y + y, color);  
        draw_pix24(vram, wide, center_x - x, center_y + y, color);  
        draw_pix24(vram, wide, center_x - x, center_y - y, color);  
        draw_pix24(vram, wide, center_x + x, center_y - y, color);  
        draw_pix24(vram, wide, center_x + y, center_y + x, color);  
        draw_pix24(vram, wide, center_x - y, center_y + x, color);  
        draw_pix24(vram, wide, center_x - y, center_y - x, color);  
        draw_pix24(vram, wide, center_x + y, center_y - x, color);  
  
        x++;
        if (p < 0) p += 2*x + 1;  
        else  
        {  
            y--;  
            p += 2*x - 2*y + 1;  
        }  
    }
    return 1;  
}

void draw_line24(uint8_t *buf,int wide, int x0, int y0, int x1, int y1, uint32_t color)
{
	int i, x, y, len, dx, dy;
	dx = x1 - x0;
	dy = y1 - y0;
	
	x = x0 << 10;
	y = y0 << 10;
	
	if(dx < 0){
		dx = -dx;
	}
	if(dy < 0){
		dy = -dy;
	}
	if(dx >= dy ){
		len = dx + 1;
		if(x0 > x1){
			dx = -1024;
		} else {
			dx = 1024;
			
		}
		if(y0 <= y1){
			dy = ((y1 - y0 + 1) << 10)/len;
		} else {
			dy = ((y1 - y0 - 1) << 10)/len;
		}
		
		
	}else{
		len = dy + 1;
		if(y0 > y1){
			dy = -1024;
		} else {
			dy = 1024;
			
		}
		if(x0 <= x1){
			dx = ((x1 - x0 + 1) << 10)/len;
		} else {
			dx = ((x1 - x0 - 1) << 10)/len;
		}	
	}
	for(i = 0; i < len; i++){
		//buf[((y >> 10)*wide + (x >> 10))*3] = color;
		draw_pix24(buf,wide, (x >> 10), (y >> 10), color);
		
		x += dx;
		y += dy;
	}
}

