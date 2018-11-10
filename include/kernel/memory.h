#ifndef _MEMORY_H
#define _MEMORY_H

#include "kernel/types.h"

#define KB  1024
#define MB  (1024*KB)

//内存
//分配内存起始地址为4M以上
#define PHY_MEM_BASE_ADDR  0x400000 //物理内存分配起始地址
#define VIR_MEM_BASE_ADDR  0x80400000//内核虚拟内存分配起始地址

//内存管理的位图位于1M以上
#define PHY_MEM_BITMAP  0x801a0000 //物理内存位图管理的数据结构起始地址
#define VIR_MEM_BITMAP  0x801c0000
//大小为4GB对应的位图大小
#define PHY_MEM_BITMAP_SIZE 0x20000 //物理内存位图大小(4g)
#define VIR_MEM_BITMAP_SIZE 0x10000//内核位图大小(2g)

//user vir addr base
#define USER_VIR_MEM_BASE_ADDR  0x00400000

 //2m-4m当做空闲空间
#define FREE_BUFFER_ADDR  0x80200000

extern int memory_total_size;//全局变量物理内存大小

//初始化总内存管理
void init_memory();

void kernel_memroy_map(int phy, int vir, int pages);//把物理地址和虚拟地址做映射，一般位于内核中

#define MEMORY_BLOCKS 0x1000 /*最大的空闲内存块数量
								每个具体信息16字节	
								*/

#define MEMORY_BLOCK_FREE 0 //内存信息块空闲
#define MEMORY_BLOCK_USING 1//内存信息块使用中

#define MEMORY_BLOCK_MODE_SMALL 0 //小块内存描述1024一下的内存块
#define MEMORY_BLOCK_MODE_BIG 1 //大块内存描述4kb为单位的内存块

//内存块储存内存的具体信息
struct memory_block 
{
	uint32_t address; //内存块分配地址
	uint32_t size;//发现
	uint32_t flags;//内存块状态
	uint32_t mode;//内存块模式
};

/*
	为了让memory_manage结构体占用整数，我们在memory_manage中增加16字节的
	成员，就让MEMORY_BLOCKS减1，这样使结构体对齐更好，分配内存更方便
*/
struct memory_manage
{
	struct memory_block free_blocks[MEMORY_BLOCKS];
};
extern struct memory_manage *memory_manage;
/*
内核的内存管理
*/
void init_memory_manage(void );	//初始化内存管理


struct memory_block *get_memory_block(void *address);	//通过地址获得对应的内存块
/*
for syscall
*/
void *kmalloc(uint32_t size);	//内核的分配
int kfree(void *address);		//内核的释放


void *memory_malloc(uint32_t size);
int memory_free(void *address);

#endif

