#ifndef _BMP_H_
#define _BMP_H_

#include "kernel/types.h"

//文件头结构体
typedef struct 
{
 unsigned char    bfType[2];
 unsigned long bfSize;
 unsigned short bfReserved1;
 unsigned short bfReserved2;
 unsigned long bfOffBits;
} __attribute__((packed)) BitMapFileHeader;

//信息头结构体
typedef struct 
{
 unsigned long biSize;
long biWidth;
long biHeight;
 unsigned short biPlanes;
 unsigned short biBitCount;
 unsigned long biCompression;
 unsigned long biSizeImage;
long biXPelsPerMeter;
long biYPelsPerMeter;
 unsigned long biClrUsed;
 unsigned long biClrImportant;
} __attribute__((packed)) BitMapInfoHeader; 

//像素点结构体
typedef struct 
{
 unsigned char Blue;
 unsigned char Green;
 unsigned char Red;
 unsigned char Reserved;
} __attribute__((packed)) RgbQuad;

int show_photo(const char *bmpname);

#endif //_BMP_H_