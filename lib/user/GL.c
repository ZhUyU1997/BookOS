#include "lib/GL.h"
#include "kernel/syscall.h"

void glInit()
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_INIT;
	syscall_package(&sp);
	return;
}

void glInitWindowSize(uint32_t width, uint32_t height)
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_WINDOW_SIZE;
	sp.i2 = width;
	sp.i3 = height;
	syscall_package(&sp);
}

void glInitWindowPosition(uint32_t x, uint32_t y)
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_WINDOW_POS;
	sp.i2 = x;
	sp.i3 = y;
	syscall_package(&sp);
}

void glCreateWindow(char *title)
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_CREATE_WINDOW;
	sp.v1 = title;
	syscall_package(&sp);
	return ;
}

void glCloseWindow()
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_CLOSE_WINDOW;
	syscall_package(&sp);
	return ;
}

void glBegin(GLenum mode)
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_BEGIN;
	sp.i2 = mode;
	syscall_package(&sp);
}

void glEnd()
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_END;
	syscall_package(&sp);
}

void glFlush()
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_FLUSH;
	syscall_package(&sp);
}

void glVertex2i(int x, int y)
{
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_VERTEX;
	sp.i2 = x;
	sp.i3 = y;
	syscall_package(&sp);
}

/*向gui发送绘制像素*/
void glDrawPixel2i(int x, int y)
{
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_DRAW_PIXEL;
	sp.i2 = x;
	sp.i3 = y;

	syscall_package(&sp);
	
	return ;
}

void glDrawline4i(int x0, int y0, int x1, int y1)
{
	
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_DRAW_LINE;
	sp.i2 = x0;
	sp.i3 = y0;
	sp.i4 = x1;
	sp.i5 = y1;
	syscall_package(&sp);
	return;
}

void glDrawText2i(int x, int y)
{
	
	
	//发送消息包信息
	struct syscall_package_t sp;
	
	sp.i1 = SYSCALL_GL_DRAW_TEXT;
	sp.i2 = x;
	sp.i3 = y;

	syscall_package(&sp);
	return;
}

void glDrawInteger2i(int x, int y)
{

	//发送消息包信息
	struct syscall_package_t sp;
	
	sp.i1 = SYSCALL_GL_DRAW_INTEGER;
	sp.i2 = x;
	sp.i3 = y;
	syscall_package(&sp);
	return;
}

void glClear(GLbitfield mask)
{
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_CLEAR;
	sp.i2 = mask;
	syscall_package(&sp);
}

void glClearColor(GLint color)
{
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_COLOR_CLEAR;
	sp.i2 = color;
	syscall_package(&sp);
}

void glColor3i(GLint red,GLint green,GLint blue)
{
	
	glColor1i(RGB24(red, green, blue));
}

void glColor1i(GLint color)
{

	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_COLOR;
	sp.i2 = color;
	syscall_package(&sp);
}

void glPixelSize2i(GLint xfactor,GLint yfactor)
{
	
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_PIXEL_SIZE;
	sp.i2 = xfactor;
	sp.i3 = yfactor;
	syscall_package(&sp);
	
}

void glTextBuffer(char *text)
{
	
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_TEXT_BUFFER;
	sp.v1 = text;
	syscall_package(&sp);
}

void glIntegerSet(int integer)
{
	
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_INTEGER_SET;
	sp.i2 = integer;
	syscall_package(&sp);
}

void glIntegerMode(int mode)
{
	
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_INTEGER_MODE;
	sp.i2 = mode;
	syscall_package(&sp);
}

void glPointSize(GLint size)
{
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_POINT_SIZE;
	sp.i2 = size;
	
	syscall_package(&sp);
}

void glPolygonMode(GLenum mode)
{
	//发送消息包信息
	struct syscall_package_t sp;
	sp.i1 = SYSCALL_GL_POLYGON_MODE;
	sp.i2 = mode;
	
	syscall_package(&sp);
}

/*gl 封装，建议在程序自己实现*/
void glPoint(int x, int y, int color)
{
	glColor1i(color);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
	glFlush();
}

void glLine(int x1, int y1, int x2, int y2, int color)
{
	glColor1i(color);
	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
	glFlush();
}

void glTrangle(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	glColor1i(color);
	glBegin(GL_TRIANGLES);
	glVertex2i(x1, y1);	//左上
	glVertex2i(x2, y2);	//右上
	glVertex2i(x3, y3);	//右下
	glEnd();
	glFlush();
}

void glRect(int x, int y, int width, int height, int color)
{
	glColor1i(color);
	glBegin(GL_RECT);
	glVertex2i( x, y);	//左上
	glVertex2i(x + width, y);	//右上
	glVertex2i(x + width, y + height);	//右下
	glVertex2i(x, y + height);	//左下
	glEnd();
	glFlush();
}

void glText(int x, int y, char *text, int color)
{
	glColor1i(color);
	glTextBuffer(text);
	glBegin(GL_TEXT);
	glVertex2i(x, y);
	glEnd();
	glFlush();
}

void glInteger(int x, int y, int integer, int color)
{
	glColor1i(color);
	glIntegerSet(integer);
	glBegin(GL_INTEGER);
	glVertex2i(x, y);
	glEnd();
	glFlush();
}

