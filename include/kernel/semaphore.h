#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_
#include "kernel/types.h"

struct semaphore 
{
	uint8_t value;	//二元信号量，只有0和1
};

void sema_init(struct semaphore* sema, uint8_t value);	//初始化
void sema_P(struct semaphore* sema);	//P操作，dwon
void sema_V(struct semaphore* sema);	//V操作， up

#endif