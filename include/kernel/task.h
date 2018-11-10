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

#define MAX_TASKS 20//�����������

/*
�����״̬
*/
#define TASK_UNUSED 0//����Ϊ����
#define TASK_READY 1 //׼�����У����Ա�����
#define TASK_RUNNING 2//��������������
#define TASK_SLEEP 3//��������״̬�����ɱ����ȣ���Ҫ����

#define TASK_EXIT 4

#define TASK_TYPE_USER  3 //����Ϊ�û�̬
#define TASK_TYPE_DRIVER  1//����Ϊ����̬
#define TASK_TYPE_UNKNOWN -1

#define TASK_NAME_LEN 24//��������
#define TASK_NAME_MAX TASK_NAME_LEN+1

/*
����6�ֽ��̵��ȵǼǣ���ֵԽ�󣬵Ǽ�Խ��
*/
#define PRIORITY_LEVEL_III 3
#define PRIORITY_LEVEL_II 1
#define PRIORITY_LEVEL_I 1
#define PRIORITY_LEVEL_Z 0


#define TASK_STACK_PAGES 10

#define STACK_ARGC_MAX 32

#define TASK_USER_START_ADDR 0X00400000

#define MAX_PATH_LEN 128	//File is new or modified


//ջ��Ľṹ
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

//���̿��ƿ�Ľṹ
struct task 
{
	struct stackframe regs;	//�Ĵ���ӳ��
	int cr3;				//ҳĿ¼��������ַ
	int *ptr_pdir;	
	
	int32_t priority;	//��Ȩ��
	int32_t status;	//״̬
	int32_t ticks;	//����ʱ��Ƭ
	int32_t run_ticks;
	int pid;		//����id
	char name[TASK_NAME_MAX];	//��������
	char cwd[MAX_PATH_LEN];		//��ǰ����·��,ָ��

	int console_id;
	
	struct console *console;
	struct ioqueue *ioqueue;
	struct bitmap vir_mem_bitmap;
	
	struct memory_manage *mm;
	
	struct mailbox *mailbox;
	
	struct view_control *vc;
	
	char type;		//��������ͣ�DRIVER ��USER
	
	uint32_t image_pages;	
	int entry_point;		
	uint32_t esp_addr;
	int mm_live_pages;	//��Ҫ����ҳ�������ڴ������
	
	int exit_status;	//�˳�ʱ��״̬
	
	struct task *next;
}__attribute__ ((packed));
/*
��ʼ���ù���ṹ
*/
struct task_manage {
	struct task tasks_table[MAX_TASKS];
};

extern struct task_manage *task_manage;//��������ȫ�ֱ���
extern struct task *task_ready;//׼�����е�����

extern struct task *task_idle;	
extern struct task *task_ready_list;

void init_tasks();//��ʼ��������
struct task *task_alloc(char type);
void task_free(struct task *task);//����һ������
void task_run(struct task *task, int priority);//ʹһ������������ж���
void task_switch();//ѡ����һ�����е���������
struct task *task_current();//��ȡ��ǰ�����е�����

void task_name(struct task *task, char *name);//�����񴢴�����
void task_init(struct task *task, int entry, char *name);//��ʼ������

void delay(int sec);
void task_idle_entry();
void task_close(struct task *task);//�ر�����
void task_sleep(struct task *task);//��������
void task_wakeup(struct task *task);//��������
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

