#ifndef _LIB_DISK_H
#define _LIB_DISK_H
#include "lib/stdint.h"

void read_sectors(int dev, int sector, char *buf, uint32_t counts);
void write_sectors(int dev, int sector, char *buf, uint32_t counts);

#endif
