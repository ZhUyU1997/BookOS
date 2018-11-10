#include "lib/disk.h"
#include "lib/stdlib.h"
#include "lib/mailbox.h"
#include "lib/func.h"

void read_sectors(int dev, int sector, char *buf, uint32_t counts)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	//fs
	mailbox.operate = MAILBOX_READ_SECTORS;	

	mailbox.msg.i1 = dev;
	mailbox.msg.i2 = sector;
	mailbox.msg.v2 = buf;
	mailbox.msg.i3 = counts;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	//return mailbox.msg.i0;
}

void write_sectors(int dev, int sector, char *buf, uint32_t counts)
{
	struct mailbox mailbox;
	mailbox_init_user(&mailbox);
	
	mailbox.operator = MAILBOX_FS;	
	mailbox.operate = MAILBOX_WRITE_SECTORS;	

	mailbox.msg.i1 = dev;
	mailbox.msg.i2 = sector;
	mailbox.msg.v2 = buf;
	mailbox.msg.i3 = counts;
	
	while(!putmailbox(&mailbox));
	while(!getmailbox(&mailbox));
	//return mailbox.msg.i0;
}
