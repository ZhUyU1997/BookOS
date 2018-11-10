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

//�ڴ�������ڴ��У�ҳ�з��䣬�����ں���ռ�ռ�
struct memory_manage *memory_manage;

static void memory_test();
 
void init_task_memory_manage(struct task *task);
 
void init_memory()
{
	int idx, i; 
	int *new_page;
	
	memory_total_size = 0;
	//��ʼ��ȡ
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
	/*Ϊ�ṹ�����ռ�*/
	memory_manage = (struct memory_manage *)kernel_alloc_page(memory_manage_pages);
	if(memory_manage == NULL){
		panic("memory manage failed!");
	}
	memset(memory_manage, 0, memory_manage_pages*PAGE_SIZE);
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		memory_manage->free_blocks[i].size = 0;	//��С��ҳ������
		memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void *kmalloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//Ҫ�����ʲô��С
	uint32_t break_cnt;//Ҫ����ɼ���
	void *new_address;
	
	//����1024�ֽھ���ҳ
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//һ��ռ���ٸ�ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = (uint32_t )kernel_alloc_page(pages);	//����ҳ
				memory_manage->free_blocks[i].address = address;	
				memory_manage->free_blocks[i].size = pages;	//��С��ҳ������
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		//�����жϣ�Ҫ��ɢ�ɶ��
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
		//��һ��Ѱ�ң�����ڿ���û���ҵ����ʹ�ɢһ��ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
		//�����û���ҵ�������һ��ҳ��Ȼ���ɢ
		//����һ��ҳ����������ɢ
		new_address = kernel_alloc_page(1);
		break_cnt = PAGE_SIZE/break_size;
		
		//��ɢ��break_cnt��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�һ�����Ա�ʹ�õ�
				//��ַ����
				
				//�������յ�ַ
				memory_manage->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//����size
				memory_manage->free_blocks[i].size = break_size;
				//����Ϊ���Է���
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//����ΪС��ģʽ
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//��ɢ���Ѱ��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}
	//size=0����û���ҵ�
	return NULL;	//ʧ��
}

int kfree(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(memory_manage->free_blocks[i].address == addr && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_USING){	//�ҵ�
			if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				kernel_free_page(memory_manage->free_blocks[i].address, memory_manage->free_blocks[i].size);
				memory_manage->free_blocks[i].size = 0;		//ֻ�д�����Ҫ��������size
			}else if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//С���ڴ����վ�����
				memset((void *)memory_manage->free_blocks[i].address, 0, memory_manage->free_blocks[i].size);
				//����һ��������Ǿ������б���ɢ���ڴ涼���ͷź󣬿�����Ҫ�ͷ��Ǹ�ҳ��Ŀǰ��û�п�����
				//С�鲻��Ҫ���ô�С����Ϊ���Ǵ�ɢ�˵Ŀ�
			}
			memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	
	return -1;	//ʧ��
}


void init_task_memory_manage(struct task *task)
{
	int i;
	uint32_t memory_manage_pages = DIV_ROUND_UP(sizeof(struct memory_manage),PAGE_SIZE);
	/*Ϊ�ṹ�����ռ�*/
	task->mm = (struct memory_manage *)kernel_alloc_page(memory_manage_pages);
	task->mm_live_pages = memory_manage_pages;
	
	if(task->mm == NULL){
		panic("task memory manage failed!");
	}
	
	memset(task->mm, 0, memory_manage_pages*PAGE_SIZE);
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		task->mm->free_blocks[i].size = 0;	//��С��ҳ������
		task->mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void *memory_malloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//Ҫ�����ʲô��С
	uint32_t break_cnt;//Ҫ����ɼ���
	void *new_address;
	struct task *task = task_current();
	struct memory_manage *task_mm = task->mm;

	//����1024�ֽھ���ҳ
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//һ��ռ���ٸ�ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = (uint32_t )task_alloc_page(task, pages);	//����ҳ
				task_mm->free_blocks[i].address = address;	
				task_mm->free_blocks[i].size = pages;	//��С��ҳ������
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				task_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("1 Alloc:%x idx:%d size:%d size:%d \n", address,i, task_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		
		//�����жϣ�Ҫ��ɢ�ɶ��
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
		
		//��һ��Ѱ�ң�����ڿ���û���ҵ����ʹ�ɢһ��ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(task_mm->free_blocks[i].size == break_size && task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = task_mm->free_blocks[i].address;
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("2 Alloc:%x idx:%d size:%d size:%d \n", address,i, task_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
		//�����û���ҵ�������һ��ҳ��Ȼ���ɢ
		//����һ��ҳ����������ɢ
		new_address = task_alloc_page(task, 1);
		break_cnt = PAGE_SIZE/break_size;
		//printk("*addr %x broken into %d parts\n", new_address, break_cnt);
		//��ɢ��break_cnt��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			
			if(task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�һ�����Ա�ʹ�õ�
				//��ַ����
				//�������յ�ַ
				task_mm->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//����size
				task_mm->free_blocks[i].size = break_size;
				//����Ϊ���Է���
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//����ΪС��ģʽ
				task_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//��ɢ���Ѱ��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(task_mm->free_blocks[i].size == break_size && task_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = task_mm->free_blocks[i].address;
				task_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("3 Alloc:%x idx:%d size:%d size:%d \n", address,i, task_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}
	//size=0����û���ҵ�
	return NULL;	//ʧ��
}

int memory_free(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	struct task *task = task_current();
	struct memory_manage *task_mm = task->mm;

	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(task_mm->free_blocks[i].address == addr && task_mm->free_blocks[i].flags == MEMORY_BLOCK_USING){	//�ҵ�
			if(task_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				task_free_page(task, task_mm->free_blocks[i].address, task_mm->free_blocks[i].size);
				task_mm->free_blocks[i].size = 0;		//ֻ�д�����Ҫ��������size
			}else if(task_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//С���ڴ����վ�����
				memset((void *)task_mm->free_blocks[i].address, 0, task_mm->free_blocks[i].size);
				//����һ��������Ǿ������б���ɢ���ڴ涼���ͷź󣬿�����Ҫ�ͷ��Ǹ�ҳ��Ŀǰ��û�п�����
				//С�鲻��Ҫ���ô�С����Ϊ���Ǵ�ɢ�˵Ŀ�
			}
			task_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	//printk("not found that address %x\n", address);
	return -1;	//ʧ��
}

