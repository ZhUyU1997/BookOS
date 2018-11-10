#ifndef _WINDOW_H
#define	_WINDOW_H
#include "kernel/types.h"
#include "gui/graphic.h"
#include "gui/layer.h"

#define	WINDOW_COLOR_DEFAULT RGB24(160,160,160)

#define	MAX_WINDOWS 12		//和layer一致

struct window {
	struct layer *layer;
	uint8_t *title;
	uint32_t x, y;
	uint32_t x0, y0;
	uint32_t wide, high;	//活动区域
	uint32_t width, height;	//窗口区域
};

extern struct window *active_window;

void init_windows();
void window_draw(uint8_t *buf,int wide, int high, char *title, char activate);
struct window *create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height,uint8_t *title);
struct window *window_topest();
void draw_close_button(uint8_t *buf,int wide, uint32_t color_button, uint32_t color_bc);
void draw_minimize_button(uint8_t *buf,int wide, uint32_t color_button, uint32_t color_bc);
void draw_screen_button(uint8_t *buf,int wide, uint32_t color_button, uint32_t color_bc);

void window_change(struct layer *layer, char activate);
void window_on(struct layer *layer);
void window_off(struct layer *layer);

void window_switch(struct window *window, char activate);
void window_focus_on(struct window *window);
void window_focus_off(struct window *window);

void window_square(struct window *win, uint32_t x,uint32_t y, uint32_t wide,uint32_t high, uint32_t color);
void window_string(struct window *win, uint32_t x,uint32_t y, uint8_t *s, uint32_t color);

void draw_moving_window(uint8_t *buf, int wide, int high, int32_t bc);

void window_top(struct window *window);
void window_bottom(struct window *window);
#endif
