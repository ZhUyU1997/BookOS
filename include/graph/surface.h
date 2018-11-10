#ifndef _GRAPH_SURFACE_H_
#define _GRAPH_SURFACE_H_
#include "kernel/types.h"
#include "graph/color.h"
#include "graph/layout.h"
#include "graph/view.h"

#define MAX_SURFACE_NR 32

#define SURFACE_UNUSED 			0x00
#define SURFACE_USING 			0x01
#define SURFACE_WINDOW 			0x02
#define SURFACE_ACTIVE 			0x04
#define SURFACE_MOVING 			0x08
#define SURFACE_LAYOUT 			0x10

#define SURFACE_NAME_LEN 128

#define WINDOW_TITLE_BAR_LEN 30
#define WINDOW_STYLE 0

struct surface 
{
	char name[SURFACE_NAME_LEN];	//表面的名字
	char *buffer;	//图层对应的缓冲
	int x, y;	//图层的位置
	int wide, high;	//图层的高度
	int linelength;
	uint32_t flags;

	int style;
	int zorder;	//Z轴排序

	struct surface *next;	
};

extern struct surface *surface_desktop;
extern struct surface *surface_mouse;
extern struct surface *surface_window;

/*
表面操作
*/
struct surface *alloc_surface();
void surface_init(struct surface *surface, int wide, int high);

void surface_refresh_zorder_aera(int vx, int vy, int wide, int high);
void surface_refresh_zorder_pos(int vx0, int vy0, int vx1, int vy1);

void surface_set_position(struct surface *surface, int x, int y);
void surface_set_name(struct surface *surface, char *name);
void surface_set_window(struct surface *surface);

void surface_set_bottom(struct surface *surface);
void surface_set_topest(struct surface *surface);

int surface_get_above();

struct surface *surface_find_zorder(int zorder);

/*
draw
*/
void surface_draw_desktop(struct surface *surface);
void surface_draw_window(struct surface *surface);
void surface_draw_mouse(struct surface *surface);

void surface_draw_view(struct surface *surface, struct view *view);


void surface_zorder_alloc(struct surface *surface);
void surface_zorder_free(struct surface *surface);
void surface_zorder_trans(struct surface *surfacea, struct surface *surfaceb);

/*

刷新
*/
void surface_refresh_pos(struct surface *surface, int x0, int y0, int x1, int y1);
void surface_refresh_aera(struct surface *surface, int x, int y, int wide, int high);

/*
图形
*/
void surface_draw_point(struct surface *surface, int32_t x, int32_t y, struct color *color);
void surface_draw_rectangle(struct surface *surface, int32_t x, int32_t y, uint32_t wide, uint32_t high,struct color *color);
void surface_draw_line(struct surface *surface, int x0, int y0, int x1, int y1, struct color *color);
void surface_draw_circle(struct surface *surface, uint32_t center_x,uint32_t center_y, uint32_t radius,struct color *color);
void surface_draw_word(struct surface *surface, uint32_t x,uint32_t y, char ch,struct color *color);
void surface_draw_string(struct surface *surface, uint32_t x,uint32_t y, uint8_t *s, struct color *color);
void surface_draw_integer16(struct surface *surface, uint32_t x,uint32_t y, uint32_t value, struct color *color);
void surface_draw_integer10(struct surface *surface, uint32_t x,uint32_t y, int32_t value, struct color *color);

void surface_draw_word_bits(struct surface *surface, uint32_t x, uint32_t y , uint8_t *ascii, struct color *color);

void surface_write_buf(struct surface *surface, int32_t x0, int32_t y0, uint8_t *buf,int32_t width, int32_t height);

#endif

