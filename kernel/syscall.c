#include "kernel/task.h"
#include "kernel/syscall.h"
#include "driver/keyboard.h"
#include "kernel/debug.h"
#include "kernel/message.h"
#include "kernel/cpu.h"
#include "lib/GL.h"
#include "gui/opengl.h"
#include "driver/vga.h"
#include "fs/fatxe.h"
#include "driver/clock.h"
#include "shell/shell.h"
#include "driver/hd.h"
#include "kernel/mailbox.h"
#include "graph/graph.h"

/**
	添加一个系统调用步骤：
	1.sys_call_table中添加函数名
	2.系统调用函数体和头文件
	3.在user的syscall中添加中断调用和头文件
*/
sys_call_t sys_call_table[NR_SYS_CALL];

void init_syscall()
{
	//系统调用信息包
	sys_call_table[_NR_PUTMAILBOX] = mailbox_put;
	sys_call_table[_NR_GETMAILBOX] = mailbox_get;
	
	sys_call_table[_NR_MALLOC] = memory_malloc;
	sys_call_table[_NR_FREE] = memory_free;
	
	sys_call_table[_NR_EXIT] = task_exit;
	sys_call_table[_NR_WAIT] = task_wait;
	sys_call_table[_NR_GETPID] = task_getpid;
	sys_call_table[_NR_PS] = task_print;
	
	sys_call_table[_NR_GETCH] = keyobard_getch;
	sys_call_table[_NR_PUTCH] = vga_putchar;
	sys_call_table[_NR_PRINTF] = memory_printf;
	sys_call_table[_NR_CLEAR] = vga_clear;
	
	sys_call_table[_NR_GETICKS] = clock_getticks;
	
	sys_call_table[_NR_CREATE_WINDOW] = graph_create_window;
	sys_call_table[_NR_CLOSE_WINDOW] = graph_window_close;
	
}

