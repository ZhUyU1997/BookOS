#ifndef _GRAPHIC_H
#define	_GRAPHIC_H
#include "kernel/types.h"

#define RGB24(r,g,b) (((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff))

#define COLOR_WHITE RGB24(0xff,0xff,0xff)
#define COLOR_BLACK RGB24(0x00, 0x00, 0x00)
#define COLOR_RED RGB24(200, 10, 10)
#define COLOR_YELLOW RGB24(0xff, 0xff, 0x00)
#define COLOR_GREEN RGB24(0x00, 0xff, 0x00)
#define COLOR_BLUE RGB24(0x00, 0x00, 0xff)
#define COLOR_BLUE RGB24(0x00, 0x00, 0xff)
#define COLOR_PURPLE RGB24(128, 0x00, 0xff)
#define COLOR_TAUPE RGB24(0x80, 0x80, 0x80)
#define COLOR_GREY RGB24(0xc0, 0xc0, 0xc0)
#define COLOR_GREY170 RGB24(170, 170, 170)

#define COLOR_WHITE150 RGB24(150, 150, 150)
#define COLOR_WHITE200 RGB24(200, 200, 200)
#define COLOR_WHITE230 RGB24(230, 230, 230)
#define COLOR_BLACK30 RGB24(30, 30, 30)

uint32_t get_pix24(uint8_t *vram,uint32_t wide, uint32_t x, uint32_t y);
void draw_pix24(uint8_t *vram,uint32_t wide, uint32_t x, uint32_t y, uint32_t color);
void draw_square24(uint8_t *vram,uint32_t wide, uint32_t x, uint32_t y, uint32_t w,uint32_t h, uint32_t color);
void draw_word24(uint8_t *vram, uint32_t wide, uint32_t x, uint32_t y , uint32_t color, uint8_t *ascii);
void draw_a_word24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y,uint32_t color, uint8_t *s);
void draw_string24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y, uint32_t color, uint8_t *s);
void draw_hex24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y,uint32_t color, uint32_t value);

void draw_int24(uint8_t *vram, uint32_t wide, uint32_t x,uint32_t y,uint32_t color, uint32_t value);
void draw_line24(uint8_t *buf,int wide, int x0, int y0, int x1, int y1, uint32_t color);
int draw_circle24(uint8_t *vram,int wide, uint32_t center_x,uint32_t center_y, uint32_t radius,uint32_t color);

void graphic_test();

#endif
