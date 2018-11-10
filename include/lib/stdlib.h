#ifndef _LIB_STDLIB_H
#define _LIB_STDLIB_H
#include "kernel/types.h"
/*clock*/
void delay(int sec);
int get_ticks();

/*task*/
int sub_exit(int status);
int exit(int status);
int32_t wait(int pid);
int32_t sub_wait(int pid);

int getpid();

int32_t execv(const char *path, const char* argv[]);
int32_t execl(const char *path, const char *arg, ...);

#endif
