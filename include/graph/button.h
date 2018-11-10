#ifndef _GRAPH_BUTTON_H_
#define	_GRAPH_BUTTON_H_

#include "kernel/types.h"
#include "graph/graph.h"
#include "graph/color.h"
#include "graph/surface.h"

#define BUTTON_UNUSED 0
#define BUTTON_USING 0X01
#define BUTTON_ACTIVE 0X02

#define BUTTON_IDLE 0
#define BUTTON_TOUCH 1
#define BUTTON_CLICK 2

#define MAX_BUTTON_NR 32

#define BUTTON_TEXT_LEN 16
#define BUTTON_NAME_LEN 12

struct button {
	struct surface *surface;
	struct aera *aera;				//按钮在哪个范围里面
	uint32_t x, y;		//layer x, y
	uint8_t status;
	uint32_t flags;
	uint32_t width, height;
	int access;
	
	
	char text[BUTTON_TEXT_LEN];
	char name[BUTTON_NAME_LEN];
	
	struct color *color;
	struct color idle_color, touch_color, click_color;
	struct color *text_color;
	void (*function)();
};

extern struct button button_table[];

void init_controller_button();
struct button *button_alloc();
int32_t button_free(struct button *button);

void button_init(struct button *button, int width, int height);
void button_set_name(struct button *button, char *name);
void button_set_text(struct button *button, char *text);
void button_set_pos(struct button *button, int32_t x, int32_t y);
void button_set_size(struct button *button, int32_t width, int32_t height);
void button_set_status(struct button *button, uint8_t status);
int button_set_aera(struct button *button, char *name);

void button_set_flags(struct button *button, uint32_t flags);
void button_clr_flags(struct button *button, uint32_t flags);

void button_draw(struct button *button);

void button_mouse_even();

void button_set_recall(struct button *button, void (*function)());
struct button *button_find_by_name(char *name);
void button_function_t();
void button_function_t2();

#endif

