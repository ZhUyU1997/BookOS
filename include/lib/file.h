#ifndef _LIB_FILE_H
#define _LIB_FILE_H
#include "kernel/types.h"
#include "fs/fatxe.h"

#define O_RDONLY 0x01 
#define O_WRONLY 0x02
#define O_RDWR 0x04
#define O_CREAT 0x08

#define SEEK_SET 1 
#define SEEK_CUR 2
#define SEEK_END 3

int fopen(const char *pathname,uint8_t flags);
int32_t fclose(int32_t fd);
int32_t funlink(const char* pathname);
int32_t fwrite(int32_t fd, void* buf, uint32_t count);
int32_t fread(int32_t fd, void* buf, uint32_t count);
int32_t fseek(int32_t fd, int32_t offset, uint8_t whence);

int32_t mkdir(const char *pathname);
int32_t rmdir(const char *pathname);

int32_t getcwd(char* buf, uint32_t size);
int32_t chdir(const char* path);

int32_t ls(char *path, int detail);


#endif

