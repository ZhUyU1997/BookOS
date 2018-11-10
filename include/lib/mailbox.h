#ifndef _LIB_MIALBOX_H_
#define _LIB_MIALBOX_H_
#include "lib/stdint.h"
#include "kernel/mailbox.h"

int putmailbox(struct mailbox *mailbox);
int getmailbox(struct mailbox *mailbox);

#endif
