#ifndef _LOCK_H_
#define _LOCK_H_
#include "kernel/types.h"
#include "kernel/semaphore.h"

struct lock 
{
   struct   task* holder;	//哪个进程持有这个锁
   struct   semaphore semaphore;	//信号量
   uint32_t holder_acqure_count;	//自己重复申请
};

#define LOCK_SIZE sizeof(struct lock )

struct lock* create_lock();
void lock_init(struct lock* lock);	//初始化锁
void lock_acquire(struct lock* lock);	//获取锁
void lock_release(struct lock* lock);	//释放锁

#endif