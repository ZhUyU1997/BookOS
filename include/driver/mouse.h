#ifndef _MOUSE_H
#define _MOUSE_H
#include "kernel/types.h"

#define MOUSE_IRQ 12//鼠标irq
#define	CASCADE_IRQ	2//从盘联系irq
#define	MOUSE_FIFO_BUF_LEN	256//鼠标缓冲区

#define	MOUSE_LEFT	1
#define	MOUSE_RIGHT	2
#define	MOUSE_ROLLER	3

#define	MOUSE_CLICK_TIME	5*1000	//1000 = 1秒
/*
鼠标结构体
*/
struct mouse {
	unsigned char read_buf[3], phase;
	int mouse_fifo_buf[MOUSE_FIFO_BUF_LEN];//鼠标缓冲区
	int x_increase, y_increase, button;
	int x, y;
	int old_x, old_y;
//鼠标位置
	int new_x, new_y;//fifo结构
	bool key_left, key_right, key_roller;//鼠标按键状态
	bool key_left_continue, key_right_continue, key_roller_continue;//鼠标按键状态
	
	int rf_x0,rf_y0,rf_x1,rf_y1;	//刷新区域
	
	
	bool active;	//是否有鼠标行动
};
extern struct mouse mouse;//全局变量

void IRQ_mouse();//汇编处理
void mouse_handler(int32_t irq);
void init_mouse();
void enable_mouse(struct mouse *mouse);
int mouse_read(struct mouse *mouse, unsigned char data);

void task_mouse_entry();

bool get_mouse_button(int key);
int32_t get_mouse_x();
int32_t get_mouse_y();


void mouse_main();

#endif

