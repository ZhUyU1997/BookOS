#include "kernel/mailbox.h"
#include "lib/string.h"
#include "lib/func.h"

void mailbox_init_user(struct mailbox *mailbox)
{
	//memset(mailbox, 0, MAILBOX_SIZE);
	mailbox->status = MAILBOX_IDLE; 
	mailbox->sender = NULL;
	mailbox->msg.v0 = mailbox->msg.v1 = mailbox->msg.v2 = mailbox->msg.v3 = NULL;
}

