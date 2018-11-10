#include "kernel/lock.h"
#include "kernel/semaphore.h"
#include "kernel/task.h"
#include "kernel/io.h"

struct lock* create_lock()
{
	return kmalloc(LOCK_SIZE);
}
void lock_init(struct lock* lock)
{
   lock->holder = NULL;
   lock->holder_acqure_count = 0;
   sema_init(&lock->semaphore, 1);
}

void lock_acquire(struct lock* lock)
{
	if (lock->holder != task_current()) {	//如果不是自己，就可以申请
		sema_P(&lock->semaphore);	//改变信号量置
		lock->holder = task_current();	//改变锁的获取者
		lock->holder_acqure_count = 1;	//默认申请1次
	}else{	//自己不能再申请了，记录次数
		printk("\nAcquire again!\n");
		lock->holder_acqure_count++;
	}
}

void lock_release(struct lock* lock)
{
	if (lock->holder_acqure_count > 1) {	//如果申请多次，现在释放，减少申请次数，返回，知道只有1次的时候才可以释放
		lock->holder_acqure_count--;	
		return;
	}
	lock->holder = NULL;
	lock->holder_acqure_count = 0;
	sema_V(&lock->semaphore);	//改变信号量，使得信号可以用
}
