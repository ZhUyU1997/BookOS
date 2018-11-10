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
	if (lock->holder != task_current()) {	//��������Լ����Ϳ�������
		sema_P(&lock->semaphore);	//�ı��ź�����
		lock->holder = task_current();	//�ı����Ļ�ȡ��
		lock->holder_acqure_count = 1;	//Ĭ������1��
	}else{	//�Լ������������ˣ���¼����
		printk("\nAcquire again!\n");
		lock->holder_acqure_count++;
	}
}

void lock_release(struct lock* lock)
{
	if (lock->holder_acqure_count > 1) {	//��������Σ������ͷţ�����������������أ�֪��ֻ��1�ε�ʱ��ſ����ͷ�
		lock->holder_acqure_count--;	
		return;
	}
	lock->holder = NULL;
	lock->holder_acqure_count = 0;
	sema_V(&lock->semaphore);	//�ı��ź�����ʹ���źſ�����
}
