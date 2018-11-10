#include "kernel/ioqueue.h"

struct ioqueue* create_ioqueue()
{
	return (struct ioqueue* )kmalloc(IOQUEUE_SIZE);
}

void ioqueue_init(struct ioqueue* ioqueue)
{
	//初始化队列锁
	lock_init(&ioqueue->lock);
	//初始化缓冲区
	ioqueue->buf = (int32_t *)kmalloc(IO_QUEUE_BUF_LEN*4);
	
	memset(ioqueue->buf, 0, IO_QUEUE_BUF_LEN*4);
	ioqueue->in = ioqueue->out = 0;
	ioqueue->size = 0;
	
	ioqueue->task = NULL;
}

/*
往队列里放入数据，如果mode是移动，那么缓冲区获取数据后就可以移动，不然就不移动
*/
int ioqueue_get(struct ioqueue* ioqueue, int mode)
{
	//加上锁，一次只能有一个进程调用
	
	lock_acquire(&ioqueue->lock);

	int data = -1;
	//取出数据
	//注意，取出数据的时候，我们不能超过读取的位置
	if(ioqueue->size > 0){	//有数据我们才从队列中取出，没有就不操作
		data = ioqueue->buf[ioqueue->out];
		if(mode){
			ioqueue->out++;	//改变指针位置
			ioqueue->size--;	//数据数量减少
			//修复越界
			if(ioqueue->out >= IO_QUEUE_BUF_LEN){
				ioqueue->out = 0;
			}
		}
	}
	//释放锁
	lock_release(&ioqueue->lock);

	return data;
}

void ioqueue_put(struct ioqueue* ioqueue, int data)
{
	//加上锁，一次只能有一个进程调用
	lock_acquire(&ioqueue->lock);
	//放入数据
	//如果最大了就直接覆盖原有信息
	ioqueue->buf[ioqueue->in] = data;
	
	ioqueue->in++;	//改变指针位置
	ioqueue->size++;	//数据数量增加
	//修复越界
	if(ioqueue->in >= IO_QUEUE_BUF_LEN){
		ioqueue->in = 0;
	}
	//释放锁
	lock_release(&ioqueue->lock);
}

bool ioqueue_empty(struct ioqueue* ioqueue)
{
	return (ioqueue->size == 0) ? 1:0;	//如果大小为0就是空的
}

void ioqueue_bound(struct ioqueue* ioqueue, struct task *task)
{
	ioqueue->task = task;
}

