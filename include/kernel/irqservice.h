#ifndef _IRQSERVICE_H
#define _IRQSERVICE_H
#include "kernel/types.h"
#include "kernel/task.h"
#include "kernel/ioqueue.h"

#define KEYBOARD_DATA_LOW 0
#define KEYBOARD_DATA_HIGH (KEYBOARD_DATA_LOW+256)

#define CLOCK_DATA_LOW KEYBOARD_DATA_HIGH
#define CLOCK_DATA_HIGH (CLOCK_DATA_LOW+256)

#define MOUSE_DATA_LOW CLOCK_DATA_HIGH
#define MOUSE_DATA_HIGH (MOUSE_DATA_LOW+256)

#define HD_DATA_LOW MOUSE_DATA_HIGH
#define HD_DATA_HIGH (HD_DATA_LOW+256)

struct irq_service
{
	struct ioqueue ioqueue;	//irq的数据缓冲
	struct task *task;
	int keyboard_key;	//output data是在输出的时候产生的数据， data是最后保存的数据
	int mouse_input_success;	//output data是在输出的时候产生的数据， data是最后保存的数据
	int clock_data;
	
	uint32_t counter;
	
	
	bool active;	//是否活跃
};
extern struct irq_service irq_service;

void irq_service_process();


#endif