#include "kernel/tss.h"
#include "kernel/io.h"
#include "driver/vga.h"
#include "lib/string.h"
#include "kernel/descriptor.h"
#include "kernel/task.h"

struct tss tss;

void init_tss()
{
	
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = KERNEL_DATA_SEL;
	
	load_tr(KERNEL_TSS_SEL);
	put_str("init tss.\n");
}
/*
	�������ҳĿ¼�����tss�У��Ժ����д��
*/
void update_tss_cr3(struct task *task)
{
	tss.cr3 = task->cr3;
	write_cr3(tss.cr3);
}

