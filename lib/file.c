#include "lib/mailbox.h"
#include "lib/file.h"
#include "lib/stdlib.h"
#include "lib/func.h"

int fopen(const char *pathname,uint8_t flags)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_FOPEN;		//fopen
	
	mailbox.msg.v1 = (char *)pathname;
	mailbox.msg.i1 = flags;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));

	return mailbox.msg.i0;
}

int fread(int32_t fd, void* buf, uint32_t count)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_FREAD;		//fopen
	
	mailbox.msg.i1 = fd;
	mailbox.msg.v2 = buf;
	mailbox.msg.i2 = count;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int fwrite(int32_t fd, void* buf, uint32_t count)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_FWRITE;		//fopen

	mailbox.msg.i1 = fd;
	mailbox.msg.v2 = buf;
	mailbox.msg.i2 = count;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int32_t fseek(int32_t fd, int32_t offset, uint8_t whence)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_FSEEK;		//fopen

	
	mailbox.msg.i1 = fd;
	mailbox.msg.i2 = offset;
	mailbox.msg.i3 = whence;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int32_t fclose(int32_t fd)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_FCLOSE;		//fopen

	mailbox.msg.i1 = fd;

	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}


int32_t funlink(const char* pathname)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_FUNLINK;		//fopen

	mailbox.msg.v1 = (char *)pathname;

	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int32_t mkdir(const char* pathname)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_MKDIR;		//fopen

	mailbox.msg.v1 = (char *)pathname;

	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int32_t rmdir(const char* pathname)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_RMDIR;		//fopen

	mailbox.msg.v1 = (char *)pathname;

	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int32_t chdir(const char* pathname)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_CHDIR;		//fopen

	mailbox.msg.v1 = (char *)pathname;

	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}
int32_t getcwd(char* buf, uint32_t size)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_GETCWD;		//fopen

	mailbox.msg.i1 = size;
	mailbox.msg.v2 = buf;

	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

int32_t ls(char *path, int detail)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_LS;		//fopen

	mailbox.msg.v1 = path;
	mailbox.msg.i1 = detail;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	return mailbox.msg.i0;
}

