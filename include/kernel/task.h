#ifndef _TASK_H
#define _TASK_H
#include "kernel/types.h"
#include "gui/layer.h"
#include "gui/desktop.h"
#include "gui/window.h"
#include "kernel/memory.h"
#include "kernel/syscall.h"
#include "gui/opengl.h"
#include "kernel/ioqueue.h"
#include "driver/console.h"
#include "fs/fatxe.h"
#include "fs/fat.h"
#include "fs/dir.h"
#include "fs/file.h"
#include "lib/bitmap.h"
#include "graph/view.h"

#define MAX_TASKS 20//最大任务数量

/*
任务的状态
*/
#define TASK_UNUSED 0//任务为分配
#define TASK_READY 1 //准备运行，可以被调度
#define TASK_RUNNING 2//处于正在运行中
#define TASK_SLEEP 3//处于休眠状态，不可被调度，需要唤醒

#define TASK_EXIT 4

#define TASK_TYPE_USER  3 //任务为用户态
#define TASK_TYPE_DRIVER  1//任务为服务态
#define TASK_TYPE_UNKNOWN -1

#define TASK_NAME_LEN 24//任务名字
#define TASK_NAME_MAX TASK_NAME_LEN+1

/*
定义6种进程调度登记，数值越大，登记越高
*/
#define PRIORITY_LEVEL_III 3
#define PRIORITY_LEVEL_II 1
#define PRIORITY_LEVEL_I 1
#define PRIORITY_LEVEL_Z 0


#define TASK_STACK_PAGES 10

#define STACK_ARGC_MAX 32

#define TASK_USER_START_ADDR 0X00400000

#define MAX_PATH_LEN 128	//File is new or modified


//栈框的结构
struct stackframe
{
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t kernel_esp;		//esp
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
};

//进程控制块的结构
struct task 
{
	struct stackframe regs;	//寄存器映像
	int cr3;				//页目录表的物理地址
	int *ptr_pdir;	
	
	int32_t priority;	//特权级
	int32_t status;	//状态
	int32_t ticks;	//运行时间片
	int32_t run_ticks;
	int pid;		//任务id
	char name[TASK_NAME_MAX];	//任务名字
	char cwd[MAX_PATH_LEN];		//当前工作路径,指针

	int console_id;
	
	struct console *console;
	struct ioqueue *ioqueue;
	struct bitmap vir_mem_bitmap;
	
	struct memory_manage *mm;
	
	struct mailbox *mailbox;
	
	struct view_control *vc;
	
	char type;		//任务的类型：DRIVER 和USER
	
	uint32_t image_pages;	
	int entry_point;		
	uint32_t esp_addr;
	int mm_live_pages;	//需要多少页来管理内存管理本身
	
	int exit_status;	//退出时的状态
	
	struct task *next;
}__attribute__ ((packed));
/*
初始化用管理结构
*/
struct task_manage {
	struct task tasks_table[MAX_TASKS];
};

extern struct task_manage *task_manage;//任务管理的全局变量
extern struct task *task_ready;//准备运行的任务

extern struct task *task_idle;	
extern struct task *task_ready_list;

void init_tasks();//初始化多任务
struct task *task_alloc(char type);
void task_free(struct task *task);//分配一个任务
void task_run(struct task *task, int priority);//使一个任务进入运行队列
void task_switch();//选择下一个运行的人人任务
struct task *task_current();//获取当前运行中的任务

void task_name(struct task *task, char *name);//给任务储存名字
void task_init(struct task *task, int entry, char *name);//初始化任务

void delay(int sec);
void task_idle_entry();
void task_close(struct task *task);//关闭任务
void task_sleep(struct task *task);//任务休眠
void task_wakeup(struct task *task);//唤醒任务
bool task_release(struct task *prev, struct task *next);
bool task_status_is(struct task *task, int status);

int task_ready_list_len();
void task_test_entry();
void task_test2_entry();
void IrqServiceProcess();

#define DRIVER_NAME_LEN TASK_NAME_LEN

#define DRIVER_NR 8

struct service_process
{
	char name[DRIVER_NAME_LEN];
	int32_t entry_point;
};

struct task *id_to_task(uint32_t id);
void task_init_bitmap(struct task *task);
void task_print();
int32_t load(char* pathname, uint32_t *pages);
/*
for syscall
*/
void task_exit(int status);
int do_fork();
void page_dir_activate(struct task* task);
void print_task_info(struct task* task);
int task_getpid();

int start_user_process(char *filename, char* argv[]);
int32_t load_bin(char* pathname, uint32_t *pages);
int32_t load_bin2(char* pathname, uint32_t *pages);
int32_t do_execv(const char* path, const char* argv[]);
int do_execl(const char *path, char *arg, ...);
int32_t task_execcv(const char* path, int _argc , const char* argv[]);

void task_mem_bitmap_alloc(struct task *task, int count);


int32_t get_service_entry_point(char *name);
bool registe_service(char *name, int32_t entry_point);
int create_service_process(char *filename);

void task_free_bitmap(struct task *task);
void task_exit2(struct task *task, int status);
int task_wait(int pid);

int32_t make_new_arguments_v(char *buf,const char **argv);
int32_t make_new_arguments_cv(char *buf,int _argc ,const char **argv);

#endif

