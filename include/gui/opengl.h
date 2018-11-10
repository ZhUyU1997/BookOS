#ifndef _OPENGL_H
#define	_OPENGL_H
#include "kernel/types.h"


/*
当前完成 绘制像素 绘制点，绘制线， 绘制矩形， 绘制多边形

*/

// gl begin mode
#define GL_POINTS 1
#define GL_LINES 2
#define GL_POLYGON 3
#define GL_RECT 4
#define GL_TRIANGLES 5
#define GL_TEXT 6
#define GL_INTEGER 7


#define GL_FILL 10

//vertex buffer max
#define GL_VERTEX_MAX 32

#define GL_COLOR_BUFFER_BIT 1

#define GL_INTEGER_16 16
#define GL_INTEGER_10 10

/*顶点结构体*/
struct vertex
{
	int x, y;
};

/*
gl的信息结构体
*/
struct graphics_library
{
	//window 
	uint32_t windowWidth, windowHeight;
	uint32_t windowPosX, windowPosY;
	uint32_t windowX0, windowY0;
	char *windowTitle;
	//pix
	uint32_t xfactor, yfactor;
	//mode
	GLenum mode, polygonMode, beginMode, integerMode;
	//vertex
	struct vertex *vertexTable;
	int vertexIndex;
	//text
	char *text;
	//text
	int integer;
	
	//other
	uint32_t width, height;
	uint32_t x, y;
	uint32_t x0, y0, x1, y1;
	uint32_t color;
	uint32_t cleanColor;
	char *title;
	
	//system
	struct window *window;
	bool enable_vertex;	//顶点开关，判断是否可以往里面写数据
};

void gl_init();
void gl_init_window_size(uint32_t width, uint32_t height);
void gl_init_window_position(uint32_t x, uint32_t y);
void gl_create_window(char *title);
void gl_close_window();

void gl_begin(int mode);
void gl_end();
void gl_flush();

void gl_pixel_size(GLint xfactor,GLint yfactor);

void gl_vertex2i(int x, int y);
void gl_draw_pixel2i(int x, int y);
void gl_draw_line4i(int x0, int y0, int x1, int y1);
void gl_draw_text(int x, int y);
void gl_draw_integer2i(int x, int y);


void gl_point_size(int size);
void gl_polygon_mode(int mode);
void gl_integer_mode(int mode);
void gl_integer_set(int integer);
void gl_text_buffer(char *text);
void gl_color(GLint color);
void gl_clear_color(GLint color);


#endif
