#include "kernel/page.h"
#include "kernel/memory.h"
#include "kernel/task.h"
#include "lib/bitmap.h"
#include "lib/math.h"

struct bitmap phy_mem_bitmap;
struct bitmap vir_mem_bitmap;
extern struct task *process_init;

void init_page()
{
	u32 *pdt = (u32 *)PAGE_DIR_PHY_ADDR;
	memset(PAGE_DIR_PHY_ADDR,0,PAGE_SIZE);	//Clean pdt
	//Set 0 pde and 512 pde for pt
	pdt[0] = PAGE_TBL_PHY_ADDR|0x07;
	//pdt[512] = PAGE_TBL_PHY_ADDR|0x07;
	//Set myself in 1023 pde
	pdt[1023] = PAGE_DIR_PHY_ADDR|0x07;
	
	int i;
	u32 *pt = (u32 *)PAGE_TBL_PHY_ADDR;
	u32 page_addr = 0x00|0x07;
	//Set low 4M physical address to pt
	for(i = 0; i < 1024; i++){
		pt[i] = page_addr;
		page_addr += PAGE_SIZE;
	}
	
	//Set page dir to cr3
	write_cr3(PAGE_DIR_PHY_ADDR);
	//Read cr0
	u32 cr0 = read_cr0();
	cr0 |= 0x80000000;
	//Write cr0
	write_cr0(cr0);
	
}

/**检测页的分配状况*/
int pages_status()
{
	int i , j = 0;
	for(i = 0; i < memory_total_size/PAGE_SIZE; i++){
		if(bitmap_scan_test(&phy_mem_bitmap, i) == 0)j++;
	}
	return j;
}

/**返回没有使用的内存数*/
int get_free_memory()
{
	int pages = pages_status();
	int free_size = pages*PAGE_SIZE;
	return free_size;
}

/*
把虚拟地址转换成物理地址
*/
uint32_t addr_v2p(uint32_t vaddr)
{
	//print_vai(vaddr);
	uint32_t *pte = pte_ptr(vaddr);
	uint32_t idx_in_page = vaddr & 0x00000fff;
	uint32_t phy_addr = *pte + idx_in_page;	//页表中的数据（物理页）+ 页中的偏移-》存放的是物理地址，但还有它的属性，所以你懂得！！
	phy_addr = phy_addr&0xfffff000;	//去除属性
	//printk(" %x->%x\n",vaddr, phy_addr);
	return phy_addr;
}

void vir_link_with_phy(uint32_t vir_addr, uint32_t phy_addr)
{
	uint32_t *pde, *pte;
	pde = pde_ptr(vir_addr);
	if(((*pde)&0x00000001) != 0x00000001){	//不存在页表
		uint32_t pt = alloc_mem_page();	//分配页表地址
		pt |= 0x00000007;
		*pde = pt;		//填写页目录项为页表的地址
	}
	pte = pte_ptr(vir_addr);
	phy_addr |= 0x00000007;
	*pte = phy_addr;	//填写页表项为页的地址
}

void *copy_kernel_pdt()
{
	void *vir_kernel_pdt_addr = kernel_alloc_page(1);	//申请
	memcpy((void *)(vir_kernel_pdt_addr), (void *)(PAGE_DIR_VIR_ADDR), 4096);	//复制
	
	//memset(vir_kernel_pdt_addr , 0 , 2048);	//复制
	return vir_kernel_pdt_addr;	//返回
}

uint32_t *pte_ptr(uint32_t vaddr)
{
	uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
	return pte;
}

uint32_t *pde_ptr(uint32_t vaddr)
{
	uint32_t *pde = (uint32_t *)((0xfffff000) + PDE_IDX(vaddr) * 4);
	return pde;
}


void fill_vir_page_talbe(int vir_address)
{
	uint32_t *pde, *pte;
	pde = pde_ptr(vir_address);
	if(((*pde)&0x00000001) != 0x00000001){	//不存在页表
		uint32_t pt = alloc_mem_page();	//分配页表地址
		pt |= 0x00000007;
		*pde = pt;		//填写页目录项为页表的地址
	}
	pte = pte_ptr(vir_address);
	if(((*pte)&0x00000001) != 0x00000001){	//不存在页表
		uint32_t page = alloc_mem_page();	//分配页地址
		page |= 0x00000007;
		*pte = page;	//填写页表项为页的地址
	}
}

uint32_t fill_vir_page_talbe_directly(int vir_address)
{
	uint32_t *pde, *pte;
	pde = pde_ptr(vir_address);
	//分配页表地址
	uint32_t pt = alloc_mem_page();
	pt |= 0x00000007;
	*pde = pt;		//填写页目录项为页表的地址
	
	pte = pte_ptr(vir_address);
	//分配页地址
	uint32_t page = alloc_mem_page();	
	page |= 0x00000007;
	*pte = page;	//填写页表项为页的地址
	return pt;
}

void clean_vir_page_table(int vir_address)
{
	uint32_t *pde, *pte;
	uint32_t page_phy_addr;
	pde = pde_ptr(vir_address);
	if(((*pde)&0x00000001) != 0x00000001){	//不存在页表,直接产生页故障
		panic("clean vir page failed.");
	}
	pte = pte_ptr(vir_address);
	if(((*pte)&0x00000001) != 0x00000001){	//不存在页表,直接产生页故障
		panic("clean vir page failed.");
	}
	page_phy_addr = *pte;	//获得页表项中页的物理地址
	*pte = 0;	//清空页表项
	page_phy_addr &= 0xfffff000;	//保留高22位，对齐
	free_mem_page(page_phy_addr);	//释放对应的物理页
}

int alloc_mem_page()
{
	int idx;
	int mem_addr;
	idx = bitmap_scan(&phy_mem_bitmap, 1);
	if(idx != -1){
		bitmap_set(&phy_mem_bitmap, idx, 1);
	}else{
		return -1;
	}
	mem_addr = idx*0x1000 + PHY_MEM_BASE_ADDR;

	return mem_addr;
}

 int free_mem_page(int address)
{
	int addr = address;
	int idx, i;

	idx = (addr-PHY_MEM_BASE_ADDR)/0x1000;
	if(bitmap_scan_test(&phy_mem_bitmap, idx)){
		bitmap_set(&phy_mem_bitmap, idx, 0);
	}else{
		return -1;
	}
	return 0;
}

uint32_t* create_page_dir(void)
{
	uint32_t* page_dir_vaddr = (uint32_t* )kmalloc(PAGE_SIZE);
	if (page_dir_vaddr == NULL) {
		printk("create_page_dir: kmalloc failed!\n");
		return NULL;
	}
	//memcpy((void *)(page_dir_vaddr), (void *)(PAGE_DIR_VIR_ADDR), 4096);	//复制
	page_dir_vaddr[1023] = (uint32_t)(addr_v2p((int )page_dir_vaddr)+0x07);	//物理地址+属性
   return page_dir_vaddr;
}

//接下来是和虚拟地址相关的
int alloc_vir_page()
{
	int idx;
	int vir_addr;
	idx = bitmap_scan(&vir_mem_bitmap, 1);
	if(idx != -1){
		bitmap_set(&vir_mem_bitmap, idx, 1);
	}else{
		return -1;
	}
	vir_addr = idx*0x1000 +  VIR_MEM_BASE_ADDR;
	return vir_addr;
}

int free_vir_page(int address)
{
	int addr = address;
	int idx, i;

	idx = (addr-VIR_MEM_BASE_ADDR)/0x1000;

	if(bitmap_scan_test(&vir_mem_bitmap, idx)){
		bitmap_set(&vir_mem_bitmap, idx, 0);
	}else{
		return -1;
	}
	return 0;
}

void *kernel_alloc_page(uint32_t pages)
{
	int i;
	int vir_page_addr, vir_page_addr_more;

	vir_page_addr = alloc_vir_page();	//分配一个虚拟地址的页
	
	fill_vir_page_talbe(vir_page_addr);		//把页添加到当前页目录表系统中，使他可以被使用
	
	if(pages == 1){	//如果只有一个页
		memset((void *)vir_page_addr,0,PAGE_SIZE);
		return (void *)vir_page_addr;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr_more = alloc_vir_page();	//分配一个虚拟地址的页
			fill_vir_page_talbe(vir_page_addr_more);		//把页添加到当前页目录表系统中，使他可以被使用	
		}
		memset((void *)vir_page_addr,0,PAGE_SIZE*pages);
		return (void *)vir_page_addr;
	}else{
		return NULL;
	}
	//应该不会到这里来
	return NULL;
}

void kernel_free_page(int vaddr, uint32_t pages)
{
	int i;
	int vir_page_addr = vaddr;
	
	free_vir_page(vir_page_addr);
	clean_vir_page_table(vir_page_addr);		//把页添加到当前页目录表系统中，使他可以被使用
	if(pages == 1){	//如果只有一个页
		return;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr += PAGE_SIZE;
			free_vir_page(vir_page_addr);
			clean_vir_page_table(vir_page_addr);		//把页添加到当前页目录表系统中，使他可以被使用
		}
		return;
	}
}

//我们根据内核的来编写用户态的
int task_alloc_vir_page(struct task *task)
{
	int idx;
	int vir_addr;
	idx = bitmap_scan(&task->vir_mem_bitmap, 1);
	if(idx != -1){
		bitmap_set(&task->vir_mem_bitmap, idx, 1);
	}else{
		return -1;
	}
	vir_addr = idx*0x1000 +  USER_VIR_MEM_BASE_ADDR;
	return vir_addr;
}

int task_free_vir_page(struct task *task,int address)
{
	int addr = address;
	int idx, i;

	idx = (addr-USER_VIR_MEM_BASE_ADDR)/0x1000;

	if(bitmap_scan_test(&task->vir_mem_bitmap, idx)){
		bitmap_set(&task->vir_mem_bitmap, idx, 0);
	}else{
		return -1;
	}
	return 0;
}

void *task_alloc_page(struct task *task, uint32_t pages)
{
	int i;
	int vir_page_addr, vir_page_addr_more;

	vir_page_addr = task_alloc_vir_page(task);	//分配一个虚拟地址的页
	
	fill_vir_page_talbe(vir_page_addr);		//把页添加到当前页目录表系统中，使他可以被使用
	
	if(pages == 1){	//如果只有一个页
		memset((void *)vir_page_addr,0,PAGE_SIZE);
		return (void *)vir_page_addr;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr_more = task_alloc_vir_page(task);	//分配一个虚拟地址的页
			fill_vir_page_talbe(vir_page_addr_more);		//把页添加到当前页目录表系统中，使他可以被使用	
		}
		memset((void *)vir_page_addr,0,PAGE_SIZE*pages);
		return (void *)vir_page_addr;
	}else{
		return NULL;
	}
	//应该不会到这里来
	return NULL;
}

void task_free_page(struct task *task, int vaddr, uint32_t pages)
{
	int i;
	int vir_page_addr = vaddr;
	task_free_vir_page(task, vir_page_addr);
	clean_vir_page_table(vir_page_addr);		//把页添加到当前页目录表系统中，使他可以被使用
	if(pages == 1){	//如果只有一个页
		return;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr += PAGE_SIZE;
			task_free_vir_page(task, vir_page_addr);
			clean_vir_page_table(vir_page_addr);		//把页添加到当前页目录表系统中，使他可以被使用
		}
		return;
	}
}
