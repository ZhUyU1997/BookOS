#ifndef _LIB_FUNC_H
#define _LIB_FUNC_H
#include "kernel/types.h"
#include "kernel/mailbox.h"
/*
In this function, we will put some indirect function at here.
It has exist in kernel, we remake it!
*/
void mailbox_init_user(struct mailbox *mailbox);

#endif

