#ifndef _LIB_GL_H
#define _LIB_GL_H
#include "kernel/types.h"
#include "gui/window.h"

/*初始化一个gl*/
void glInit();
/*创建窗口*/
void glInitWindowSize(uint32_t width, uint32_t height);
void glCreateWindow(char *title);
void glInitWindowPosition(uint32_t x, uint32_t y);
void glCloseWindow();

/*绘制图形*/
void glBegin(GLenum mode);
void glEnd();
void glVertex2i(int x, int y);
void glClear(GLbitfield mask);
/*设置信息*/
void glClearColor(GLint color);
void glColor3i(GLint red,GLint green,GLint blue);
void glPixelSize2i(GLint xfactor,GLint yfactor);
void glPointSize(GLint size);
void glPolygonMode(GLenum mode);
void glTextBuffer(char *text);
void glColor1i(GLint color);
void glIntegerSet(int integer);
void glIntegerMode(int mode);

/*
系统调用相关
只能被其它函数调用
*/
void glDrawPixel2i(int x, int y);
void glDrawline4i(int x0, int y0, int x1, int y1);
void glDrawText2i(int x, int y);
void glDrawInteger2i(int x, int y);

//gl 封装
void glPoint(int x, int y, int color);
void glLine(int x1, int y1, int x2, int y2, int color);
void glTrangle(int x1, int y1, int x2, int y2, int x3, int y3, int color);
void glRect(int x, int y, int width, int height, int color);
void glText(int x, int y, char *text, int color);
void glInteger(int x, int y, int integer, int color);

#endif
