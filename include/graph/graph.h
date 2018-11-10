#ifndef _GRAPH_SERVICE_H
#define _GRAPH_SERVICE_H
#include "kernel/types.h"
#include "graph/color.h"
#include "kernel/ioqueue.h"
#include "kernel/task.h"
#include "graph/layout.h"
#include "graph/view.h"



struct graphc_service
{
	struct ioqueue *ioqueue;
	struct task *task;
	
	int mouse_refresh_couter;
};
extern struct graphc_service graphc_service;

void init_graph();
void graph_service_process();


/*
图形处理
*/
void graph_write_piex(uint32_t x, uint32_t y, struct color *color);
void graph_read_piex(uint32_t x, uint32_t y, struct color *color);

/*
显示
*/
void layout_source_dir(char *path);
void layout_view_control();

void source_new_file();
void source_new_dir();

void source_rm_dir();
void source_rm_file();

/*
图形接口
*/
struct view_control *graph_create_window(char *title, int width, int height);
int graph_window_close();

#endif

