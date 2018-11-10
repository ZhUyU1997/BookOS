#ifndef _SYSCALL_H
#define _SYSCALL_H
#include "kernel/types.h"
#include "kernel/task.h"
#include "gui/layer.h"
#include "gui/window.h"
#include "lib/GL.h"

#define _NR_PUTMAILBOX 1
#define _NR_GETMAILBOX 2

#define _NR_MALLOC 3
#define _NR_FREE 4

#define _NR_EXIT 10
#define _NR_WAIT 11
#define _NR_GETPID 12
#define _NR_PS 13

#define _NR_GETCH 20
#define _NR_PUTCH 21
#define _NR_PRINTF 22
#define _NR_CLEAR 23

#define _NR_GETICKS 30

#define _NR_CREATE_WINDOW 31
#define _NR_CLOSE_WINDOW 32

#define NR_SYS_CALL 40

extern sys_call_t sys_call_table[NR_SYS_CALL];
void init_syscall();//初始化中断调用

/*interrupt.asm*/
void intrrupt_sys_call();//中断调用服务程序

/*syscall.c*/
/*系统调用函数*/
int sys_getTicks();
int sys_print();
int sys_getch();

/**
i1 传入是function 输出是返回值
*/
struct syscall_package_t
{
	int i1,i2,i3,i4,i5,i6;	
	void *v1,*v2;
};

void syscall_package(struct syscall_package_t *sp);	//asm
void do_syscall_package(struct syscall_package_t *sp);	//c

#define SYSCALL_CREATE_WINDOW 20
#define SYSCALL_DRAW_PIXEL 21
#define SYSCALL_DRAW_LINE 22
#define SYSCALL_DRAW_TEXT 23
#define SYSCALL_DRAW_INTEGER 24
#define SYSCALL_CLOSE_WINDOW 25

/**
GL
*/
#define SYSCALL_GL_INIT 30
#define SYSCALL_GL_WINDOW_SIZE 31
#define SYSCALL_GL_WINDOW_POS 32
#define SYSCALL_GL_CREATE_WINDOW 33
#define SYSCALL_GL_CLOSE_WINDOW 34

#define SYSCALL_GL_BEGIN 35
#define SYSCALL_GL_END 36
#define SYSCALL_GL_VERTEX 37

#define SYSCALL_GL_CLEAR 38
#define SYSCALL_GL_COLOR_CLEAR 39
#define SYSCALL_GL_COLOR 40
#define SYSCALL_GL_PIXEL_SIZE 41
#define SYSCALL_GL_POINT_SIZE 42
#define SYSCALL_GL_POLYGON_MODE 43
#define SYSCALL_GL_TEXT_BUFFER 44
#define SYSCALL_GL_INTEGER_SET 45
#define SYSCALL_GL_INTEGER_MODE 46

#define SYSCALL_GL_DRAW_PIXEL 47
#define SYSCALL_GL_DRAW_LINE 48
#define SYSCALL_GL_DRAW_TEXT 49
#define SYSCALL_GL_DRAW_INTEGER 50

#define SYSCALL_GL_FLUSH 51



int do_get_ticks();
int do_getch();
void do_task_exit();

#endif

