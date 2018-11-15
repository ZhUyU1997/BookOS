#include "driver/mouse.h"
#include "driver/keyboard.h"
#include "kernel/8259a.h"
#include "kernel/memory.h"
#include "kernel/irqservice.h"
#include "lib/math.h"
#include "driver/clock.h"

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

struct mouse mouse;

void init_mouse()
{
	mouse.x = mouse.old_x = 0;		//鼠标信息初始化
	mouse.y = mouse.old_y = 0;
	
	mouse.key_left = false;
	mouse.key_right = false;
	mouse.key_roller = false;
	mouse.key_left_continue = false;
	mouse.key_right_continue = false;
	mouse.key_roller_continue = false;

	//初始化鼠标中断
	put_irq_handler(MOUSE_IRQ, mouse_handler);
	//打开从片
	enable_irq(CASCADE_IRQ);
	/*//打开鼠标中断
	enable_irq(MOUSE_IRQ);
	//激活鼠标前要设定键盘相关信息
	enable_mouse(&mouse);*/
}

void enable_mouse(struct mouse *mouse)
{
	/* 激活鼠标 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/*键盘控制器返回ACK(0xfa)*/
	mouse->phase = 0;
	
	return;
}

void mouse_handler(int32_t irq)
{
	uint8_t data;
	data = io_in8(PORT_KEYDAT);
	mouse.old_x = mouse.x;
	mouse.old_y = mouse.y;
	ioqueue_put(&irq_service.ioqueue, data + MOUSE_DATA_LOW);
	
	irq_service.task->priority = PRIORITY_LEVEL_I;
	task_wakeup(irq_service.task);
	return;
}

bool get_mouse_button(int key)
{
	switch(key){
		case MOUSE_LEFT:
			return mouse.key_left;
		case MOUSE_RIGHT:
			return mouse.key_right;
		case MOUSE_ROLLER:
			return mouse.key_roller;

		default : break;
	}
	return false;
}

int32_t get_mouse_x()
{
	return mouse.x;
}
int32_t get_mouse_y()
{
	return mouse.y;
}

void mouse_main()
{
	//从缓冲区读取一个数据，并解析每个数据，如果解析成功
	if(mouse_read(&mouse, ioqueue_get(&irq_service.ioqueue, IQ_MODE_MOVE) - MOUSE_DATA_LOW) != 0) {
		//对原始数据操作
		mouse.x += mouse.x_increase;
		mouse.y += mouse.y_increase;

		if (mouse.x < 0) {
			mouse.x = 0;
		}
		if (mouse.y < 0) {
			mouse.y = 0;
		}
		if (mouse.x > 80) {
			mouse.x = 80;
		}
		if (mouse.y >= 25) {
			mouse.y = 25;
		}
		if((mouse.button & 0x01) != 0){
			mouse.key_left = true;
		}else{
			mouse.key_left = false;
		}
		if((mouse.button & 0x02) != 0){
			mouse.key_right = true;
		}else{
			mouse.key_right = false;
		}
		if((mouse.button & 0x04) != 0){
			mouse.key_roller = true;
		}else{
			mouse.key_roller = false;
		}
		
	}
}

int mouse_read(struct mouse *mouse, unsigned char data)
{
	if (mouse->phase == 0) {
		if (data == 0xfa) {
			mouse->phase = 1;
		}
		return 0;
	}
	if (mouse->phase == 1) {
		if ((data & 0xc8) == 0x08) {
			mouse->read_buf[0] = data;
			mouse->phase = 2;
		}
		return 0;
	}
	if (mouse->phase == 2) {
		mouse->read_buf[1] = data;
		mouse->phase = 3;
		return 0;
	}
	if (mouse->phase == 3) {
		mouse->read_buf[2] = data;
		mouse->phase = 1;
		mouse->button = mouse->read_buf[0] & 0x07;
		mouse->x_increase = mouse->read_buf[1];
		mouse->y_increase = mouse->read_buf[2];
		if ((mouse->read_buf[0] & 0x10) != 0) {
			mouse->x_increase |= 0xffffff00;
		}
		if ((mouse->read_buf[0] & 0x20) != 0) {
			mouse->y_increase |= 0xffffff00;
		}
		mouse->y_increase = - mouse->y_increase;
		return 1;
	}
	return -1; 
}