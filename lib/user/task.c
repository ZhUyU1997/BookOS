#include "lib/stdlib.h"
#include "kernel/mailbox.h"
#include "lib/mailbox.h"
#include "lib/func.h"
#include "lib/stdarg.h"
int32_t execv(const char *path, const char* argv[])
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_EXECV;		//fopen

	mailbox.msg.v1 = (char *)path;
	mailbox.msg.v2 = argv;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int32_t execl(const char *path, const char *arg, ...)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_EXECV;		//fopen

	va_list parg = (char*)(&arg);
	char **p = (char**)parg;
	return execv(path, (const char **)p);
}

int exit(int status)
{
	sub_exit(status);
	while(1);
}

int32_t wait(int pid)
{
	int status;
	do{
		status = sub_wait(pid);
	}while(status == -2);
	return status;
}