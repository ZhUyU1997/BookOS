#ifndef _ICON_H
#define	_ICON_H
#include "kernel/types.h"
#include "gui/graphic.h"
#include "gui/layer.h"


struct icon 
{
	int x, y;
	int width, height;
	struct layer *layer;
	char *file;
};
void icon_init(struct icon *icon, uint32_t x, uint32_t y, char *file);
void icon_draw(struct icon *icon);

#endif

