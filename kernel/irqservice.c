#include "kernel/irqservice.h"
#include "kernel/task.h"
#include "driver/clock.h"
#include "driver/keyboard.h"
#include "driver/vga.h"
#include "driver/mouse.h"
#include "driver/console.h"
#include "shell/shell.h"
#include "graph/surface.h"

/**
各自注册自己的irq服务，然后在irq服务中来提供对数据的解读，并且保存最后的结果，我们需要时，只需要从中读取就是了
*/

struct irq_service irq_service;
void irq_service_process()
{
	struct task *task_self = task_current();
	int ioqueue_data;

	printk("%s is running...\n", task_self->name);
	ioqueue_init(&irq_service.ioqueue);
	irq_service.task = task_self;

	irq_service.keyboard_key = -1;
	irq_service.mouse_input_success = 0;
	irq_service.clock_data = -1;
	irq_service.counter = 0;

	irq_service.active = false;

	init_keyboard();
	init_mouse();

	for(;;){
		if(ioqueue_empty(&irq_service.ioqueue) && irq_service.active == false){
			irq_service.task->priority = PRIORITY_LEVEL_Z;
			task_sleep(task_self);
		}else{
			irq_service.active = true;
			ioqueue_data = ioqueue_get(&irq_service.ioqueue, IQ_MODE_IDLE);
			if(IQ_DATA_RANGE(ioqueue_data,KEYBOARD_DATA_LOW,KEYBOARD_DATA_HIGH)){
				//分析数据
				keyboard_main();
				
			}else if(IQ_DATA_RANGE(ioqueue_data,CLOCK_DATA_LOW,CLOCK_DATA_HIGH)){
				clock_main();
				
			}else if(IQ_DATA_RANGE(ioqueue_data,MOUSE_DATA_LOW,MOUSE_DATA_HIGH)){
				//分析数据
				mouse_main();
				
			}
			irq_service.active = false;
		}
	}
}
