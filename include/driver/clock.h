#ifndef _CLOCK_H
#define _CLOCK_H
#include "kernel/types.h"

#define PIT_CTRL	0x0043//控制端口
#define PIT_CNT0	0x0040//数据端口
#define TIMER_FREQ     1193200	
#define HZ             100	//1000 快速 100 普通
#define CLOCK_IRQ 0//时钟中断的irq号

extern int ticks;//全局变量ticks

void IRQ_clock();//汇编处理
void clock_handler(int irq);//中断处理过程
void schdule();//任务调度
void intr_exit_clock();

struct time
{
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t second;
	uint32_t minute;
	uint32_t hour;
};

struct clock
{
	
	uint32_t ticks;
	uint32_t last_ticks;
	
	bool can_schdule;
};

extern struct clock clock;
extern struct time time;

void clock_change_date_time();
void print_date_time(int date, int tm);

int clock_getticks();
void clock_main();

void enable_schdule();
void disable_schdule();

#endif

