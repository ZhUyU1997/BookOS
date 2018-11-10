#include "gui/opengl.h"
#include "kernel/task.h"
#include "kernel/debug.h"
extern struct layer *layer_desktop;

void gl_init()
{
	struct task *task = task_current();

	//为任务的gl分配内存
	//分配时要注意是给那个内存分配地址，别弄错了
	if(task->gl == NULL){
		task->gl = (struct graphics_library *)kmalloc(sizeof(struct graphics_library));
		if(task->gl == NULL){
			return;
		}
		//debug_printf("gl address:%x", (int )task->gl);
		//初始化一些其它数据
		task->gl->vertexIndex = 0;
		task->gl->enable_vertex = false;
		task->gl->vertexTable = (struct vertex *)kmalloc(8*GL_VERTEX_MAX);	//32个顶点

		//debug_printf("init gl done!\n");
	}
}
void gl_init_window_size(uint32_t width, uint32_t height)
{
	struct task *task = task_current();

	task->gl->vertexIndex = 0;

	struct graphics_library *gl;
	
	gl = task->gl;
	gl->windowWidth = width;
	gl->windowHeight = height;
	
	//debug_printf("window size:%d %d\n", gl->windowWidth, gl->windowHeight);
}

void gl_init_window_position(uint32_t x, uint32_t y)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->windowPosX = x;
	gl->windowPosY = y;
	//debug_printf("window pos:%d %d\n", gl->windowPosX, gl->windowPosY);
}

void gl_create_window(char *title)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->windowTitle = title;
	
	struct window * window;
	
	//debug_printf("%s",gl->windowTitle);
	window = create_window(gl->windowPosX,gl->windowPosY,gl->windowWidth,gl->windowHeight,gl->windowTitle);
	
	//切换激活窗口
	window_focus_off(active_window);
	
	window_top(window);
	window_focus_on(window);
	
	active_window = window;
	//
	gl->window = window;
	task->window = window;
	
	gl->windowX0 = window->x0;
	gl->windowY0 = window->y0;
	//图层与任务
	window->layer->task = task;
}

void gl_close_window()
{
	struct task *task = task_current();
/*
	if(task->window != NULL){
		window_free(task->gl->window);
	}
	//释放占用的内存
	if(task->gl->vertexTable != NULL){
		kfree(task->gl->vertexTable);
	}
	if(task->gl != NULL){
		kfree(task->gl);
	}*/
	layer_height(task->window->layer, -1);
	//task->gl = NULL;
	
	struct window *window_top = window_topest();
	//激活下一个窗口
	if(window_top != NULL){
		active_window = window_top;
		//active_layer = window_top->layer;
		window_focus_on(active_window);
	}

}

void gl_begin(int mode)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	
	gl->beginMode = mode;
	//清空顶点缓存
	gl->vertexIndex = 0;
	gl->enable_vertex = true;
	//debug_printf("gl_begin mode:%d\n", gl->beginMode);
}

void gl_end()
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->enable_vertex = false;
	//debug_printf("gl_end mode:%d\n", gl->beginMode);
}

void gl_flush()
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	
	//debug_printf("gl_flush mode:%d\n", gl->beginMode);
	int i, j, n;
	switch(gl->beginMode){
		case GL_POINTS:
			while(gl->vertexIndex > 0){
				gl_draw_pixel2i(gl->vertexTable[gl->vertexIndex - 1].x, gl->vertexTable[gl->vertexIndex - 1].y);
				gl->vertexIndex--;
			}
			break;
		case GL_LINES:
			while(gl->vertexIndex > 0){
				gl_draw_line4i(gl->vertexTable[gl->vertexIndex].x, gl->vertexTable[gl->vertexIndex].y, gl->vertexTable[gl->vertexIndex-1].x, gl->vertexTable[gl->vertexIndex-1].y);
				gl->vertexIndex--;
			}
			break;
		case GL_TRIANGLES:
			n = gl->vertexIndex/3;	//判断有几个三角形，如果不构成三角形，那就不显示
			i = 0;
			while(n > 0){
				gl_draw_line4i(gl->vertexTable[i+2].x, gl->vertexTable[i+2].y, gl->vertexTable[i].x, gl->vertexTable[i].y);
				
				gl_draw_line4i(gl->vertexTable[i].x, gl->vertexTable[i].y, gl->vertexTable[i+1].x, gl->vertexTable[i+1].y);
				gl_draw_line4i(gl->vertexTable[i+1].x, gl->vertexTable[i+1].y, gl->vertexTable[i+2].x, gl->vertexTable[i+2].y);
				n--;
				i += 3;
			}
			break;
		case GL_POLYGON:
			
			if(gl->polygonMode == GL_LINES){
				while(gl->vertexIndex > 0){
					gl_draw_line4i(gl->vertexTable[gl->vertexIndex].x, gl->vertexTable[gl->vertexIndex].y, gl->vertexTable[gl->vertexIndex-1].x, gl->vertexTable[gl->vertexIndex-1].y);
					gl->vertexIndex--;
				}
			}else if(gl->polygonMode == GL_POINTS){
				while(gl->vertexIndex > 0){
					gl_draw_pixel2i(gl->vertexTable[gl->vertexIndex - 1].x, gl->vertexTable[gl->vertexIndex - 1].y);
					gl->vertexIndex--;
				}
			}else if(gl->polygonMode == GL_FILL){	//支持4个定点的矩形
				/*//(x0, y0) gl->vertexTable[0].x, gl->vertexTable[0].y
				//(x1, y1) gl->vertexTable[1].x, gl->vertexTable[1].y
				//(x2, y2) gl->vertexTable[2].x, gl->vertexTable[2].y
				//(x3, y3) gl->vertexTable[3].x, gl->vertexTable[3].y
				glPixelSize2i(gl,gl->vertexTable[1].x - gl->vertexTable[0].x ,gl->vertexTable[2].y - gl->vertexTable[1].y );	//改变像素大小
				glDrawPixel2i(gl, gl->vertexTable[0].x, gl->vertexTable[0].y);
				glPixelSize2i(gl,1,1);*/
			}
			break;
		case GL_RECT:
			
			gl_pixel_size(gl->vertexTable[1].x - gl->vertexTable[0].x ,gl->vertexTable[2].y - gl->vertexTable[1].y );	//改变像素大小
			gl_draw_pixel2i(gl->vertexTable[0].x, gl->vertexTable[0].y);
			gl_pixel_size(1,1);
			break;
		case GL_TEXT:
			for(i = 0; i < gl->vertexIndex; i++){
				gl_draw_text(gl->vertexTable[i].x, gl->vertexTable[i].y);
			}
			break;
		case GL_INTEGER:
			i = 0;
			gl_draw_integer2i(gl->vertexTable[i].x, gl->vertexTable[i].y);
			break;
		default : break;
	}
}



void gl_vertex2i(int x, int y)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	if(gl->enable_vertex){
		if(gl->vertexIndex < GL_VERTEX_MAX){
			gl->vertexTable[gl->vertexIndex].x = x;
			gl->vertexTable[gl->vertexIndex].y = y;
			gl->vertexIndex++;
		}
	}
}

void gl_draw_pixel2i(int x, int y)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	
	gl->x = x + gl->windowX0;
	gl->y = y + gl->windowY0;
	
	//执行操作
	layer_square(gl->window->layer, gl->x , gl->y , gl->xfactor, gl->yfactor, gl->color);
}

void gl_draw_line4i(int x0, int y0, int x1, int y1)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;

	gl->x0 = gl->windowX0 + x0;
	gl->y0 = gl->windowY0 + y0;
	gl->x1 = gl->windowX0 + x1;
	gl->y1 = gl->windowY0 + y1;
	
	//执行操作
	layer_line(gl->window->layer, gl->x0, gl->y0, gl->x1, gl->y1, gl->color);
}

void gl_draw_text(int x, int y)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	
	gl->x = gl->windowX0 + x;
	gl->y = gl->windowY0 + y;
	
	//执行操作
	layer_string(gl->window->layer, gl->x, gl->y, gl->text, gl->color);
}

void gl_draw_integer2i(int x, int y)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	
	gl->x = gl->windowX0 + x;
	gl->y = gl->windowY0 + y;
	
	//执行操作
	layer_integer(gl->window->layer, gl->x, gl->y, gl->integerMode, gl->integer,gl->color);
}

void gl_clear(GLbitfield mask)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	
	switch(mask){
		case GL_COLOR_BUFFER_BIT:
			gl_color(gl->cleanColor);
			gl_pixel_size(gl->window->wide,gl->window->high);	//改变像素大小
			gl_draw_pixel2i(0, 0);
			gl_pixel_size(1,1);	//默认像素为1
			break;
		default: break;
	}
}

void gl_clear_color(GLint color)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->cleanColor = color;
}

void gl_color(GLint color)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->color = color;
}

void gl_pixel_size(GLint xfactor,GLint yfactor)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->xfactor = xfactor;
	gl->yfactor = yfactor;
}

void gl_text_buffer(char *text)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->text = text;
	
}

void gl_integer_set(int integer)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->integer = integer;
}
void gl_integer_mode(int mode)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->integerMode = mode;
}

void gl_point_size(int size)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->xfactor = size;
	gl->yfactor = size;
}

void gl_polygon_mode(int mode)
{
	struct task *task = task_current();
	struct graphics_library *gl;
	gl = task->gl;
	gl->polygonMode = mode;
}


