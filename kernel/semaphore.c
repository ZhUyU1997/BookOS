#include "kernel/semaphore.h"
#include "kernel/task.h"
#include "kernel/io.h"

void sema_init(struct semaphore* sema, uint8_t value)
{
	sema->value = value;
}

void sema_P(struct semaphore* sema)
{
	while(sema->value == 0);	//ѭ���ȴ��������1�Ϳ���ʹ�ã�Ȼ�����ó�0
	sema->value--;
}

void sema_V(struct semaphore* sema)
{
	sema->value++;	//ʹ���źſ���
}

