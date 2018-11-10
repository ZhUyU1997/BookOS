#ifndef _GRAPH_COLOR_H_
#define _GRAPH_COLOR_H_

#include "kernel/types.h"

//把ARGB转换成color
#define ARGB_TO_COLOR(color,a,r,g,b) color.alpha=a;color.red=r;color.green=g;color.blue=b;

struct color
{
	uint8_t blue;	//blue
	uint8_t green;	//green
	uint8_t red;	//red
	uint8_t alpha;	
};

extern struct color color_red;
extern struct color color_blue;
extern struct color color_green;
extern struct color color_white;
extern struct color color_black;
extern struct color color_yellow;
extern struct color color_gray;
extern struct color color_empty;
extern struct color color_global;

void init_color();

void color_init(struct color *color, uint8_t a, uint8_t r, uint8_t g, uint8_t b);
void color_copy(struct color *colora, struct color *colorb);

#endif

