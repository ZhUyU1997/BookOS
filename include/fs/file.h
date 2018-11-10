#ifndef _FILE_H_
#define _FILE_H_
#include "kernel/types.h"
#include "kernel/task.h"

/* file */
struct file
{
	uint32_t fd_pos;      // file operate offset,start at 0,max is file size - 1
	uint32_t fd_flags;	//file's attr
	struct dir_entry* fd_dir;	//pointer to dir
	struct dir_entry* fd_parent;	//pointer to dir
}__attribute__ ((packed));

#define FILE_SIZE sizeof(struct file) 

struct file_search_record {
   struct dir_entry *parent_dir;
   struct dir_entry *child_dir;
};

struct stat 
{
	uint8_t st_cluster;	 // 文件类型
	uint32_t st_size;		 // 尺寸
	uint8_t st_filetype;	 // 文件类型
};

/*file.c*/
int32_t file_create(struct dir_entry *parent_dir, char *name, uint8_t attr);
int32_t file_close(struct file* file);
void init_file_table();
int32_t alloc_in_file_table(void);
int32_t file_open(struct dir_entry *parent_dir, char *name);
int search_file(const char* pathname, struct file_search_record *record);
int32_t file_read(struct file* file, void* buf, uint32_t count);
int32_t file_write(struct file* file, void* buf, uint32_t count);
struct file *get_file_though_fd(int fd);

#endif

