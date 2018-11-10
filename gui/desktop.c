#include "gui/gui.h"
#include "gui/layer.h"
#include "gui/graphic.h"
#include "kernel/memory.h"
#include "driver/vga.h"
#include "driver/video.h"
#include "driver/console.h"
#include "kernel/ards.h"
#include "kernel/task.h"
#include "gui/desktop.h"
#include "gui/window.h"
#include "driver/mouse.h"
#include "gui/button.h"
#include "gui/icon.h"
#include "kernel/debug.h"
#include "kernel/system.h"

#define BAR_RIGHT_LEFT 24
#define BAR_CHAR_WIDE 8

struct task_bar_block tbb_table[TBB_NR];

struct task_bar_block *tbb_desktop, *tbb_console;
extern struct layer *layer_taskbar_down,*layer_desktop;
extern struct window *win0;
struct layer *active_layer;
struct task *task_console;
struct window *active_window;
struct fifo32 desktop_fifo;

int desktop_buf[64];

extern struct task *task_terminal;
extern struct icon icon_console;
extern struct windw_manage *windw_manage;

void init_desktop()
{
	int i;
	fifo32_init(&desktop_fifo, 64, desktop_buf);
	for(i = 0; i < TBB_NR; i++){
		tbb_table[i].status = TBB_STATUS_FREE;
	}
}

struct task_bar_block *alloc_tbb()
{
	int i;
	for(i = 0; i < TBB_NR; i++){
		if(tbb_table[i].status == TBB_STATUS_FREE){
			tbb_table[i].status = TBB_STATUS_OFF;
			return &tbb_table[i];
		}
	}
	return NULL;
}

void tbb_display()
{
	int i;
	for(i = 0; i < TBB_NR; i++){
		if(tbb_table[i].status != TBB_STATUS_FREE){
			tbb_show(&tbb_table[i]);
		}
	}
}
extern struct button *btn0;
extern int refresh_bar;
struct icon icon_console;
extern uint8_t *buf_desktop;
void task_desktop_entry()
{
	int i, j, l, x, y;
	struct task *task = task_current();
	
	struct window *window = NULL;	//活动中的图层

	struct button *button;

	int clock_x, clock_y;
	clock_x = layer_desktop->wide - BAR_RIGHT_LEFT;
	clock_y = 3;
	int font_color = COLOR_WHITE230;
	int bc_color = COLOR_BLACK30;
	//tbb_console = alloc_tbb();
	//tbb_init(tbb_console,1, TBB_STATUS_OFF);
	/*draw_square16(layer_desktop ,0,100,100,100, 0);
	
	layer_refresh(layer_desktop, 0,100, 100, 200);
	*/
	//fs_main();

	init_button();
	//init_windows();
	
	//icon_init(&icon_console, 32, 32, "/user/console.bmp");
	//icon_draw(&icon_console);
	
	//task_run(task_terminal, 1);
	
	//layer_image_bmp(layer_desktop, 0,0, "/ico");
	//layer_image_bmp(win0->layer, 0,win0->y0, "/win");
	
	//printf("Driver of keyboard is running.\n");
	//desktop_init_finish = 1;
	int counter = 0;
	refresh_bar = 0;
	/*
	struct layer *layer_opening;
	uint16_t *buf;
	
	layer_opening = layer_alloc(LAYER_TYPE_NOTMOVE);
	buf = sys_malloc(video_info.wide * video_info.high * 2);
	
	layer_init(layer_opening, buf, video_info.wide, video_info.high, LAYER_NOINVISIBLE);
	
	layer_shift(layer_opening,0,0);
	
	layer_height(layer_opening,layer_manage->top);
	
	layer_refresh(layer_opening, 0, 0, layer_opening->wide, layer_opening->high);
	*/
	/*
	uint16_t *buf_taskbar;
	
	buf_taskbar = sys_malloc(video_info.wide * video_info.high * 2);
	*/
	
	struct menu main_menu;
	
	menu_bound(&main_menu,layer_desktop);
	menu_init(&main_menu, 1, 1, 32+2, 16+2, "menu");
	
	menu_show(&main_menu);
	
	for(i = 0; i < MENU_SUB_MAX; i++){
		main_menu.menu_sub_table[i] = NULL;
	}
	creat_menu_sub(&main_menu, "console");
	creat_menu_sub(&main_menu, "test");
	creat_menu_sub(&main_menu, "gl");
	creat_menu_sub(&main_menu, "cpuid");
	creat_menu_sub(&main_menu, "notepad");
	creat_menu_sub(&main_menu, "html");
	creat_menu_sub(&main_menu, "2048");

	//底层bar
	layer_square(layer_desktop, 0, 0, layer_desktop->wide,TASKBAR_UP_HIGH, COLOR_BLACK30);
	
	int click_time = 0;
	int click_can = 0;
	menu_show(&main_menu);
	//exception_handler(0x100,1, 456, 56456, 8, 41);
	for(;;){
		if(refresh_bar != 0){
			//底层bar
			//layer_square(layer_desktop, 0, 0, layer_desktop->wide,TASKBAR_UP_HIGH, COLOR_BLACK30);
			//layer_square(layer_desktop, 0, layer_desktop->high-TASKBAR_DOWN_HIGH, layer_desktop->wide,TASKBAR_DOWN_HIGH, COLOR_WHITE150);
			//tbb_display();
			layer_square(layer_desktop, layer_desktop->wide - BAR_CHAR_WIDE*22, 0, BAR_CHAR_WIDE*19,TASKBAR_UP_HIGH, COLOR_BLACK30);
			
			//time
			layer_square(layer_desktop, clock_x - BAR_CHAR_WIDE*19, clock_y, 8*9,16, bc_color);
			layer_integer(layer_desktop, clock_x - BAR_CHAR_WIDE*19 , clock_y, 10, time.hour, font_color);
			layer_string(layer_desktop, clock_x - BAR_CHAR_WIDE*17, clock_y, ":", font_color);
			//layer_square(layer_desktop, clock_x - BAR_CHAR_WIDE*16, clock_y, 8*6,16, bc_color);
			layer_integer(layer_desktop, clock_x - BAR_CHAR_WIDE*16 , clock_y, 10, time.minute, font_color);
			layer_string(layer_desktop, clock_x - BAR_CHAR_WIDE*14, clock_y, ":", font_color);
			layer_integer(layer_desktop , clock_x - BAR_CHAR_WIDE*13, clock_y, 10, time.second, font_color);

			//date
			layer_square(layer_desktop , clock_x - BAR_CHAR_WIDE*10, clock_y, BAR_CHAR_WIDE*10,16, bc_color);
			layer_integer(layer_desktop , clock_x - BAR_CHAR_WIDE*10 , clock_y, 10, time.year, font_color);
			layer_string(layer_desktop , clock_x - BAR_CHAR_WIDE*6, clock_y, "/", font_color);
			
			layer_integer(layer_desktop , clock_x - BAR_CHAR_WIDE*5 , clock_y, 10, time.month, font_color);
			layer_string(layer_desktop , clock_x - BAR_CHAR_WIDE*3, clock_y, "/", font_color);
			layer_integer(layer_desktop , clock_x - BAR_CHAR_WIDE*2, clock_y, 10,time.day, font_color);

			menu_show(&main_menu);
	
			//task_wakeup(task_system);
			refresh_bar = 0;
		}
		click_time++;
		if(click_time > 0x20000){
			click_can = 1;
		}
		//collision_box_show(main_menu.box);
		if(collision_box_update(main_menu.box)){
			if(get_mouse_button(MOUSE_LEFT) && main_menu.status == MENU_STATUS_OFF && click_can){
				//debug_printf("menu on!\n");
				main_menu.status = MENU_STATUS_ON;
				for(i = 0; i < MENU_SUB_MAX; i++){
					if(main_menu.menu_sub_table[i] != NULL){
						menu_sub_show(main_menu.menu_sub_table[i]);
					}
				}
				click_can = 0;
				click_time = 0;
				
				
				
			}else if(get_mouse_button(MOUSE_LEFT) && main_menu.status == MENU_STATUS_ON && click_can){
				//debug_printf("menu off!\n");
				menu_close(&main_menu);
				click_can = 0;
				click_time = 0;
			}
		}
		
		if(main_menu.status == MENU_STATUS_ON){
			
			for(i = 0; i < MENU_SUB_MAX; i++){
				if(main_menu.menu_sub_table[i] != NULL){
					//menu_sub_show(main_menu.menu_sub_table[i]);
					
					if(collision_box_update(main_menu.menu_sub_table[i]->box)){
						if(get_mouse_button(MOUSE_LEFT) && main_menu.menu_sub_table[i]->box->status == CLN_BOX_STATUS_TOUCHED){
							if(main_menu.menu_sub_table[i]->title == "console"){
								open_console();
								//debug_printf("menu sub clicked!\n");
								menu_close(&main_menu);
							}else{
								//点击了制定位置
								application_command(main_menu.menu_sub_table[i]->title);
								
								//debug_printf("menu sub clicked!\n");
								menu_close(&main_menu);
								//break;
							}
							
						}
					}
					
					
					
				}
			}
		}
		
	
		
		//collision_box_show(menu.box);
		
		//layer_square(layer_desktop, menu_cbox->x, menu_cbox->y, 10, 10, COLOR_BLACK);
		
		/*
		for(i = 0; i < BUTTON_MAXS; i++){
			if(button_tables_ptr[i] != NULL){
				button = button_tables_ptr[i];
				if(button_mouse_check(button)){
					//gprintk("touched ");
					//sys_milliDelay(1000);
					button_status(button, 1);
					button_show(button);
					if(get_mouse_button(1)){
						//gprintk("clicked ");
						//sys_milliDelay(1000);
						button_status(button, 2);
						if(button->function != NULL){
							button->function(button);
						}
						button_show(button);
					}
				}else{
					//sys_milliDelay(1000);
					button_status(button, 0);
					button_show(button);
				}
				
			}
		}*/
		/*
		for(i = 0; i < task_manage->task_running; i++){
			if(task_manage->tasks_ptr[i]->status != TASK_UNUSED){
				layer_square(layer_desktop , 0, i*16, 50,16, COLOR_WHITE);
				layer_integer(layer_desktop , 0, i*16, 10, task_manage->tasks_ptr[i]->status, COLOR_BLACK);
			}
		}
		*/
		/*if(button_mouse_check(btn0)){
			//gprintk("touched ");
			sys_milliDelay(1000);
			button_status(btn0, 1);
			button_show(btn0);
			if(get_mouse_button(1)){
				//gprintk("clicked ");
				sys_milliDelay(1000);
				button_status(btn0, 2);
				button_show(btn0);
			}
		}else{
			sys_milliDelay(1000);
			button_status(btn0, 0);
			button_show(btn0);
		}
		*/
		
		
		
	}
}
//box
void collision_box_init(struct collision_box *c_box, int x, int y, int width, int height)
{
	c_box->x = x;
	c_box->y = y;
	c_box->width = width;
	c_box->height = height;
	c_box->status = CLN_BOX_STATUS_IDLE;
	c_box->color = COLOR_BLUE;
}

void collision_box_bound(struct collision_box *c_box,struct layer *layer)
{
	c_box->layer = layer;
}

int collision_box_update(struct collision_box *c_box)
{
	if(c_box->x <= get_mouse_x() && get_mouse_x() < c_box->x + c_box->width && c_box->y <= get_mouse_y() && get_mouse_y() < c_box->y + c_box->height){
		
		c_box->status = CLN_BOX_STATUS_TOUCHED;
		//debug_printf("t! ");
		return 1;
	}else{
		
		c_box->status = CLN_BOX_STATUS_IDLE;
		return 0;
	}
}

void collision_box_show(struct collision_box *c_box)
{
	if(c_box->layer != NULL){
		layer_square(c_box->layer, c_box->x, c_box->y, c_box->width, c_box->height, c_box->color);
	}
}
//menu
void menu_init(struct menu *menu, int x, int y, int width, int height, char *title)
{
	menu->x = x;
	menu->y = y;
	menu->width = width;
	menu->height = height;
	menu->status = MENU_STATUS_OFF;
	menu->color = COLOR_BLACK;
	menu->title = title;
	menu->menu_sub_counts = 0;
	menu->box = kmalloc(sizeof(struct collision_box));
	
	collision_box_init(menu->box, menu->x, menu->y, menu->width, menu->height);
	collision_box_bound(menu->box,menu->layer);
	//

}

void menu_bound(struct menu *menu,struct layer *layer)
{
	menu->layer = layer;
}

void menu_show(struct menu *menu)
{
	layer_square(menu->layer, menu->x, menu->y, menu->width, menu->height, menu->color);
	layer_string(menu->layer, menu->x+1, menu->y+1, menu->title, COLOR_WHITE);

}

void menu_close(struct menu *menu)
{
	int i;
	menu->status = MENU_STATUS_OFF;
	for(i = 0; i < MENU_SUB_MAX; i++){
		if(menu->menu_sub_table[i] != NULL){					
			menu_sub_hide(menu->menu_sub_table[i]);
		}
	}
	
}

//menu_sub
void menu_sub_init(struct menu_sub *menu_sub, int x, int y, int width, int height)
{
	menu_sub->x = x;
	menu_sub->y = y;
	menu_sub->width = width;
	menu_sub->height = height;
	menu_sub->status = MENU_SUB_USING;
	menu_sub->color = COLOR_BLACK;
	
	menu_sub->box = kmalloc(sizeof(struct collision_box));
	
	collision_box_init(menu_sub->box, menu_sub->x, menu_sub->y, menu_sub->width, menu_sub->height);
	collision_box_bound(menu_sub->box,menu_sub->layer);
}

void menu_sub_bound(struct menu_sub *menu_sub,struct layer *layer)
{
	menu_sub->layer = layer;
}

int menu_sub_update(struct menu_sub *menu_sub)
{
	if(menu_sub->x <= get_mouse_x() && get_mouse_x() < menu_sub->x + menu_sub->width && menu_sub->y <= get_mouse_y() && get_mouse_y() < menu_sub->y + menu_sub->height){
		
		menu_sub->status = CLN_BOX_STATUS_TOUCHED;
		//debug_printf("t! ");
		return 1;
	}else{
		menu_sub->status = CLN_BOX_STATUS_IDLE;
		return 0;
	}
}

void menu_sub_show(struct menu_sub *menu_sub)
{
	if(menu_sub->layer != NULL){
		layer_square(menu_sub->layer, menu_sub->x, menu_sub->y, menu_sub->width, menu_sub->height, menu_sub->color);
		layer_string(menu_sub->layer, menu_sub->x+2, menu_sub->y+2, menu_sub->title, COLOR_WHITE);
	}
	
}

void menu_sub_hide(struct menu_sub *menu_sub)
{
	if(menu_sub->layer != NULL){
		layer_square(menu_sub->layer, menu_sub->x, menu_sub->y, menu_sub->width, menu_sub->height, DEFAULT_DESKTOP_COLOR);
	}
}

struct menu_sub *creat_menu_sub(struct menu *menu, char *title)
{
	struct menu_sub *ms = kmalloc(sizeof(struct menu_sub));
	int i;
	menu_sub_init(ms, menu->x, menu->y + menu->height + menu->menu_sub_counts *MENU_SUB_HEIGHT , MENU_SUB_WIDTH, MENU_SUB_HEIGHT);
	menu_sub_bound(ms,menu->layer);
	ms->title = title;
	menu->menu_sub_counts++;
	for(i = 0; i < MENU_SUB_MAX; i++){
		if(menu->menu_sub_table[i] == NULL){
			menu->menu_sub_table[i] = ms;
			break;
		}
	}

	return ms;
}

void tbb_init(struct task_bar_block *tbb, int pos, int status)
{
	tbb->x = pos;
	tbb->y = 2;
	tbb->wide = TBB_BLOCK;
	tbb->high = TBB_BLOCK;
	tbb->color = COLOR_WHITE;
	tbb->status = status;
}

void tbb_show(struct task_bar_block *tbb)
{
	draw_square24(layer_desktop->buf ,layer_desktop->wide,4+tbb->x*TBB_SIZE,layer_desktop->high-TASKBAR_DOWN_HIGH+tbb->y,tbb->wide,tbb->high, COLOR_WHITE150);
	
	if(tbb->status == TBB_STATUS_ON){
		tbb->color = COLOR_WHITE230;
	}else{
		tbb->color = COLOR_WHITE200;
	}
	
	draw_square24(layer_desktop->buf ,layer_desktop->wide,4+tbb->x*TBB_SIZE,layer_desktop->high-TASKBAR_DOWN_HIGH+tbb->y,tbb->wide,tbb->high, tbb->color);
	layer_refresh(layer_desktop, tbb->x,tbb->y, 4+tbb->x*TBB_SIZE + tbb->wide, layer_desktop->high-TASKBAR_DOWN_HIGH+tbb->y + tbb->high);
}

