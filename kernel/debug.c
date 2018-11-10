#include "kernel/debug.h"
#include "driver/vga.h"
#include "lib/stdarg.h"

//停机并输出大量信息
void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);
	set_color(0x06);
	printk("\n>>>>>panic<<<<<: %s", buf);

	while(1);
	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
//断言
void assertion_failure(char *exp, char *file, char *base_file, int line)
{
	set_color(0x06);
	printk("\nassert(%s) failed:\nfile: %s\nbase_file: %s\nln%d",
	exp, file, base_file, line);

	spin(">>>>>assertion_failure()<<<<<");

	/* should never arrive here */
        __asm__ __volatile__("ud2");
}
//停机显示函数名
void spin(char * func_name)
{
	printk("\nspinning in %s ...\n", func_name);
	while(1);
}

