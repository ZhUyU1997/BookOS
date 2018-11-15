#include "kernel/task.h"
#include "kernel/io.h"
#include "driver/clock.h"
#include "kernel/descriptor.h"
#include "kernel/task.h"
#include "kernel/tss.h"
#include "kernel/memory.h"
#include "kernel/irqservice.h"

struct clock clock;

struct time time;

extern struct task *task_system;

/*
time hour 24hour format，if is pm， we add 8， if is am we sub16
if computer is 0 am(0 )， we get hour 16
if computer is 11 pm(23 )， we get hour 15
*/

void init_clock(void)
{

	//0.001秒触发一次中断
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, (unsigned char) (TIMER_FREQ/HZ));
	io_out8(PIT_CNT0, (unsigned char) ((TIMER_FREQ/HZ) >> 8));

	clock.last_ticks = clock.ticks = 0;
	
	clock.can_schdule = true;
	
	//用一个循环让秒相等
	do{
		time.year = get_year();
		time.month = get_mon_hex();
		time.day = get_day_of_month();
		
		time.hour = get_hour_hex();
		time.minute =  get_min_hex8();
		time.second = get_sec_hex();
		//trans time to 24 format
		if(time.hour >= 16){
			time.hour -= 16;
		}else{
			time.hour += 8;
		}
	}while(time.second != get_sec_hex());
	
	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);
	put_str(">init clock\n");
}

void clock_handler(int irq)
{
	struct task *task = task_current();
	clock.last_ticks = clock.ticks;
	clock.ticks++;
	if(clock.ticks%HZ == 0){
		ioqueue_put(&irq_service.ioqueue, CLOCK_DATA_LOW);
		irq_service.task->priority = PRIORITY_LEVEL_Z;
		task_wakeup(irq_service.task);
	}
	if(task->ticks <= 0){
		if(clock.can_schdule){
			//put_str("!");
			schdule();
		}
	}else{
		task->run_ticks++;
		task->ticks--;
	}
}

void clock_main()
{
	//分析数据
	int clock_data = ioqueue_get(&irq_service.ioqueue, IQ_MODE_MOVE) - CLOCK_DATA_LOW;
	clock_change_date_time();
	//print_date_time(1, 1);
}

void clock_change_date_time()
{
	time.second++;
	if(time.second >= 60){
		time.minute++;
		time.second = 0;
		if(time.minute >= 60){
			time.hour++;
			time.minute = 0;
			if(time.hour >= 24){
				time.day++;
				time.hour = 0;
				
				//现在开始就要判断平年和闰年，每个月的月数之类的
				if(time.day > 30){
					time.month++;
					time.day = 1;
					
					if(time.month > 12){
						time.year++;	//到年之后就不用调整了
						time.month = 1;
					}
				}
			}
		}
	}
}

void print_date_time(int date, int tm)
{
	if(date){
		printk("[date] %d/%d/%d\n",
			time.year, time.month, time.day
		);
	}
	if(tm){
		printk("[time] %d:%d %d\n",
			time.hour, time.minute, time.second
		);
	}	
}

int clock_getticks()
{
	return clock.ticks;
}

void enable_schdule()
{
	clock.can_schdule = true;
}

void disable_schdule()
{
	clock.can_schdule = false;
}

