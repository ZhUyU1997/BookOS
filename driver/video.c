#include "driver/video.h"
#include "driver/vga.h"
#include "driver/mouse.h"

struct video_info video_info;

void init_vesa()
{
	uint32_t *vram_addr;
	int vram;
	//直接从地址获取
	video_info.bpp = *((uint16_t *)VIDEO_INFO_ADDR);
	video_info.wide = (uint32_t )(*((uint16_t *)(VIDEO_INFO_ADDR+2)));
	video_info.high = (uint32_t )(*((uint16_t *)(VIDEO_INFO_ADDR+4)));
	video_info.linelength = video_info.wide;
	
	
	//先获取地址，在转换成指针
	vram_addr = (uint32_t *)(VIDEO_INFO_ADDR+6);
	video_info.vram = (uint8_t *) (*vram_addr);
	
	video_info.doubleBufferingSize = video_info.wide*video_info.high*4;
	video_info.buffer = (uint8_t *)kmalloc(video_info.doubleBufferingSize);
	memset(video_info.buffer, 0, video_info.doubleBufferingSize);

	put_str("\n");
	put_int(video_info.bpp);
	put_str("\n");
	put_int(video_info.wide);
	put_str("\n");
	put_int(video_info.high);
	put_str("\n");
	put_int((int )*vram_addr);
	put_str("\n");
}




/*
根据区域把缓冲区的数据写入显存
*/
void video_refresh_aera(int x, int y, int wide, int high)
{
	video_refresh_pos(x, y, x + wide, y + high);
}

void video_refresh_pos(int x0, int y0, int x1, int y1)
{
	//修正刷新范围
	if(x0 < 0){
		x0 = 0;
	}
	if(x1 >= video_info.wide){
		x1 = video_info.wide;
	}
	if(y0 < 0){
		y0 = 0;
	}
	if(y1 >= video_info.high){
		y1 = video_info.high;
	}
	int x, y;
	int idx_vram, idx_buf;
	for(y = y0; y < y1; y++){
		for(x = x0; x < x1; x++){
			idx_vram = ((y * video_info.linelength) + x)*3;
			idx_buf = ((y * video_info.linelength) + x)*4;
			if(video_info.buffer[idx_buf+3] != 0){
				video_info.vram[idx_vram] = video_info.buffer[idx_buf];	//blue
				video_info.vram[idx_vram+1] = video_info.buffer[idx_buf+1];	//green
				video_info.vram[idx_vram+2] = video_info.buffer[idx_buf+2];	//red
			}
		}
	}
}

