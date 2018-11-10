#include "kernel/types.h"
#include "driver/vga.h"
#include "kernel/memory.h"  
#include "kernel/task.h"
#include "kernel/descriptor.h"
#include "driver/mouse.h"
#include "driver/keyboard.h"
#include "driver/console.h"
#include "driver/clock.h"
#include "driver/hd.h"
#include "gui/layer.h"
#include "gui/desktop.h"
#include "fs/fatxe.h"
#include "kernel/message.h"
#include "driver/hd.h"
#include "kernel/page.h"
#include "kernel/kernel.h"
#include "lib/elf.h"
#include "kernel/mailbox.h"
#include "fs/fatxe.h"
#include "fs/fat.h"
#include "fs/dir.h"
#include "fs/file.h"
#include "kernel/debug.h"
#include "lib/math.h"
#include "lib/stdarg.h"

struct task_manage *task_manage;

struct task *task_ready, *task_idle; //添加一个闲置任务，用于任务结束后跳转到此

extern struct layer *layer_desktop, *layer_mouse, *layer_console, *layer_taskbar;

struct task *task_ready_list;

/*
休眠和唤醒测试成功
关闭任务也成功
*/
static bool segment_load(int32_t fd, uint32_t offset, uint32_t filesz, uint32_t vaddr, uint32_t *pages);

struct service_process service_table[DRIVER_NR];

void init_tasks()
{
	int i;
	
	init_tss();
	
	task_manage = (struct task_manage *)kmalloc(sizeof(struct task_manage));
	task_ready_list = NULL;
	memset(task_manage,0,sizeof(struct task_manage));
	for(i = 0; i < MAX_TASKS; i++){
		task_manage->tasks_table[i].status = TASK_UNUSED;
		task_manage->tasks_table[i].pid = 0;	//释放任务时要设置pid
	}
	//clean driver table
	memset(&service_table[0],0,sizeof(struct service_process) * DRIVER_NR);
	
	struct task *_init;

	_init = task_alloc(TASK_TYPE_DRIVER);
	//make image
	task_init_bitmap(_init);
	task_init(_init, (int )&init, "init");
	task_run(_init, 3);

	task_ready = _init;
	update_tss_cr3(task_ready);
	intr_exit_clock();
}

bool registe_service(char *name, int32_t entry_point)
{
	int i;
	for(i = 0; i < DRIVER_NR; i++){
		if(service_table[i].entry_point == 0){	//free
			break;
		}
	}
	if(i == DRIVER_NR){
		return false;
	}
	
	strcpy(service_table[i].name, name);
	
	service_table[i].name[strlen(name)] = 0;
	
	service_table[i].entry_point = entry_point;

	return true;
}

int32_t get_service_entry_point(char *name)
{
	int i;
	for(i = 0; i < DRIVER_NR; i++){
		if(strcmp(service_table[i].name, name) == 0){	//free
			return service_table[i].entry_point;
		}
	}
	panic("task: get driver entry point error!");
	return -1;	//error
}

int create_service_process(char *filename)
{
	struct task *parent_task = task_current();	//get my self
	
	//1.ready a task
	struct task *task = task_alloc(TASK_TYPE_DRIVER);
	
	int32_t entry_point = get_service_entry_point(filename);     
	if (entry_point == -1) {	 // 若加载失败则返回-1
		return -1;
	}

	//init task
	task_init(task, (int )entry_point, filename);
	
	task_run(task, PRIORITY_LEVEL_II);

	return task->pid;	//we let parent known it's child's id
}

static bool elf_segment_load(int32_t fd, uint32_t offset, uint32_t filesz, uint32_t vaddr, uint32_t *pages) 
{
   uint32_t vaddr_first_page = vaddr & 0xfffff000;    // vaddr地址所在的页框
   uint32_t size_in_first_page = PAGE_SIZE - (vaddr & 0x00000fff);     // 加载到内存后,文件在第一个页框中占用的字节大小
   uint32_t occupy_pages = 0;
   /* 若一个页框容不下该段 */
   if (filesz > size_in_first_page) {
      uint32_t left_size = filesz - size_in_first_page;
      occupy_pages = DIV_ROUND_UP(left_size, PAGE_SIZE) + 1;	     // 1是指vaddr_first_page
   } else {
      occupy_pages = 1;
	}
	*pages = occupy_pages;
	
	/* 为进程分配内存 */
	uint32_t page_idx = 0;
	uint32_t vaddr_page = vaddr_first_page;
	while (page_idx < occupy_pages) {
      uint32_t* pde = pde_ptr(vaddr_page);
      uint32_t* pte = pte_ptr(vaddr_page);

      /* 如果pde不存在,或者pte不存在就分配内存.
       * pde的判断要在pte之前,否则pde若不存在会导致
       * 判断pte时缺页异常 */
		if (!(*pde & 0x00000001) || !(*pte & 0x00000001)) {
			fill_vir_page_talbe(vaddr_page);
		} // 如果原进程的页表已经分配了,利用现有的物理页,直接覆盖进程体
		vaddr_page += PAGE_SIZE;
		page_idx++;
	}
	sys_lseek(fd, offset, SEEK_SET);
	//从磁盘读取数据进内存，在当前任务中
	sys_read(fd, (void*)vaddr, filesz);
   return true;
}

/* 从文件系统上加载用户程序pathname,成功则返回程序的起始地址,否则返回-1 */
int32_t load_elf(char* pathname, uint32_t *pages)
{
	int32_t ret = -1;
	struct Elf32_Ehdr elf_header;
	struct Elf32_Phdr prog_header;
	memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));

	int32_t fd = sys_open(pathname, O_RDONLY);
	if (fd == -1) {
		printk("open %s faild!\n",pathname);
		return -1;
	}

	if (sys_read(fd, &elf_header, sizeof(struct Elf32_Ehdr)) != sizeof(struct Elf32_Ehdr)) {
		ret = -1;
		printk("read elf head faild!\n");
		goto done;
	}

	/* 校验elf头 */
	if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) \
		|| elf_header.e_type != 2 \
		|| elf_header.e_machine != 3 \
		|| elf_header.e_version != 1 \
		|| elf_header.e_phnum > 1024 \
		|| elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) 
	{
		ret = -1;
		printk("check elf head faild!\n");
		goto done;
	}

	Elf32_Off prog_header_offset = elf_header.e_phoff; 
	Elf32_Half prog_header_size = elf_header.e_phentsize;

	/* 遍历所有程序头 */
	uint32_t prog_idx = 0;
	uint32_t pg = 0;	//we need max size 
	
	
	while (prog_idx < elf_header.e_phnum) {
		memset(&prog_header, 0, prog_header_size);
      
		/* 将文件的指针定位到程序头 */
		sys_lseek(fd, prog_header_offset, SEEK_SET);

		/* 只获取程序头 */
		if (sys_read(fd, &prog_header, prog_header_size) != prog_header_size) {
			ret = -1;
			printk("read porgram head faild!\n");
			goto done;
		}

		/* 如果是可加载段就调用segment_load加载到内存 */
		if (PT_LOAD == prog_header.p_type) {
			printk("program header off %x size %x va %x\n", prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr);
			if (!elf_segment_load(fd, prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr, &pg)) {
				ret = -1;
				printk("load segment faild!\n");
				goto done;
			}
			if(pg > *pages){	//we will get max 
				*pages = pg;
			}
		}
		/* 更新下一个程序头的偏移 */
		prog_header_offset += elf_header.e_phentsize;
		prog_idx++;
	}
	ret = elf_header.e_entry;
done:
	sys_close(fd);
	return ret;
}

/* 从文件系统上加载用户程序pathname,成功则返回程序的起始地址,否则返回-1 */
int32_t load_bin(char* pathname, uint32_t *pages)
{
	int32_t ret = -1;
	int32_t fd = sys_open(pathname, O_RDONLY);
	if (fd == -1) {
		printk("open %s faild!\n",pathname);
		return -1;
	}
	
	struct file *file = get_file_though_fd(fd);
	
	uint32_t occupy_pages = (file->fd_dir->size - 1)/PAGE_SIZE + 1;
	*pages = occupy_pages;
	
	/* 为进程分配内存 */
	uint32_t page_idx = 0;
	uint32_t vaddr_page = TASK_USER_START_ADDR;
	while (page_idx < occupy_pages) {
      uint32_t* pde = pde_ptr(vaddr_page);
      uint32_t* pte = pte_ptr(vaddr_page);
      /* 如果pde不存在,或者pte不存在就分配内存.
       * pde的判断要在pte之前,否则pde若不存在会导致
       * 判断pte时缺页异常 */
		if (!(*pde & 0x00000001) || !(*pte & 0x00000001)) {
			fill_vir_page_talbe(vaddr_page);
		} // 如果原进程的页表已经分配了,利用现有的物理页,直接覆盖进程体
		vaddr_page += PAGE_SIZE;
		page_idx++;
	}
	
	sys_lseek(fd, 0, SEEK_SET);
	vaddr_page = TASK_USER_START_ADDR;
	sys_read(fd, (void*)vaddr_page, file->fd_dir->size);
	sys_close(fd);
	ret = TASK_USER_START_ADDR;
	return ret;
}

struct task *task_alloc(char type)
{
	int i;
	struct task *task;

	//make struct
	for(i = 0; i < MAX_TASKS; i++){
		if(task_manage->tasks_table[i].status == TASK_UNUSED){
			task = &task_manage->tasks_table[i];
			//task->status = TASK_READY;
			task->type = type;
			if(task->type == TASK_TYPE_DRIVER){
				task->regs.cs = DRIVER_CODE_SEL;
				task->regs.ds = DRIVER_DATA_SEL;
				task->regs.es = DRIVER_DATA_SEL;
				task->regs.fs = DRIVER_DATA_SEL;
				task->regs.gs = DRIVER_DATA_SEL;
				task->regs.ss = DRIVER_DATA_SEL;
				task->regs.eflags = EFLAGS_IOPL_1 | EFLAGS_MBS | EFLAGS_IF_1;	//支持IO
				
				task->ptr_pdir = (int *)copy_kernel_pdt();
				
				task->ptr_pdir[1023] = (uint32_t)(addr_v2p((int )task->ptr_pdir)+0x07);	//物理地址+属性
				task->cr3 = (uint32_t)addr_v2p((uint32_t)task->ptr_pdir);	//驱动就直接使用内核的页目录表
			}else if(task->type == TASK_TYPE_USER){
				task->regs.cs = USER_CODE_SEL;
				task->regs.ds = USER_DATA_SEL;
				task->regs.es = USER_DATA_SEL;
				task->regs.fs = USER_DATA_SEL;
				task->regs.gs = USER_DATA_SEL;
				task->regs.ss = USER_DATA_SEL;
				task->regs.eflags = EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1;	//不支持IO
				
				//make pdir
				task->ptr_pdir = (int *)copy_kernel_pdt();
				
				task->ptr_pdir[1023] = (uint32_t)(addr_v2p((int )task->ptr_pdir)+0x07);	//物理地址+属性
				task->cr3 = (uint32_t)addr_v2p((uint32_t)task->ptr_pdir);	//驱动就直接使用内核的页目录表
			}
			break;
			
		}
	}
	task->pid = i;
	task->ioqueue = NULL;
	task->console = NULL;
	task->vc = NULL;
	
	return task;
}

void task_free(struct task *task)
{
	int i;
	for(i = 0; i < MAX_TASKS; i++){
		//找到一样的
		if(&task_manage->tasks_table[i] == task){
			//释放占用的页目录表
			kernel_free_page((int)task->ptr_pdir, 1);
			//设置为没有使用
			task->status = TASK_UNUSED;
		}
	}
}

void task_run(struct task *task, int priority)
{
	uint32_t eflags = io_load_eflags();
	io_cli();
	task->status = TASK_READY;
	task->priority = priority;
	task->ticks = task->priority;
	task->run_ticks = 0;
	if(task_ready_list == NULL){
		task_ready_list = task;	//指向第一个
		task_ready_list->next = NULL;
	}else{
		struct task *p = task_ready_list;
		while(p->next != NULL){
			p = p->next;
		}
		p->next = task;
		task->next = NULL;
	}
	io_store_eflags(eflags);
}

void task_switch()
{
	uint32_t eflags = io_load_eflags();
	io_cli();
	
	struct task *prev, *next;
		do{
			//切换
			prev = task_ready;
			next = prev->next;
			task_ready = next;
			if(task_ready == NULL){
				task_ready = task_ready_list;	
			}
			//debug_printf("%dto%d\n", prev->pid, task_ready->pid);
		}while(task_ready->status == TASK_SLEEP);	//如果是休眠的就跳过
		while(task_ready->status == TASK_EXIT){	//如果是退出，那就去删除它
			prev = task_ready;
			next = task_ready->next;
			if(next == NULL){
				next = task_ready_list;	
			}
			//让下一个任务来击杀上一个任务
			write_cr3(addr_v2p((uint32_t )next->ptr_pdir));
			task_release(prev, next);
			task_ready = next;
			//释放
			
		}
	io_store_eflags(eflags);
}

int task_ready_list_len()
{
	struct task *p = task_ready_list;
	int i = 0;
	while(p != NULL){
		if(p->status != TASK_EXIT){	//如果不是要退出的任务才可以统计
			i++;
		}
		p = p->next;
	}
	return i;
}

void task_close(struct task *task)
{
	task->status = TASK_EXIT;
	task->ticks = 0;
}

struct task *id_to_task(uint32_t id)
{
	int i;
	for(i = 0; i < MAX_TASKS; i++){
		if(task_manage->tasks_table[i].pid == id){
			return &task_manage->tasks_table[i];
		}
	}
}

/*
we let next task to release prev task
*/
#define DEBUG_RELEASE 0

bool task_release(struct task *prev, struct task *next)
{
	struct task *old_task = next;	//get current interrupt task
	struct task *task = prev;
	if(DEBUG_RELEASE){
		//printk("start release %s\n",task->name);
	}
	
	//寻找当前任务
	struct task *p = task_ready_list;
	while(p != NULL){
		if(p == task){	//找到
			break;
		}
		p = p->next;
	}
	if(p == NULL){	//如果没找到，就直接退出
		return false;
	}
	
	//释放区域
	/*
	we need free what?
	1.task stack when running
	2.task image when running
	3.task pdir 
	4.task struct
	5.task mm 
	*/
	int i;
	if(old_task != task){ //not slef
		if(DEBUG_RELEASE){
			//printk("other task %s to release task %s\n", old_task->name, task->name);
		}
		//free stack
		kfree((void *)(task->esp_addr-PAGE_SIZE));	//释放任务的栈
		
		//free image
		uint32_t *old_pdir = (uint32_t *)kmalloc(PAGE_SIZE);
		//保存原有数据
		memcpy(&old_pdir[1], &old_task->ptr_pdir[1], 2044);
		
		//copy task bitmap 
		//_init->vir_mem_bitmap.bits = task->vir_mem_bitmap.bits;
		//memcpy(_init->vir_mem_bitmap.bits, task->vir_mem_bitmap.bits, VIR_MEM_BITMAP_SIZE);
		
		//把要释放的任务的页空间复制到当前任务
		memcpy(&old_task->ptr_pdir[1], &task->ptr_pdir[1], 2044);
		if(DEBUG_RELEASE){
			//printk("we will free %d pages.\n",task->image_pages);
		}
		//free space
		for(i = 0; i < task->image_pages; i++){
			clean_vir_page_table(task->entry_point);
			task->entry_point += PAGE_SIZE;
		}
		//恢复原来的页表关系
		memcpy(&old_task->ptr_pdir[1], &old_pdir[1], 2044);
		
		//write_cr3(addr_v2p((uint32_t)old_pdir));	//change pdir
		//memset(_init->vir_mem_bitmap.bits, 0, VIR_MEM_BITMAP_SIZE);
		
		//free mem bitmap
		//我们直接把整块区域释放了，就不用具体慢慢释放单个
		kfree(task->vir_mem_bitmap.bits);
		
		kernel_free_page((int )task->mm,task->mm_live_pages);
		
		//free task pdir
		kernel_free_page((int )task->ptr_pdir,1);
		if(DEBUG_RELEASE){
			//printk("release stack,image,bitap,bitmap struct and pdir success!\n");
		}
	}else{
		if(DEBUG_RELEASE){
			//printk("self task %s to release task %s\n", old_task->name, task->name);
		}
		return  false;
	}
	
	//free task struct
	for(i = 0; i < MAX_TASKS; i++){
		if(&task_manage->tasks_table[i] == p){	//寻找
			//把数据清空
			task_manage->tasks_table[i].status = TASK_UNUSED;
			task_manage->tasks_table[i].pid = 0;
			break;
		}
	}
	//数据清空
	p->status = TASK_UNUSED;
	p->pid = 0;
	if(DEBUG_RELEASE)
		//printk("release task struct success!\n");
	//再从链表上去掉
	if(p == task_ready_list){	//如果任务是在队列最开始
		task_ready_list = p->next;	//为下一个任务，这样任务就脱离列表了
	}
	//如果不是在最前面
	struct task *last;
	//在表上寻找
	last = task_ready_list;
	while(last->next != NULL){
		if(last->next == task){	//找到
			last->next = task->next;	//把task跳过，这样把它脱离了
			break;
		}
		last = last->next;
	}
	if(DEBUG_RELEASE)
		//printk("unlink from task chain success!\n");
	//print_tasks();
	
	return true;
}

void task_sleep(struct task *task)
{
	if(task->status == TASK_RUNNING){	//休眠自己
		task->status = TASK_SLEEP;
		//debug_printf("sleep myself name:%s status:%d\n", task->name, task->status);
		//switch_to();	//如果是自己那就切换进程
	}else {	//休眠其它
		task->status = TASK_SLEEP;
		//是别人就等待调度切换
		//debug_printf("sleep task name:%s status:%d\n", task->name, task->status);
	}
	//设定命令后循环
	//while(task->status == TASK_SLEEP);
}

void task_wakeup(struct task *task)
{
	if(task->status == TASK_SLEEP){	//唤醒
		task->status = TASK_READY;
	}
	//设定命令后循环
	//while(task->status != TASK_RUNNING);
}

bool task_status_is(struct task *task, int status)
{
	if(task->status == status){
		return 1;
	}
	return 0;
}

struct task *task_current()
{
	return task_ready;
}

void task_idle_entry()
{
	int i;
	printk("Running in idle!");
	for(;;){
		i++;
		if(i%1000000 == 0){
			
		}
	}
}

void task_name(struct task *task, char *name)
{
	char *str = name;
	int i;
	for(i = 0; i < TASK_NAME_LEN; i++){
		if(*str != 0){
			task->name[i] = *str;
			str++;
		}else{
			task->name[i] = ' ';
		}
	}
	task->name[TASK_NAME_LEN] = 0;
}

void task_init(struct task *task, int entry, char *name)
{
	task->regs.eip = entry;
	//make stack
	task->regs.ebp = task->regs.esp = task->esp_addr = ((uint32_t )(kmalloc(PAGE_SIZE*TASK_STACK_PAGES) + PAGE_SIZE*TASK_STACK_PAGES));
	task_name(task, name);
	
	//都是根目录
	strcpy(task->cwd,"/");
	//printk("kernel task cwd %s\n", task->cwd);
	//other
	//创建信箱
	task->mailbox = create_mailbox();
	mailbox_init(task->mailbox);
	
	//退出状态为无
	task->exit_status = 0;
}

void task_init_bitmap(struct task *task)
{
	task->vir_mem_bitmap.bits = (uint8_t*)kmalloc(VIR_MEM_BITMAP_SIZE);
	task->vir_mem_bitmap.btmp_bytes_len = VIR_MEM_BITMAP_SIZE;
	bitmap_init(&task->vir_mem_bitmap);
	
}
void task_free_bitmap(struct task *task)
{
	kfree(task->vir_mem_bitmap.bits);
}

void schdule()
{
	struct task *task = task_current();

	if(task->status == TASK_RUNNING || task->status == TASK_READY){	//如果要调度的是在运行的或者是准备好进行调度的
		//设置优先级和状态
		task->ticks = task->priority;
		task->status = TASK_READY;
		//切换任务
		task_switch();
		//设置新任务状态
		task_ready = task_current();
		//如果优先级没有就说明是一个执行后有结束的任务
		task_ready->status = TASK_RUNNING;
		task_ready->ticks = task_ready->priority;
		//printk("%s ", task_ready->name);
		//更新cr3准备切换任务
		update_tss_cr3(task_ready);
		//printk("pid:%d\n",task_ready->pid);
	}else if(task->status == TASK_SLEEP || task->status == TASK_EXIT){	//如果要调度的是要休眠的，或者退出的
		//休眠的话，不重新设置ticks
		//切换任务
		task_switch();
			//设置新任务状态
		task_ready = task_current();

		//如果优先级没有就说明是一个执行后有结束的任务
		task_ready->status = TASK_RUNNING;
		task_ready->ticks = task_ready->priority;
		
		//更新cr3准备切换任务
		update_tss_cr3(task_ready);
	}
}

void task_print()
{
	struct task *p = task_ready_list;
	while(p != NULL){
		printk("  pid:%d status:%d ticks:%x name:%s\n",p->pid, p->status, p->run_ticks, p->name);
		p = p->next;
	}
}

void task_exit(int status)
{
	struct task *task = task_current();
	
	task->ticks = 0;
	task->exit_status = status;
	//printk("[exit] %s pid %d status %d\n", task->name, task->pid, task->exit_status);
	
	//等所有信息都设置完后才设置退出
	task->status = TASK_EXIT;	//juset exit, and we will let schdule to deal it
	//循环等待被kill
	//while(1);
}

/*
active pdir
*/
void page_dir_activate(struct task* task) 
{
	write_cr3(task->cr3);
}

void print_task_info(struct task* task)
{
	printk("=====task:%s=====\npriority:%d status:%d pid:%d\n", task->name, task->priority, task->status, task->pid);
	printk("image_pages:%d entry_point:%x esp_addr:%x\n", task->image_pages, task->entry_point, task->esp_addr);
	printk("kernel_esp:%x eflags:%x esp:%x eip:%x\n", task->regs.kernel_esp, task->regs.eflags, task->regs.esp, task->regs.eip);
}

int task_getpid()
{
	struct task *task = task_current();
	return task->pid;
}

/*
finally, we will have a argc and a argv
*/
int32_t task_execcv(const char* path, int argc , const char* argv[]) 
{
	disable_schdule();
	/*
	printk("path:%s\n", path);
	int j;
	for(j = 0; j < argc; j++){
		
		printk("%s \n", argv[j]);

	}
	
	*/
	//1.alloc a task
	struct task* child_task = task_alloc(TASK_TYPE_USER);
	
	//2.open image file
	int32_t fd = sys_open(path, O_RDONLY);
	if (fd == -1) {
		printk("execv: open %s faild!\n",path);
		task_free(child_task);
		//可以切换任务
		clock.can_schdule = true;
		return -1;
	}
	//get file by fd
	struct file *file = get_file_though_fd(fd);
	
	//3.make task name.
	char name[DE_NAME_LEN];
	memset(name,0,DE_NAME_LEN);
	path_to_name(path, name);
	
	//4.init mem bitmap.
	task_init_bitmap(child_task);
	
	//5.link pages with parent vir memory.
	//how many pages should we occupy.
	uint32_t pages = (file->fd_dir->size-1)/PAGE_SIZE + 1;
	
	//vir addr map
	uint32_t vaddr = TASK_USER_START_ADDR;
	int i;
	
	for(i = 0; i < pages; i++){
		fill_vir_page_talbe(vaddr + i*PAGE_SIZE);
	}
	
	//6.read data form disk into vir memory.
	sys_lseek(fd, 0, SEEK_SET);
	sys_read(fd, (void*)vaddr, file->fd_dir->size);
	sys_close(fd);
	
	/*7.copy pages from parent task to child task,
	*so that child can live in memory.
	*/
	uint32_t pde_nr = (pages-1)/(PAGE_SIZE*PAGE_SIZE) + 1;
	//we don't use 0~4MB, because we afraid it will occupy kernel.
	memcpy(&child_task->ptr_pdir[1], &task_mailbox->ptr_pdir[1], pde_nr*4);
	
	//8.set child task's entrypoint.
	int32_t entry_point	= vaddr;     
	if (entry_point == -1) {	 // 若加载失败则返回-1
		printk("execv: file load error!\n");
		
		task_free(child_task);
		task_free_bitmap(child_task);
		
		//open schedule
		clock.can_schdule = true;
		return -1;
	}

	child_task->entry_point = entry_point;
	child_task->image_pages = pages;
	
	
	//9.init child task's memory manage environment, such as malloc, free
	init_task_memory_manage(child_task);
	//we had used some page for child to live in.
	task_mem_bitmap_alloc(child_task, pages);
	
	//10.init task info, now a task is ready to run.
	task_init(child_task, child_task->entry_point, name);
	
	//11.make arguments
	//make a new argv at new place.
	char *arg_stack = (char *)(child_task->esp_addr-PAGE_SIZE);
	uint32_t _argc = make_new_arguments_cv(arg_stack, argc, argv);

	//put arguments into task's registers
	child_task->regs.ebx = (int32_t)arg_stack;
	child_task->regs.ecx = _argc;

	//12.run the task! when next clock hit there.
	task_run(child_task, PRIORITY_LEVEL_II);
	//open schedule
	enable_schdule();
	
	return child_task->pid;
}

/*
in this func, we just need argv, when put into mailbox
*/

int32_t make_new_arguments_v(char *buf, const char **argv)
{
	//11.make arguments
	uint32_t argc = 0;
	//把参数放到栈中去
	char *arg_stack = (char *)buf;
	if(argv != NULL){
		
		//先复制下来
		while (argv[argc]) {  
			argc++;
		}
		//printk("argc %d\n", argc);
		//构建整个栈
		
		//临时字符串
		int str_stack[STACK_ARGC_MAX/4];
		
		int stack_len = 0;
		//先预留出字符串的指针
		int i;
		for(i = 0; i < argc; i++){
			stack_len += 4;
		}
		//printk("stack_len %d\n", stack_len);
		int idx, len;
		
		for(idx = 0; idx < argc; idx++){
			len = strlen(argv[idx]);
			memcpy(arg_stack + stack_len, argv[idx], len);
			str_stack[idx] = (int )(arg_stack + stack_len);
			//printk("str ptr %x\n",str_stack[idx]);
			arg_stack[stack_len + len + 1] = 0;
			stack_len += len + 1;
			//printk("len %d pos %d\n",len, stack_len);
		}
		//转换成int指针
		int *p = (int *)arg_stack;
		//重新填写地址
		for(idx = 0; idx < argc; idx++){
			p[idx] = str_stack[idx];
			//printk("ptr %x\n",p[idx]);
		}
		//p[argc] = NULL;
		//指向地址
		char **argv2 = (char **)arg_stack;
		
		//argv2[argc] = NULL;
		//打印参数
		for (i = 0; i < argc; i++) {  
			//printk("args:%s\n",argv2[i]);
		}
		
	}
	return argc;
}

/*
in this func, we need argv and argc, when  execcv make new argu.
*/
int32_t make_new_arguments_cv(char *buf,int argc ,const char **argv)
{
	//11.make arguments
	uint32_t _argc = argc;
	//把参数放到栈中去
	char *arg_stack = (char *)buf;
	if(argv != NULL){
		
		//临时字符串
		int str_stack[STACK_ARGC_MAX/4];
		
		int stack_len = 0;
		//先预留出字符串的指针
		int i;
		for(i = 0; i < _argc; i++){
			stack_len += 4;
		}
		//printk("stack_len %d\n", stack_len);
		int idx, len;
		
		for(idx = 0; idx < _argc; idx++){
			len = strlen(argv[idx]);
			memcpy(arg_stack + stack_len, argv[idx], len);
			str_stack[idx] = (int )(arg_stack + stack_len);
			//printk("str ptr %x\n",str_stack[idx]);
			arg_stack[stack_len + len + 1] = 0;
			stack_len += len + 1;
			//printk("len %d pos %d\n",len, stack_len);
		}
		//转换成int指针
		int *p = (int *)arg_stack;
		//重新填写地址
		for(idx = 0; idx < _argc; idx++){
			p[idx] = str_stack[idx];
			//printk("ptr %x\n",p[idx]);
		}
		//p[argc] = NULL;
		//指向地址
		char **argv2 = (char **)arg_stack;
		
		//argv2[argc] = NULL;
		//打印参数
		for (i = 0; i < _argc; i++) {  
			//printk("args:%s\n",argv2[i]);
		}
		
	}
	return _argc;
}

int start_user_process(char *filename, char* argv[])
{
	//close schedule
	disable_schdule();
	
	struct task *parent = task_current();
	
	//1.alloc a task
	struct task *child = task_alloc(TASK_TYPE_USER);

	//2.open image file.
	int32_t fd = sys_open(filename, O_RDONLY);
	if (fd == -1) {
		printk("error: open %s faild!\n",filename);
		task_free(child);
		//可以切换任务
		enable_schdule();
		return -1;
	}
	struct file *file = get_file_though_fd(fd);
	
	//3.make task name.
	char name[DE_NAME_LEN];
	memset(name,0,DE_NAME_LEN);
	path_to_name(filename, name);
	
	//4.init mem bitmap.
	task_init_bitmap(child);

	//5.link pages with parent vir memory.
	//how many pages should we occupy.
	uint32_t pages = (file->fd_dir->size-1)/PAGE_SIZE + 1;
	//vir addr map
	uint32_t vaddr = USER_VIR_MEM_BASE_ADDR;
	int i;
	for(i = 0; i < pages; i++){
		fill_vir_page_talbe(vaddr + i*PAGE_SIZE);
	}
	
	//6.read data form disk into vir memory.
	sys_lseek(fd, 0, SEEK_SET);
	sys_read(fd, (void*)vaddr, file->fd_dir->size);
	sys_close(fd);
	
	/*7.copy pages from parent task to child task,
	*so that child can live in memory.
	*/
	uint32_t pde_nr = (pages-1)/(PAGE_SIZE*PAGE_SIZE) + 1;
	//we don't use 0~4MB, because we afraid it will occupy kernel.
	memcpy(&child->ptr_pdir[1], &parent->ptr_pdir[1], pde_nr*4);
	
	//8.set child task's entrypoint.
	int32_t entry_point	= vaddr; 
	if (entry_point == -1) {	 // 若加载失败则返回-1
		printk("error: file load error!\n");
		
		task_free(child);
		task_free_bitmap(child);
		
		//open schedule
		enable_schdule();
		return -1;
	}
	child->entry_point = entry_point;
	child->image_pages = pages;
	
	//9.init child task's memory manage environment, such as malloc, free
	init_task_memory_manage(child);
	task_mem_bitmap_alloc(child, pages);
	
	//10.init task info, now a task is ready to run.
	task_init(child, entry_point, name);
	//11.make arguments
	//because arguments are in kernel, so we don't need to remake it.
	uint32_t argc = 0;
	if(argv != NULL){
		while (argv[argc]) {  
			argc++;
		}
	}
	child->regs.ebx = (int32_t)argv;
	child->regs.ecx = argc;
	
	//12.run the task! when next clock hit there.
	task_run(child, PRIORITY_LEVEL_II);
	//open schedule
	enable_schdule();

	return child->pid;
}

/*
从任务的内存位图中申请count个位
*/

void task_mem_bitmap_alloc(struct task *task, int count)
{
	int i;
	int idx = bitmap_scan(&task->vir_mem_bitmap, count);
	//printk("*mem bitmap idx %d counts %d\n", idx, count);
	for(i = 0; i < count; i++){
		bitmap_set(&task->vir_mem_bitmap, idx + i, 1);
	}
}

/*
wait a pid task end, return status 
*/
int task_wait(int pid)
{
	int i;
	
	for(i = 0; i < MAX_TASKS; i++){
		if(task_manage->tasks_table[i].pid == pid){
			if(task_manage->tasks_table[i].status == TASK_EXIT){
				//printk("task: pid %d task ret status %d\n", pid, task_manage->tasks_table[i].exit_status);
				return task_manage->tasks_table[i].exit_status;
			}
		}
	}
	return -2;
}

