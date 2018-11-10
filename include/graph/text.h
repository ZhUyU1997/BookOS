#ifndef _GRAPH_TEXT_H_
#define	_GRAPH_TEXT_H_

#include "kernel/types.h"
#include "graph/graph.h"
#include "graph/color.h"
#include "graph/surface.h"

#define TEXT_UNUSED 0
#define TEXT_USING 0X01
#define TEXT_ACTIVE 0X10

#define TEXT_IDLE 0
#define TEXT_TOUCH 1
#define TEXT_CLICK 2

#define MAX_TEXT_NR 1

#define TEXT_TEXT_LEN 11
#define TEXT_NAME_LEN 12

struct text {
	struct surface *surface;
	struct aera *aera;				//按钮在哪个范围里面
	uint32_t x, y;		//layer x, y
	uint8_t status;
	uint32_t flags;
	uint32_t width, height;
	int access;
	char txt[TEXT_TEXT_LEN+1];
	char reserved;
	char name[TEXT_NAME_LEN];
	
	struct color *color;
	struct color idle_color, touch_color, click_color;
	struct color *text_color;
	
	int cursor_x, cursor_y;
	int char_counts;
};

extern struct text text_table[];
extern struct text *current_text;

void init_controller_text();
struct text *text_alloc();
int32_t text_free(struct text *text);

void text_init(struct text *text, int width, int height);
void text_set_name(struct text *text, char *name);
void text_set_text(struct text *text, char *txt);
void text_set_pos(struct text *text, int32_t x, int32_t y);
void text_set_size(struct text *text, int32_t width, int32_t height);
void text_set_status(struct text *text, uint8_t status);
int text_set_aera(struct text *text, char *name);

void text_set_flags(struct text *text, uint32_t flags);
void text_clr_flags(struct text *text, uint32_t flags);

void text_draw(struct text *text);

void text_mouse_even();
void text_keyboard_even();

struct text *text_find_by_name(char *name);
void reset_current_text();

#endif //_GRAPH_TEXT_H_

