#ifndef _SHELL_H
#define _SHELL_H
#include "kernel/types.h"

#define CMD_LINE_LEN 128
#define MAX_ARG_NR 16


void shell_process();
void sys_readline(char *buf, uint32_t count);
void readline(char *buf, uint32_t count);

#endif

