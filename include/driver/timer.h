#ifndef _TIMER_H
#define _TIMER_H
#include "kernel/types.h"

#define TIMER_UNUSED 0
#define TIMER_ALLOC 1
#define TIMER_USING 2

#define TIMEROUT_MAX		0xffffffff	//超时最大值

/* timer.c */
#define MAX_TIMER		100
/* 定时器结构体 */
struct timer {
	unsigned int timeout, status;
	struct fifo32 *fifo;
	int data;
	int occured;	//是否发生了超时
	struct timer *next;
};
/* 管理定时器的结构 */
struct timer_manage {
	unsigned int count, next;
	struct timer *head;
	struct timer timers_table[MAX_TIMER];
};

extern struct timer_manage *timer_manage;
void init_timer_manage();
struct timer *timer_alloc(void);
void timer_free(struct timer *timer);
void timer_init(struct timer *timer, struct fifo32 *fifo, int data);
void timer_data(struct timer *timer, int data);
void timer_settime(struct timer *timer, unsigned int timeout);


#endif

