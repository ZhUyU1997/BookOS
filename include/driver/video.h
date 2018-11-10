#ifndef _DRIVER_VIDEO_H_
#define _DRIVER_VIDEO_H_

#include "kernel/types.h"
#include "graph/color.h"

#define VIDEO_INFO_ADDR 0x80006100
/*
	VIDEO_INFO_ADDR = vodeo color 
	VIDEO_INFO_ADDR+2 = vodeo wide
	VIDEO_INFO_ADDR+4 = vodeo high
	VIDEO_INFO_ADDR+6 = vodeo ram
*/

struct video_info {
	uint16_t bpp;		//2字节,每个像素占多少位
	uint32_t wide, high;	//2字节+2字节
	uint8_t *vram;				//指针
	
	uint32_t linelength;	//每一行的长度
	
	uint8_t *buffer;	//双缓冲的后端数据区（屏幕中的内存映射是前缓冲）
	int32_t doubleBufferingSize;	//记录双缓冲的大小
};

extern struct video_info video_info;

void init_vesa(void);


void video_refresh_pos(int x0, int y0, int x1, int y1);
void video_refresh_aera(int x, int y, int wide, int high);


#endif

