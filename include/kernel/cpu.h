#ifndef _CPU_H
#define _CPU_H
#include "kernel/types.h"

/*
cpu结构体
*/
struct cpu 
{
	char vendor_string[16];
	char name_string[50];
	unsigned int family, model, stepping;
	unsigned int family_ex, model_ex, stepping_ex;
	unsigned int max_cpuid, max_cpuid_ex;
};

struct tsc 
{
	int start_high, start_low;
	int end_high, end_low;
};

extern struct cpu cpu;
extern struct tsc tsc;
void init_cpu();//初始化


void asm_cpuid(int id_eax, int id_ecx, int *eax, int *ebx, int *ecx, int *edx);//获取cpuid
void asm_rdtsc(int *high, int *low);

#endif

