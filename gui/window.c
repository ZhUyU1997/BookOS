#include "gui/window.h"
#include "gui/layer.h"
#include "gui/button.h"
#include "fs/fs.h"
#include "kernel/task.h"

extern struct layer_manage *layer_manage;
extern struct task *task_terminal;
extern struct layer *layer_desktop;

struct window *win0, *win1;

void init_windows()
{
	int i;
	
	//win0 = create_window(300,0,320,240,"Picture");
	//win1 = create_window(480,36,320,240,"File Manager");
	//task_terminal->window  = 
	
	/*struct button *button;
	struct block *dir_e = NULL;
	struct block *root_dir = (struct block *)opendir("/");
	int idx = 0;
	int levl = 0;

	if (root_dir) {
		button = button_create();
		button_layer(button, win1->layer);
		button_positon(button, levl*16, 24 + idx*20);
		button_size(button, 320-levl*16, 20);
		button_text(button, "/", "/", "/");
		button_text_color(button, COLOR_BLACK);
		button_function(button, button_show_dir);
		button_color(button, COLOR_TAUPE, COLOR_GREY, COLOR_GREY170);
		button->dir_entry = root_dir;
		
		button_show(button);
		button_table_add(button);
		idx++;
		levl++;
		layer_string(win1->layer, 4, 240-16, "/ open done!",COLOR_BLACK);
		
		char *type = NULL;
		dir_e = NULL;
		while ((dir_e = sys_readdir(root_dir))) {
			if(dir_e->type == FILE_TYPE_REGULAR) {
				type = "regular";
			}else {
				type = "directory";
			}
			button = button_create();
			button_layer(button, win1->layer);
			button_positon(button, levl*16, 24 + idx*20);
			button_size(button, 320 -levl*16, 20);
			button_text(button, dir_e->name, dir_e->name, dir_e->name);
			button_text_color(button, COLOR_BLACK);
			button_color(button, COLOR_TAUPE, COLOR_GREY, COLOR_GREY170);
			button_function(button, button_show_dir);
			button->dir_entry = dir_e;
			button_show(button);
			button_table_add(button);
			
			//layer_string(win1->layer, 4, 32+idx*16, type, COLOR_BLACK);
			//layer_string(win1->layer, 4+100, 32+idx*16, dir_e->name, COLOR_BLACK);
			idx++;
		}
	}else {
		layer_string(win1->layer, 4, 480-16, "/ open failed!", COLOR_BLACK);
	}*/
}
/*
int32_t sys_window_bmp(struct Window *Window, uint32_t x,uint32_t y, char *bmpname)
{
	layer_image_bmp(Window->window_handle->layer, x,y, bmpname);
}
*/
void window_square(struct window *win, uint32_t x,uint32_t y, uint32_t wide,uint32_t high, uint32_t color)
{
	layer_square(win->layer, win->x0 + x,win->y0 + y, wide,high,color);
}

void window_string(struct window *win, uint32_t x,uint32_t y, uint8_t *s, uint32_t color)
{
	layer_string(win->layer, win->x0 + x,win->y0 + y, s,color);
}

struct window *create_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height,uint8_t *title)
{
	struct window *window;
	struct layer *layer;
	uint8_t *buf;
	
	window = (struct window *)kmalloc(sizeof(struct window));
	layer = layer_alloc(LAYER_TYPE_WINDOW);
	
	//gprintk("alloc layer-> %d #", layer->type);
	
	window->layer = layer;	//互相绑定对象
	layer->window = window;	//互相绑定对象
	window->x = x;
	window->y = y;
	window->x0 = 1;	//x0 y0 是活动窗口的位置
	window->y0 = 24;
	window->width = width;
	window->height = height;
	window->wide = width-2;
	window->high = height - 1 - 24;
	
	window->title = title;
	window->layer->task = NULL;
	
	buf = (uint8_t *)kmalloc(window->width*window->height*3);
	
	layer_init(layer, buf, window->width, window->height ,LAYER_NOINVISIBLE);
	
	window_draw(buf,window->width, window->height, window->title, 0);
	
	layer_shift(layer,x,y);
	layer_height(layer,-1);
	
	layer_refresh(layer, 0, 0, layer->wide, layer->height);
	
	return window;
}

struct window *window_topest()
{
	int i;
	struct layer *layer;
	for(i = layer_manage->top-1; i >0; i--){
		layer = height2layer(i);
		if(layer->window != NULL && layer->window->layer != layer_desktop){
			
			return layer->window;
		}
	}
	return NULL;
}

//释放窗口以及其占用的空间
void window_free(struct window *window)
{
	//struct layer *layer = height2layer(layer_manage->top-1);
	//释放图层占用的数据
	kfree(window->layer->buf);	//释放图层占用的内存
	layer_free(window->layer);	//释放图层
	kfree(window);	//释放窗口结构体占用的内存
	
	struct window *window_top = window_topest();
	//激活下一个窗口
	if(window_top != NULL){
		active_window = window_top;
		active_layer = window_top->layer;
	}
	window_focus_on(active_window);

}

//让窗口显示在鼠标下面
void window_top(struct window *window)
{
	layer_height(window->layer,layer_manage->top);
	
}
//让窗口显示在桌面上一层
void window_bottom(struct window *window)
{
	layer_height(window->layer,1);
}

/* 
绘制一个窗口 
通过所给屏幕大小推断出窗口所有的大小
*/
void window_draw(uint8_t *buf,int wide, int high, char *title, char activate)  
{
	int title_len;
	int j, i;
	
	uint32_t color_bar, color_button,color_title,color_border;
	if(!activate){	// 0
		color_button = COLOR_BLACK;
		color_bar = RGB24(220,220,220);
		color_title = COLOR_BLACK;
		color_border = RGB24(220,220,220);
	}else{
		color_button = COLOR_BLACK;
		color_bar = WINDOW_COLOR_DEFAULT;
		color_title = COLOR_BLACK;
		color_border = WINDOW_COLOR_DEFAULT;
	}
	//绘制窗体及边框
	draw_square24(buf,wide, 1, 0, wide-2,24, color_bar);
	
	draw_square24(buf,wide, 0, 0, 1,high, color_border);
	draw_square24(buf,wide, 0, 0, wide,1, color_border);
	draw_square24(buf,wide, wide-1, 0, 1,high, color_border);
	draw_square24(buf,wide, 0, high-1, wide,1, color_border);
	//绘制活动区域-白色
	draw_square24(buf,wide, 1, 24, wide-2,high-1-24, COLOR_WHITE);
	
	//绘制标题
	title_len = strlen(title);
	draw_string24(buf,wide,  1+16+4, 4, color_title, title );
	
	draw_close_button(buf,wide, color_button, color_bar);
	/*
	draw_minimize_button(buf,wide, color_button, color_bar);
	draw_screen_button(buf,wide, color_button, color_bar);
	*/
} 

void window_change(struct layer *layer, char activate)
{
	int title_len;
	int j, i, wide = layer->wide, high = layer->high;
	char close_button_element;
	uint32_t color_button, color_bar, color_title;
	uint32_t close_button_color;
	uint8_t *buf = layer->buf;
	
	uint32_t color_border;
	if(!activate){	// 0
		color_button = COLOR_BLACK;
		color_bar = RGB24(220,220,220);
		color_title = COLOR_BLACK;
		color_border = RGB24(220,220,220);
	}else{
		color_button = COLOR_BLACK;
		color_bar = WINDOW_COLOR_DEFAULT;
		color_title = COLOR_BLACK;
		color_border = WINDOW_COLOR_DEFAULT;
	}
	
	draw_square24(buf,wide, 1, 0, wide-2,24, color_bar);
	
	draw_square24(buf,wide, 0, 0, 1,high, color_border);
	draw_square24(buf,wide, 0, 0, wide,1, color_border);
	draw_square24(buf,wide, wide-1, 0, 1,high, color_border);
	draw_square24(buf,wide, 0, high-1, wide,1, color_border);
	
	
	//绘制标题
	//title_len = strlen(title);
	//draw_string16(buf,wide, wide - 4 - title_len*8 - 8, 4, color_title, title );
	
	draw_close_button(buf,wide, color_button, color_bar);
	/*
	
	draw_minimize_button(buf,wide, color_button, color_bar);
	draw_screen_button(buf,wide, color_button, color_bar);*/
}


void window_switch(struct window *window, char activate)
{
	struct layer *layer = window->layer;
	int title_len;
	int j, i, wide = layer->wide, high = layer->high;
	char close_button_element;
	
	uint32_t color_button, color_bar, color_title;
	uint32_t close_button_color;
	uint8_t *buf = layer->buf;
	
	uint32_t color_border;
	if(!activate){	// 0
		color_button = COLOR_BLACK;
		color_bar = RGB24(220,220,220);
		color_title = COLOR_BLACK;
		color_border = RGB24(220,220,220);
	}else{
		color_button = COLOR_BLACK;
		color_bar = WINDOW_COLOR_DEFAULT;
		color_title = COLOR_BLACK;
		color_border = WINDOW_COLOR_DEFAULT;
	}
	
	draw_square24(buf,wide, 1, 0, wide-2,24, color_bar);
	
	draw_square24(buf,wide, 0, 0, 1,high, color_border);
	draw_square24(buf,wide, 0, 0, wide,1, color_border);
	draw_square24(buf,wide, wide-1, 0, 1,high, color_border);
	draw_square24(buf,wide, 0, high-1, wide,1, color_border);
	
	//绘制标题
	title_len = strlen(window->title);
	draw_string24(buf,wide,  1+16+4, 4, color_title, window->title );
	
	draw_close_button(buf,wide, color_button, color_bar);
	/*
	
	draw_minimize_button(buf,wide, color_button, color_bar);
	draw_screen_button(buf,wide, color_button, color_bar);*/
}


void draw_close_button(uint8_t *buf,int wide, uint32_t color_button, uint32_t color_bc)
{
	//绘制标题按钮
	static char button[14][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,},
		{0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,},
		{0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,},
		{0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,},
		{0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,},
		{0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,},
		{0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,},
		{0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,},
	};
	char button_element;
	uint32_t button_color;
	int i, j;
	for(j = 0; j < 14; j++){
		for(i = 0; i < 16; i++){
			button_element = button[j][i];
			if(button_element == 0){
				button_color = color_bc;
			}else if(button_element == 1){
				button_color = color_button;
			}else if(button_element == 2){
				button_color = color_button;	
			}
			draw_pix24(buf,wide, i+wide -20, j + 4 , button_color);
			//buf[(j + 4 ) * wide + (i+wide -20)] = button_color;
		}
	}
}

void draw_minimize_button(uint8_t *buf,int wide, uint32_t color_button, uint32_t color_bc)
{
	//绘制标题按钮
	/*static char close_button[16][16] = {
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,},
		{0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,},
		{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		
	};*/
	/*static char button[16][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,},
		{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,},
		{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,},
		{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,},
		{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,},
		{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,},
		{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		
	};*/
	/*static char button[14][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,},
		{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,},
		{0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,},
		{0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,},
		{0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,},
		{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
	};
	char button_element;
	uint16_t button_color;
	int i, j;
	for(j = 0; j < 14; j++){
		for(i = 0; i < 16; i++){
			button_element = button[j][i];
			if(button_element == 0){
				button_color = color_bc;
			}else if(button_element == 1){
				button_color = color_button;
			}else if(button_element == 2){
				button_color = color_button;	
			}
			buf[(j + 4 ) * wide + (4 +16 + 4 + i )] = button_color;
		}
	}*/
}

void draw_screen_button(uint8_t *buf,int wide, uint32_t color_button, uint32_t color_bc)
{
	//绘制标题按钮
	/*static char close_button[16][16] = {
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,},
		{0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,},
		{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,},
		{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,},
		{0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,},
		{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,},
		
	};*/
	
	/*static char button[14][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,},
		{0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},	
	};
	char button_element;
	uint16_t button_color;
	int i, j;
	for(j = 0; j < 14; j++){
		for(i = 0; i < 16; i++){
			button_element = button[j][i];
			if(button_element == 0){
				button_color = color_bc;
			}else if(button_element == 1){
				button_color = color_button;
			}else if(button_element == 2){
				button_color = color_button;	
			}
			buf[(j + 4 ) * wide + (4 +16+4+16 + 4 + i )] = button_color;
		}
	}*/
}

void window_on(struct layer *layer)
{
	if(layer->type == LAYER_TYPE_WINDOW){
		window_change(layer, 1);
	}
	
	layer->active = true;
	layer_refresh(layer, 0, 0, layer->wide, layer->high);
}
void window_off(struct layer *layer)
{
	if(layer->type == LAYER_TYPE_WINDOW){
		window_change(layer, 0);
	}
	layer->active = false;
	layer_refresh(layer, 0, 0, layer->wide, layer->high);
}

void window_focus_on(struct window *window)
{
	if(window->layer->type == LAYER_TYPE_WINDOW){
		window_switch(window, 1);
	}
	
	window->layer->active = true;
	layer_refresh(window->layer, 0, 0, window->layer->wide, window->layer->high);
}
void window_focus_off(struct window *window)
{
	if(window->layer->type == LAYER_TYPE_WINDOW){
		window_switch(window, 0);
	}
	
	window->layer->active = false;
	layer_refresh(window->layer, 0, 0, window->layer->wide, window->layer->high);
}

void draw_moving_window(uint8_t *buf, int wide, int high, int32_t bc)
{
	int x, y;
	for (y = 0; y < high; y++) {
		for (x = 0; x < wide; x++) {
			buf[y*wide+x] = bc;
		}
	}
	draw_square24(buf,wide, 0, 0, wide,1,RGB24(220, 220, 220));
	draw_square24(buf,wide, 0, 0, 1,high,RGB24(220, 220, 220));
	draw_square24(buf,wide, wide-1, 0, 1,high,RGB24(220, 220, 220));
	draw_square24(buf,wide, 0, high-1, wide,1,RGB24(220, 220, 220));
}

