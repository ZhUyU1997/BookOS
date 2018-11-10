#include "kernel/semaphore.h"
#include "kernel/task.h"
#include "kernel/io.h"

void sema_init(struct semaphore* sema, uint8_t value)
{
	sema->value = value;
}

void sema_P(struct semaphore* sema)
{
	while(sema->value == 0);	//循环等待，如果是1就可以使用，然后设置成0
	sema->value--;
}

void sema_V(struct semaphore* sema)
{
	sema->value++;	//使得信号可用
}

