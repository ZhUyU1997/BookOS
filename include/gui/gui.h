#ifndef _GUI_H
#define	_GUI_H
#include "kernel/types.h"
#include "gui/graphic.h"
#include "gui/layer.h"

void init_gui();
extern struct layer *layer_desktop, *layer_mouse, *layer_win, *layer_console;
extern struct layer *layer_taskbar_up;

struct layer *creat_layer(int type, int x, int y, int wide, int high);
extern int is_open_interface;
int sys_gprint(char* buf, int len);
void task_gui_entry();

#endif
