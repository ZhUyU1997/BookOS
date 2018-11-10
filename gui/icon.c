#include "lib/bmp.h"
#include "fs/fs.h"
#include "gui/gui.h"
#include "gui/icon.h"

void icon_init(struct icon *icon, uint32_t x, uint32_t y, char *file)
{
	icon->x = x;
	icon->y = y+24;	//24是总标题栏
	icon->width = icon->height = 48;
	icon->layer = layer_desktop;
	icon->file = file;
}

void icon_draw(struct icon *icon)
{
	
}
