#ifndef _DESKTOP_H
#define	_DESKTOP_H
#include "kernel/types.h"
#include "gui/layer.h"

#define TBB_NR 10


#define TBB_STATUS_ON 2
#define TBB_STATUS_OFF 1
#define TBB_STATUS_FREE 0

#define TASKBAR_UP_HIGH 20
#define TASKBAR_DOWN_HIGH 36

#define TBB_SIZE 36
#define TBB_BLOCK 32



//task bar block struct
struct task_bar_block
{
	int x, y;		//位置
	int wide, high;	//宽高
	int status;		//状态
	int color;
};


#define CLN_BOX_STATUS_IDLE 1
#define CLN_BOX_STATUS_TOUCHED 2

struct collision_box	//碰撞盒子
{
	int status;	//盒子状态	（touching idle）
	int x, y;	//位置
	int width, height;	//宽高
	struct layer *layer;
	int color;
};

#define MENU_STATUS_ON 1
#define MENU_STATUS_OFF 0



#define MENU_SUB_WIDTH (16*8+4)
#define MENU_SUB_HEIGHT (16+4)

#define MENU_SUB_UNUSED 0
#define MENU_SUB_USING 1


struct menu_sub
{
	int status;	//盒子状态	（touching idle）
	int x, y;	//位置
	int width, height;	//宽高
	struct layer *layer;
	int color;
	struct collision_box *box;
	char *title;
};
#define MENU_SUB_MAX 20

struct menu 
{
	int status;	//盒子状态	（touching idle）
	int x, y;	//位置
	int width, height;	//宽高
	struct layer *layer;
	int color;
	struct collision_box *box;
	int menu_sub_counts;	//一共有多少个子菜单
	struct menu_sub *menu_sub_table[MENU_SUB_MAX];	//保存指针
	char *title;
};


extern int desktop_init_finish;

void task_desktop_entry();

//tbb
struct task_bar_block *alloc_tbb();
void tbb_init(struct task_bar_block *tbb, int pos, int status);
void tbb_show(struct task_bar_block *tbb);
void tbb_display();

//box
void collision_box_init(struct collision_box *c_box, int x, int y, int width, int height);
void collision_box_show(struct collision_box *c_box);
void collision_box_bound(struct collision_box *c_box,struct layer *layer);
int collision_box_update(struct collision_box *c_box);
//menu
void menu_init(struct menu *menu, int x, int y, int width, int height, char *title);
void menu_show(struct menu *menu);
void menu_bound(struct menu *menu,struct layer *layer);
void menu_close(struct menu *menu);
//menu_sub
void menu_sub_init(struct menu_sub *menu_sub, int x, int y, int width, int height);
void menu_sub_show(struct menu_sub *menu_sub);
void menu_sub_hide(struct menu_sub *menu_sub);
void menu_sub_bound(struct menu_sub *menu_sub,struct layer *layer);
int menu_sub_update(struct menu_sub *menu_sub);

struct menu_sub *creat_menu_sub(struct menu *menu, char *title);

#define DEFAULT_DESKTOP_COLOR COLOR_TAUPE



#endif
