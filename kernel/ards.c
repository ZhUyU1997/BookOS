#include "kernel/ards.h"
#include "kernel/memory.h"

struct ards *ards;
void init_ards()
{
	uint16_t ards_nr =  *((uint16_t *)ARDS_NR);	//ards 结构数
	ards = (struct ards *) ARDS_ADDR;	//ards 地址
	int i;
	for(i = 0; i < ards_nr; i++){
		//寻找可用最大内存
		if(ards->type == 1){
			//冒泡排序获得最大内存
			if(ards->base_low+ards->length_low > memory_total_size){
				memory_total_size = ards->base_low+ards->length_low;
			}
		}
		ards++;
	}
	printk("ARDS %x nr %d\n", ARDS_ADDR, ards_nr);

	printk("Memory size:%dMB\n", memory_total_size/(1024*1024));
	
}