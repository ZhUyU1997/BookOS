#ifndef _VHD_H
#define _VHD_H
#include "kernel/types.h"
#include "driver/hd.h"
#include "kernel/memory.h"

/**
	vhd is virtual hard disk.
1. we will load secotr data from disk.
2. we will build a fs in memory
3. when we read file ,we will operat in memory.
4. we we will sync disk from memory.
*/

/*
2MB
*/
#define VHD_SECTORS 2048*2
#define VHD_VIR_MEM_SIZE (512*VHD_SECTORS)	


#endif

