#ifndef _GRAPH_VIEW_H_
#define	_GRAPH_VIEW_H_

#include "kernel/types.h"
#include "graph/graph.h"
#include "graph/color.h"
#include "kernel/task.h"

#define VIEW_NAME_LEN 128

#define MAX_VIEW_NR 32

#define VIEW_CTL_WIDTH 36
#define VIEW_CTL_HEIGHT 36

#define VIEW_CTL_RANGE 48

#define VIEW_CTL_UNUSED 0
#define VIEW_CTL_USING 1
#define VIEW_CTL_TOUCH 2
#define VIEW_CTL_ACTIVE 3

#define VIEW_UNUSED 0
#define VIEW_USING 1

#define VIEW_ACTIVE 2

struct view 
{
	int x, y;	//视图位置
	int width, height;
	char *buffer;	//视图自己的缓冲区
	char *title;	//视图自己的缓冲区
	int status;
};

struct view_control
{
	struct view *view;
	struct task *task;
	int x, y;	//视图位置
	int width, height;
	int status;
	int access;
	struct color *color;
	
	//char name[VIEW_NAME_LEN];	//视图自己的缓冲区
	
};

extern struct view_control *current_view_control;
extern int view_control_counts;



void init_views();

struct view *view_alloc();

struct view_control *view_control_alloc();
void view_control_init(struct view_control *vc, int vc_x, int vc_y, int vwidth, int vheight);

void view_control_draw(struct view_control *vc);

void view_control_mouse_even();
void view_display(struct view_control *vc);
void view_set_title(struct view *view, char *name);

void view_title_display(struct view *view);

void view_read_piex(struct view *view, uint32_t x, uint32_t y, struct color *color);
void view_write_piex(struct view *view, uint32_t x, uint32_t y, struct color *color);

void view_draw_point(struct view *view, int32_t x, int32_t y, struct color *color);
void view_draw_rectangle(struct view *view, int32_t x, int32_t y, uint32_t wide, uint32_t high,struct color *color);
void view_draw_line(struct view *view, int x0, int y0, int x1, int y1, struct color *color);
void view_draw_circle(struct view *view, uint32_t center_x,uint32_t center_y, uint32_t radius,struct color *color) ;
void view_draw_word(struct view *view, uint32_t x,uint32_t y, char ch,struct color *color);
void view_draw_string(struct view *view, uint32_t x,uint32_t y, uint8_t *s, struct color *color);
void view_draw_integer16(struct view *view, uint32_t x,uint32_t y, uint32_t value, struct color *color);
void view_draw_integer10(struct view *view, uint32_t x,uint32_t y, int32_t value, struct color *color);
void view_draw_word_bits(struct view *view, uint32_t x, uint32_t y , uint8_t *ascii, struct color *color);

void view_refresh_pos(struct view *view, int x0, int y0, int x1, int y1);
void view_refresh_aera(struct view *view, int x0, int y0, int x1, int y1);

void view_control_clean_screen(struct view_control *vc, int title, int view, int task);

#endif //_GRAPH_VIEW_H_

