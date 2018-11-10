#ifndef _MAILBOX_H_
#define _MAILBOX_H_
#include "kernel/types.h"
#include "kernel/task.h"

/*
Mailbox is a message struct.
When we need to do something,we send a message to Mailbox.
Kernel will fetch it and do something.
When kernel finish it, he will send a message to mailbox, user
will get it.
*/
struct message 
{
	//邮箱保存的内容
	/*
	i0 , v0 用来保存返回值
	i1,i2,i3是参数传递时的第几个整数
	v1 是传入的字符串
	v2 是传入的缓冲区
	*/
	int i0,i1,i2,i3;	//整数信息
	void *v0, *v1, *v2, *v3;	//指针信息
};
#define MESSAGE_SIZE sizeof(struct message )


struct mailbox 
{
	//信箱信息
	int status;		//信箱的状态
	int operator;	//请求什么操作
	int operate;	//请求什么操作
	struct task *sender;	//发送者
	struct message msg;
};

#define MAILBOX_SIZE sizeof(struct mailbox )


#define MAILBOX_IDLE 1
#define MAILBOX_BUSY 2
#define MAILBOX_BACK 3

#define MAILBOX_FS 1


#define MAILBOX_FOPEN 1
#define MAILBOX_FREAD 2
#define MAILBOX_FWRITE 3
#define MAILBOX_FSEEK 4
#define MAILBOX_FCLOSE 5
#define MAILBOX_FUNLINK 6

#define MAILBOX_MKDIR 7
#define MAILBOX_RMDIR 8

#define MAILBOX_CHDIR 9
#define MAILBOX_GETCWD 10

#define MAILBOX_LS 11

#define MAILBOX_EXECV 12
#define MAILBOX_WAIT 13

#define MAILBOX_READ_SECTORS 14
#define MAILBOX_WRITE_SECTORS 15


extern struct task *task_mailbox;

void mailbox_service_process();

struct mailbox *create_mailbox();
void mailbox_init(struct mailbox *mb);
int mailbox_put(struct mailbox *mb);
int mailbox_get(struct mailbox *mb);
int mailbox_print(const char *fmt, ...);

#endif