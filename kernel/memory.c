#include "kernel/memory.h"
#include "kernel/ards.h"
#include "kernel/types.h"
#include "driver/vga.h"
#include "kernel/8259a.h"
#include "kernel/task.h"
#include "kernel/message.h"
#include "kernel/debug.h"
#include "kernel/page.h"
#include "lib/bitmap.h"
#include "lib/string.h"
#include "lib/math.h"

int memory_total_size;
int memory_used_size;
extern struct bitmap phy_mem_bitmap;
extern struct bitmap vir_mem_bitmap;

//内存管理在内存中，页中分配，不在内核中占空间
struct memory_manage *memory_manage;

static void memory_test();
 
void init_task_memory_manage(struct task *task);
 
void init_memory()
{
	int idx, i; 
	int *new_page;
	
	memory_total_size = 0;
	//开始获取
	init_ards();
	
	int pages = memory_total_size/PAGE_SIZE;
	int page_byte = pages/8;
	
	phy_mem_bitmap.bits = (uint8_t*)PHY_MEM_BITMAP;
	phy_mem_bitmap.btmp_bytes_len = page_byte;
	
	vir_mem_bitmap.bits = (uint8_t*)VIR_MEM_BITMAP;
	vir_mem_bitmap.btmp_bytes_len = page_byte;
	
	bitmap_init(&phy_mem_bitmap);
	bitmap_init(&vir_mem_bitmap);
	//clean low 2G
	memset((void *)(PAGE_DIR_VIR_ADDR+4),0,2044);
	
	printk("~Init memory done.\n");

}

static void memory_test()
{
	uint32_t *a;
	a = kmalloc(32);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(32);
	printk("%x\n", a);
	a = kmalloc(1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(1024);
	printk("%x\n", a);
	a = kmalloc(4096);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(4096);
	printk("%x\n", a);
	a = kmalloc(1024*1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(1024*1024);
	printk("%x\n", a);
	a = kmalloc(10*1024*1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(10*1024*1024);
	printk("%x\n", a);
}

void init_memory_manage(void )
{
	int i;
	uint32_t memory_manage_pages = DIV_ROUND_UP(sizeof(struct memory_manage),PAGE_SIZE);
	/*为结构体分配空间*/
	memory_manage = (struct memory_manage *)kernel_alloc_page(memory_manage_pages);
	if(memory_manage == NULL){
		panic("memory manage failed!");
	}
	memset(memory_manage, 0, memory_manage_pages*PAGE_SIZE);
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		memory_manage->free_blocks[i].size = 0;	//大小是页的数量
		memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void *kmalloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//要打碎成什么大小
	uint32_t break_cnt;//要打碎成几块
	void *new_address;
	
	//大于1024字节就用页
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//一共占多少个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = (uint32_t )kernel_alloc_page(pages);	//分配页
				memory_manage->free_blocks[i].address = address;	
				memory_manage->free_blocks[i].size = pages;	//大小是页的数量
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		//对齐判断，要打散成多大
		if(0 < size && size <= 32){
			break_size = 32;
		}else if(32 < size && size <= 64){
			break_size = 64;
		}else if(64 < size && size <= 128){
			break_size = 128;
		}else if(128 < size && size <= 256){
			break_size = 256;
		}else if(256 < size && size <= 512){
			break_size = 512;
		}else if(512 < size && size <= 1024){
			break_size = 1024;
		}else if(1024 < size && size <= 2048){
			break_size = 2048;
		}
		//第一次寻找，如果在块中没有找到，就打散一个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
		//如果都没有找到，分配一个页，然后打散
		//分配一个页，用来被打散
		new_address = kernel_alloc_page(1);
		break_cnt = PAGE_SIZE/break_size;
		
		//打散成break_cnt个
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到一个可以被使用的
				//地址增加
				
				//设置最终地址
				memory_manage->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//设置size
				memory_manage->free_blocks[i].size = break_size;
				//设置为可以分配
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//设置为小块模式
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//打散后的寻找
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}
	//size=0或者没有找到
	return NULL;	//失败
}

int kfree(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(memory_manage->free_blocks[i].address == addr && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_USING){	//找到
			if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				kernel_free_page(memory_manage->free_blocks[i].address, memory_manage->free_blocks[i].size);
				memory_manage->free_blocks[i].size = 0;		//只有大块才需要重新设置size
			}else if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//小块内存就清空就是了
				memset((void *)memory_manage->free_blocks[i].address, 0, memory_manage->free_blocks[i].size);
				//存在一种情况，那就是所有被打散的内存都被释放后，可能需要释放那个页，目前还没有考虑它
				//小块不需要设置大小，因为就是打散了的块
			}
			memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	
	return -1;	//失败
}


void init_task_memory_manage(struct task *task)
{
	int i;
	uint32_t memory_manage_pages = DIV_ROUND_UP(sizeof(struct memory_manage),PAGE_SIZE);
	/*为结构体分配空间*/
	task->mm = (struct memory_manage *)kernel_alloc_page(memory_manage_pages);
	task->mm_live_pages = memory_manage_pages;
	
	if(task->mm == NULL){
		panic("task memory manage failed!");
	}
	
	memset(task->mm, 0, memory_manage_pages*PAGE_SIZE);
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		task->mm->free_blocks[i].size = 0;	//大小是页的数量
		task->mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void *memory_malloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//要打碎成什么大小
	uint32_t break_cnt;//要打碎成几块
	void *new_address;
	struct task *task = task_current();
	struct memory_manage *task_mm = task->mm;

	//大于1024字节就用页
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//一共占多少个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = (uint32_t )task_alloc_page(task, pages);	//分配页
				task_mm->free_blocks[i].address = address;	
				task_mm->free_blocks[i].size = pages;	//大小是页的数量
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				task_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("1 Alloc:%x idx:%d size:%d size:%d \n", address,i, task_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		
		//对齐判断，要打散成多大
		if(0 < size && size <= 32){
			break_size = 32;
		}else if(32 < size && size <= 64){
			break_size = 64;
		}else if(64 < size && size <= 128){
			break_size = 128;
		}else if(128 < size && size <= 256){
			break_size = 256;
		}else if(256 < size && size <= 512){
			break_size = 512;
		}else if(512 < size && size <= 1024){
			break_size = 1024;
		}else if(1024 < size && size <= 2048){
			break_size = 2048;
		}
		
		//第一次寻找，如果在块中没有找到，就打散一个页
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(task_mm->free_blocks[i].size == break_size && task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = task_mm->free_blocks[i].address;
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("2 Alloc:%x idx:%d size:%d size:%d \n", address,i, task_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
		//如果都没有找到，分配一个页，然后打散
		//分配一个页，用来被打散
		new_address = task_alloc_page(task, 1);
		break_cnt = PAGE_SIZE/break_size;
		//printk("*addr %x broken into %d parts\n", new_address, break_cnt);
		//打散成break_cnt个
		for(i = 0; i < MEMORY_BLOCKS; i++){
			
			if(task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到一个可以被使用的
				//地址增加
				//设置最终地址
				task_mm->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//设置size
				task_mm->free_blocks[i].size = break_size;
				//设置为可以分配
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//设置为小块模式
				task_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//打散后的寻找
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(task_mm->free_blocks[i].size == break_size && task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//找到
				address = task_mm->free_blocks[i].address;
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("3 Alloc:%x idx:%d size:%d size:%d \n", address,i, task_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}
	//size=0或者没有找到
	return NULL;	//失败
}

int memory_free(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	struct task *task = task_current();
	struct memory_manage *task_mm = task->mm;

	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(task_mm->free_blocks[i].address == addr && task_mm->free_blocks[i].flags == MEMORY_BLOCK_USING){	//找到
			if(task_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				task_free_page(task, task_mm->free_blocks[i].address, task_mm->free_blocks[i].size);
				task_mm->free_blocks[i].size = 0;		//只有大块才需要重新设置size
			}else if(task_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//小块内存就清空就是了
				memset((void *)task_mm->free_blocks[i].address, 0, task_mm->free_blocks[i].size);
				//存在一种情况，那就是所有被打散的内存都被释放后，可能需要释放那个页，目前还没有考虑它
				//小块不需要设置大小，因为就是打散了的块
			}
			task_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	//printk("not found that address %x\n", address);
	return -1;	//失败
}

