#ifndef _GRAPH_LAYOUT_H_
#define _GRAPH_LAYOUT_H_
#include "kernel/types.h"
#include "graph/color.h"
#include "graph/graph.h"
#include "fs/dir.h"

#define MAX_LAYOUT_NR 5
#define LAYOUT_NAME_LEN 12

struct aera
{
	int x0, x1, y0, y1;
};

struct layout
{
	int id;
	char name[LAYOUT_NAME_LEN];
	//布局区域
	struct aera aera;
};

extern struct color color_layout;


void init_layout();
void layout_init(int id, int x0, int y0, int x1, int y1);
struct aera *layout_get_aera_by_id(int id);
struct aera *layout_get_aera_by_name(char *name);
void layout_reset(char *name);

void layout_reset_off(char *name, int left, int up, int right, int down);

#define MAX_FOLDER_NR 16

#define FOLDER_IDLE  0
#define FOLDER_TOUCH 1
#define FOLDER_CLICK 2

#define FOLDER_WIDE 192
#define FOLDER_HIGH 20

#define FOLDER_UNUSED 0
#define FOLDER_USING 0X01

#define FOLDER_PATH_LEN 128


/*
用于管理系统资源的树状目录
*/
struct folder
{
	int x, y;
	int wide, high;
	
	int status;
	int flags;
	int is_dir;
	int access;
	
	struct dir_entry *de;
	struct color *color;
	struct color *ico_color;
	
	char *path;
};

extern char folder_path[];
extern char operate_path[];
extern char operate_name[];

extern struct folder *folder_checked;

void init_folder();
struct folder *folder_alloc();
void folder_set_pos(struct folder *folder, int x, int y);
int folder_draw(struct folder *folder);
void folder_mouse_even();

void recycle_folders();

int dir_path_to_last(char *path);
void folder_select_operate();
void change_operate_path();
#endif

