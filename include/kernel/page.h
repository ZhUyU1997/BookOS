#ifndef _PAGE_H_
#define _PAGE_H_

#include "kernel/types.h"
#include "kernel/task.h"

#define PAGE_SIZE 4096
//一个页的大小

//分页机制
#define PAGE_DIR_PHY_ADDR 0x1000
 //内核页目录表地址
#define PAGE_TBL_PHY_ADDR 0x2000
 //内核页表地址

#define PAGE_DIR_VIR_ADDR 0x80001000
//映射后的虚拟地址
#define PAGE_TBL_VIR_ADDR 0x80002000

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
 //过去页目录项的索引(存放了页表的地址和属性)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)
//过去页表项的索引(存放了页的地址和属性)
void init_page();

//分配
void *kernel_alloc_page(uint32_t pages);
//内核分配页
int alloc_mem_page();
//分配一个物理页
int alloc_vir_page();
 //分配一个虚拟页地址
//释放
void kernel_free_page(int vaddr, uint32_t pages);
//释放内核页
int free_mem_page(int address);
//释放物理页
int free_vir_page(int address);
//释放虚拟页地址

//填充页表
void fill_vir_page_talbe(int vir_address);
//把虚拟地址做页目录表，页表，页的映射
void clean_vir_page_table(int vir_address);
//清楚页目录表，页表，页的联系

//其它
int *copy_kernel_page();
//把内核的页表作为一个副本给其它任务使用

uint32_t *pde_ptr(uint32_t vaddr);
//页目录项指针
uint32_t *pte_ptr(uint32_t vaddr);
//页表项指针
uint32_t addr_v2p(uint32_t vaddr);
//把虚拟地址转换成物理地址
int pages_status();
//获取物理页的分配状态
int get_free_memory();
//过去剩余内存

//过去内核的页目录表
void *get_kernel_page();
//虚拟地址转换成物理地址
int vir2phy(int vir_address);
//复制内核的页目录
void *copy_kernel_pdt();

uint32_t* create_page_dir(void);

void vir_link_with_phy(uint32_t vir_addr, uint32_t phy_addr);
uint32_t fill_vir_page_talbe_directly(int vir_address);


int task_alloc_vir_page(struct task *task);
int task_free_vir_page(struct task *task,int address);
void *task_alloc_page(struct task *task, uint32_t pages);
void task_free_page(struct task *task, int vaddr, uint32_t pages);

#endif  // INCLUDE_TYPES_H_