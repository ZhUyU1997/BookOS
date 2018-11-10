#ifndef _LIB_STDIO_H
#define _LIB_STDIO_H
#include "kernel/types.h"
#include "lib/stdarg.h"

#define STR_DEFAULT_LEN 256

int getch();
void putch(char ch);


int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);

#endif
