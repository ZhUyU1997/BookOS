#include "kernel/types.h"
#include "driver/vga.h"
#include "kernel/descriptor.h"
#include "kernel/8259a.h"
#include "driver/clock.h"
#include "driver/keyboard.h"
#include "kernel/memory.h"
#include "kernel/syscall.h"
#include "kernel/task.h"
#include "driver/video.h"
#include "application/application.h"
#include "driver/hd.h"
#include "kernel/cpu.h"
#include "kernel/debug.h"
#include "kernel/page.h"
#include "fs/fatxe.h"
#include "kernel/lock.h"
#include "kernel/ioqueue.h"
#include "kernel/interruption.h"
#include "kernel/irqservice.h"
#include "driver/clock.h"
#include "shell/shell.h"
#include "kernel/mailbox.h"
#include "graph/graph.h"


void main()
{
	init_vga();
	init_descriptor();
	init_memory();
	init_tasks();
	/*
	After we init_task(), we will run _init,
	so it won't be arrive here!
	*/
}

/*
kernel going running
*/
#define WRITE_DISK 0

#define LAUNCH_USER 0

#define WRITE_ID 2

#define LOAD_OFF 400

#if WRITE_ID == 1
	#define WRITE_NAME "/echo"
	#define FILE_SECTORS 10
#elif WRITE_ID == 2
	#define WRITE_NAME "/type"
	#define FILE_SECTORS 10
#elif WRITE_ID == 3
	#define WRITE_NAME "/shell"
	#define FILE_SECTORS 20
#endif

void write_bin(char *name);
void task_testA_entry();
int graph_ok = 0;
void init()
{
	init_hdd();
	hd_identify(&hda);
	hd_identify(&hdb);
	set_current_disk(&hdb);
	
	init_clock();
	init_syscall();
	//初始化的时候直接使用hd
	fs_format();
	
	
	//现在多个任务可以进行并且切换
	struct task *task = task_current();
	struct ioqueue ioqueue;
	ioqueue_init(&ioqueue);
	task->ioqueue = &ioqueue;
	
	//irq is servicing for interruption
	registe_service("irq service", (int)&irq_service_process);
	create_service_process("irq service");
	
	//mailbox is servicing for user
	registe_service("mailbox service", (int)&mailbox_service_process);
	create_service_process("mailbox service");
	
	char *buf = kmalloc(512*FILE_SECTORS);
	//开启第一个应用程序
	set_current_disk(&hdb);
	if(WRITE_DISK){
		write_bin(WRITE_NAME);
	}
	if(LAUNCH_USER){
		char *argv[3];
		argv[0] = "Hello";
		argv[1] = "world!";
		int pid = start_user_process("/shell",NULL);
	}
	
	registe_service("graph service", (int)&graph_service_process);
	create_service_process("graph service");

	
	struct task *task_testA = task_alloc(TASK_TYPE_USER);
	task_init(task_testA, (int )task_testA_entry, "testA");
	task_run(task_testA, 1);

	
	for(;;){
		if(task->ioqueue != NULL){
			if(ioqueue_empty(task->ioqueue)){
				task_sleep(task);

			}else{
				ioqueue_get(task->ioqueue, IQ_MODE_MOVE);
			}
		}
	}
}

void task_testA_entry()
{
	while(1){
		if(graph_ok){
			create_window2("testA",320, 240);
			window_close2();
			graph_ok = 0;
		}
		
	}
}

void write_bin(char *name)
{
	printk("write_bin start...\n");
	
	set_current_disk(&hda);
	char *free_space = (char *)FREE_BUFFER_ADDR;

	int i;
	for(i = 0; i < FILE_SECTORS; i++){
		hd_read_sectors(LOAD_OFF+i,free_space + i*SECTOR_SIZE,1);
	}

	set_current_disk(&hdb);
	int fd;
	int written;
	
	fd = sys_open(name, O_CREAT|O_RDWR);
	written = sys_write(fd, free_space, FILE_SECTORS*SECTOR_SIZE);
	printk("write bin %s success!\n",name);	
	sys_close(fd);
}

