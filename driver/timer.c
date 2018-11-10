#include "driver/timer.h"
#include "kernel/io.h"
#include "kernel/memory.h"


struct timer_manage *timer_manage;

void init_timer_manage()
{
	int i;
	struct timer *timer_idle;
	
	//timer_manage = (struct timer_manage *)kernel_alloc_page(DIV_ROUND_UP(sizeof(struct timer_manage ),PAGE_SIZE));
	timer_manage = (struct timer_manage *)kmalloc(sizeof(struct timer_manage ));
	
	timer_manage->count = 0;
	for (i = 0; i < MAX_TIMER; i++) {	
		timer_manage->timers_table[i].status = TIMER_UNUSED; 	/* 未使用 */
	}
	timer_idle = timer_alloc(); 			/* 创建一个新的定时器(哨兵) */
	timer_idle->timeout = TIMEROUT_MAX;	/* 因为该定时器始终保持在最后面, 所以超时时间设置为最大 */
	timer_idle->status = TIMER_USING;	
	timer_idle->next = 0; /* 它是最后一个,所以它的next域为0 */
	timer_manage->head = timer_idle; /* 因为现在只有哨兵,所以它是最前面的 */
	timer_manage->next = TIMEROUT_MAX; /* 因为只有哨兵,所以下一个超时时刻就是哨兵的时刻 */
}

/* 创建新的定时器并返回指向该定时器结构的指针 */
struct timer *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timer_manage->timers_table[i].status == 0) {	/* 找到第一个未使用的定时器位置 */
			timer_manage->timers_table[i].status = TIMER_ALLOC;	/* 设定其状态为已配置 */
			timer_manage->timers_table[i].fifo = NULL;
			timer_manage->timers_table[i].data = -1;
			timer_manage->timers_table[i].occured = 0;
			return &timer_manage->timers_table[i];	/* 返回这个结构的指针 */
		}
	}
	return 0;	/* 没找到就返回0 */
}

/* 释放timer所指向的定时器 */
void timer_free(struct timer *timer)
{
	timer->status = TIMER_UNUSED; /* 直接修改为未使用状态 */
	return;
}

/* 定时器结构的初始化 */
void timer_init(struct timer *timer, struct fifo32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_data(struct timer *timer, int data)
{
	timer->data = data;
	return;
}
/* 定时器的设定 */
/* 如果对该函数的算法不太明白的可以参考任意一本数据结构书籍中关于单链表的插入操作 */
void timer_settime(struct timer *timer, unsigned int timeout)
{
	int e;
	struct timer *front, *back;	//前一个和后一个定时器
	timer->timeout = timeout + timer_manage->count;
	timer->status = TIMER_USING;
	timer->occured = 0;
	e = io_load_eflags();
	io_cli();
	front = timer_manage->head;
	if (timer->timeout <= front->timeout) {
		/* 插入最前面的情况 */
		timer_manage->head = timer;
		timer->next = front; /* 下一个定时器是设定为t */
		timer_manage->next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* 搜索插入位置 */
	for (;;) {
		back = front;
		front = front->next;
		if (timer->timeout <= front->timeout) {
			/* 插入s个t之间的情况 */
			back->next = timer; /* s的下一个是timer */
			timer->next = front; /* timer的下一个是t */
			io_store_eflags(e);
			return;
		}
	}
}

