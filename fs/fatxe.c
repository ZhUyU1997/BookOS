#include"driver/hd.h"
#include"kernel/task.h"
#include"fs/fatxe.h"
#include"fs/file.h"
#include"fs/dir.h"
#include"fs/fat.h"
uint32_t fat_start_lba;
uint32_t fat_sectors;
uint32_t data_start_lba;
uint32_t root_dir_start_lba;

uint32_t sector_per_cluster;

struct dbr dbr;
struct fsinfo fsinfo;

struct dir dir_root;

static void init_dbr();
static void init_fsinfo();

void sync_fsinfo();


char *sync_buffer;

void fs_format()
{
	uint8_t *sync_buffer = (uint8_t *)kmalloc(1024);
	init_dbr();
	init_fsinfo();
	init_fat();
	init_directory();
	init_file_table();
	open_root_dir();
	//mkdir("/bin/dd");
	
	/*
	mkdir("/user");
	mkdir("/user/txt");
	*/
	//sys_open("/bin",O_CREAT);
	
	/*fd = sys_open("/bin2",O_CREAT|O_RDWR);
	printk("fd:%d\n",fd);
	fd = sys_open("/bin/dd",O_CREAT|O_RDWR);
	printk("fd:%d\n",fd);
	fd = sys_open("/bin/a",O_CREAT|O_RDONLY);
	printk("fd:%d\n",fd);
	sys_close(fd);
	sys_unlink("/bin/a");
	*/
	/*mkdir("/bin");
	
	int fd;
	fd = sys_open("/bin/test",O_CREAT|O_RDWR);
	
	memset(fs_bufffer, 0, SECTOR_SIZE*2);
	
	strcpy(fs_bufffer,"hello , i am in /bin/test");
	
	int written = sys_write(fd, fs_bufffer, 512);
	printk("fd:%d written:%d\n", fd, written);
	
	memset(fs_bufffer, 0, SECTOR_SIZE*2);
	sys_lseek(fd, 0, SEEK_SET);
	
	int read = sys_read(fd, fs_bufffer, SECTOR_SIZE*2);
	printk("fd:%d read:%d\n", fd, read);
	
	printk("%s\n", fs_bufffer);
	
	sys_close(fd);
	
	struct dir *dir = sys_opendir("/bin");
	//
	struct dir_entry de;
	de = *sys_readdir(dir);
	printk("name:%s\n",de.name);
	de = *sys_readdir(dir);
	printk("name:%s\n",de.name);
	de = *sys_readdir(dir);
	printk("name:%s\n",de.name);
	
	dir_close(dir);
	
	struct stat a;
	memset(&a,0,sizeof(struct stat ));
	sys_stat("/", &a);
	printk("type:%x cluster:%x size:%d\n",a.st_filetype,a.st_cluster,a.st_size);
	memset(&a,0,sizeof(struct stat ));
	sys_stat("/bin", &a);
	printk("type:%x cluster:%x size:%d\n",a.st_filetype,a.st_cluster,a.st_size);
	memset(&a,0,sizeof(struct stat ));
	sys_stat("/bin/test", &a);
	printk("type:%x cluster:%x size:%d\   n",a.st_filetype,a.st_cluster,a.st_size);
	*/
	
	//de = *sys_readdir(dir);

	/*
	int idx = 0;
	printk("name:%s\n",&dir->dir_buf[idx]);
	idx += 32;
	printk("name:%s\n",&dir->dir_buf[idx]);
	idx += 32;
	printk("name:%s\n",&dir->dir_buf[idx]);
	*/
	/*
	fd = sys_open("/test",O_CREAT|O_RDWR);
	memset(fs_bufffer, 0, SECTOR_SIZE*5);
	
	fs_bufffer[0] = 0x11;

	strcpy(fs_bufffer,"hello , i am in /test");
	
	written = sys_write(fd, fs_bufffer, 512);
	printk("fd:%d written:%d\n", fd, written);
	
	sys_close(fd);*/
	/*fd = sys_open("/bin/test",O_CREAT|O_RDWR);
	
	//printk("fd:%d flags:%x\n", fd, file_table[fd].fd_flags);
	
	memset(fs_bufffer, 0, SECTOR_SIZE*5);
	
	fs_bufffer[0] = 'A';
	fs_bufffer[511] = 'B';
	fs_bufffer[512] = 'C';
	fs_bufffer[1023] = 'D';
	fs_bufffer[1024] = 0x33;
	fs_bufffer[1024+256-1] = 0xcc;
	fs_bufffer[2048-1] = 'F';
	fs_bufffer[2048-2] = 'E';
	
	written = sys_write(fd, fs_bufffer, 2048+32);
	printk("fd:%d written:%d\n", fd, written);
	
	sys_close(fd);
	*/
	/*fd = sys_open("/bin",O_CREAT);
	printk("fd:%d\n",fd);
	fd = sys_open("/bin2",O_CREAT|O_RDWR);
	printk("fd:%d\n",fd);*/
	
	//sys_unlink("/bin2");
	//sys_unlink("/bin/dd");
	//sys_unlink("/bin/a");
	
	/*fd = sys_open("/bin/ld",O_CREAT|O_RDWR);
	printk("fd:%d\n",fd);
	fd = sys_open("/bin/ld",O_CREAT|O_RDWR);
	printk("fd:%d\n",fd);
	*/
	/*fd = sys_open("/bin/ld",O_CREAT|O_RDWR);
	printk("fd:%d\n",fd);*/
	//printk("%s\n",name);
	
	/*
	int fd = file_create(&root_dir, "foo.bin", ATTR_NORMAL);
	printk("fd:%d\n",fd);
	
	fd = file_create(&root_dir, "hal.bin", ATTR_NORMAL);
	printk("fd:%d\n",fd);
	*/
	/*rmdir("/bin/dd");
	//mkdir("/bin/dd");
	
	//rmdir("/bin/dd");
	rmdir("/user/txt");
	
	//rmdir("/bin");
	rmdir("/user");
*/
	/*
	int i = get_a_cluster();
	printk("%d ",i);
	
	i = get_a_cluster();
	printk("%d ",i);
	i = get_a_cluster();
	*/
	
	//printk("%d\n",get_a_cluster());
	//printk("%d\n",get_a_cluster());
}

/* 更改当前工作目录为绝对路径path,成功则返回0,失败返回-1 */
int32_t sys_chdir(struct task *task, const char* path)
{
	int32_t ret = -1;
   
	struct file_search_record record;  
	memset(&record, 0, sizeof(struct file_search_record));
	int found = search_file(path, &record);
	//printk("sys_chdir: %s is found!\n", path);
	//找到该目录
	if (found == 1) {
		//printk("sys_chdir: %s is found!\n", path);
		if(record.child_dir->attributes&ATTR_DIRECTORY){
			//task_current()->cwd = inode_no;
			strcpy(task->cwd, path);
			//printk("sys_chdir: task pwd %s \n", task->cwd);
			
			ret = 0;
		}
	}else {	//没有找到
		printk("chdir: %s isn't exist!\n", path);
	}
	//close_dir_entry(record.parent_dir);
	close_dir_entry(record.child_dir);
	return ret;
}

int sys_getcwd(struct task *task, char* buf, uint32_t size)
{
	memcpy(buf, task->cwd, size);
	return 0;
}

int32_t sys_stat(char* path, struct stat* buf)
{
   /* 若直接查看根目录'/' */
   if (!strcmp(path, "/")) {
      buf->st_filetype = ATTR_DIRECTORY;
      buf->st_cluster = root_dir.high_cluster<<16|root_dir.low_cluster;
      buf->st_size = root_dir.size;
      return 0;
   }

   int32_t ret = -1;	// 默认返回值
   struct file_search_record record;
   memset(&record, 0, sizeof(struct file_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
   int found = search_file(path, &record);
   if (found == 1) {
		if (record.child_dir->attributes&ATTR_DIRECTORY) {
			//printk("dir file!\n");
			buf->st_filetype = ATTR_DIRECTORY;
		}else if (record.child_dir->attributes&ATTR_NORMAL) {
			//printk("normal file!\n");
			buf->st_filetype = ATTR_NORMAL;
		}
		buf->st_size = record.child_dir->size;
		buf->st_cluster = record.child_dir->high_cluster<<16|record.child_dir->low_cluster;
		//close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		ret = 0;
   } else {
      printk("stat: %s not found\n", path);
   }
	close_dir_entry(record.parent_dir);
   return ret;
}



void sys_close_dir(struct dir* dir)
{
	if(dir == NULL || dir == &dir_root){	// can't close root dir
		
		return;
	}
	//free all
	if(dir->dir_ptr != NULL){
		kfree(dir->dir_ptr);
	}
	kfree(dir);
}
void open_root_dir() 
{
	memset(&dir_root,0,sizeof(struct dir));
	dir_root.dir_ptr = &root_dir;
	dir_root.dir_pos = 0;
	
	uint32_t cluster = dir_root.dir_ptr->high_cluster<<16|dir_root.dir_ptr->low_cluster;
	hd_read_sectors(cluster_to_lba(cluster), dir_root.dir_buf, 1);
	//printk("child:%s cluster:%x\n", dir_root.dir_ptr->name,cluster);
}
void sys_rewinddir(struct dir* dir)
{
   dir->dir_pos = 0;
}

void sys_ls(char *path, int detail)
{
	//打开根目录
	struct dir *dir = sys_opendir(path);

	//重定位
	sys_rewinddir(dir);
	//读取一个目录项
	struct dir_entry* de;
	char type;
	
	de = sys_readdir(dir);
	while(de != NULL){
		if(de->name[0]){	//显示有名字的
			if(detail){
				if(de->attributes&ATTR_DIRECTORY){
					type = 'd';
				}else{
					type = '-';
				}
				printk("%c %s %d ", type, de->name, de->size);
				printk("%d/%d/%d",
					DATA16_TO_DATE_YEA(de->create_date),
					DATA16_TO_DATE_MON(de->create_date),
					DATA16_TO_DATE_DAY(de->create_date));
				printk(" %d:%d:%d\n",
					DATA16_TO_TIME_HOU(de->create_time),
					DATA16_TO_TIME_MIN(de->create_time),
					DATA16_TO_TIME_SEC(de->create_time));
			}else{
				printk("%s ", de->name);
			}
			
		}
		de = sys_readdir(dir);
	}
	sys_close_dir(dir);
}

struct dir_entry* sys_readdir(struct dir* dir)
{
	if(dir == NULL){	
		return NULL;
	}
	struct dir_entry *de = (struct dir_entry *)kmalloc(sizeof(struct dir_entry));
	if(de == NULL){	
		return NULL;
	}
	de = (struct dir_entry *)&dir->dir_buf[dir->dir_pos];
	dir->dir_pos += 32;
	if(dir->dir_pos > 512){
		//dir->dir_pos = 0;
		return NULL;
	}
	return de;
}

struct dir* sys_opendir(const char* name)
{
	//if it's root , we return root dir
	if (name[0] == '/' && name[1] == 0) {
		uint32_t cluster = dir_root.dir_ptr->high_cluster<<16|dir_root.dir_ptr->low_cluster;
		
		hd_read_sectors(cluster_to_lba(cluster), dir_root.dir_buf, 1);
		return &dir_root;
	}
	struct file_search_record record;
	
	int found = search_file(name, &record);
	
	struct dir *dir = NULL;
	uint32_t cluster;
	memset(dir,0,sizeof(struct dir));
	if(found == 1){	//fount
		if (record.child_dir->attributes&ATTR_NORMAL) {	///dir
			//printk("%s is regular file!\n", name);
			
			close_dir_entry(record.child_dir);
		} else if (record.child_dir->attributes&ATTR_DIRECTORY) {
			//printk("%s is dir file!\n", name);
			dir = (struct dir *)kmalloc(sizeof(struct dir));
			
			if(dir == NULL){
				close_dir_entry(record.parent_dir);
				close_dir_entry(record.child_dir);
				
				return NULL;
			}
			dir->dir_ptr = record.child_dir;
			dir->dir_pos = 0;
			
			cluster = dir->dir_ptr->high_cluster<<16|dir->dir_ptr->low_cluster;
			hd_read_sectors(cluster_to_lba(cluster), dir->dir_buf, 1);
			//printk("child:%s cluster:%x\n", dir->dir_ptr->name,cluster);
		}
	}else{
		printk("in path %s not exist\n", name); 
	}
	close_dir_entry(record.parent_dir);

	return dir;
}

int32_t sys_lseek(int32_t fd, int32_t offset, uint8_t whence)
{
	if (fd < 0) {
		printk("lseek: fd error\n");
		return -1;
	}

	struct file* pf = &file_table[fd];
	
	//printk("seek file %s\n",pf->fd_dir->name);
	int32_t new_pos = 0;   //new pos must < file size
	int32_t file_size = (int32_t)pf->fd_dir->size;
	
	switch (whence) {
		case SEEK_SET: 
			new_pos = offset; 
			break;
		case SEEK_CUR: 
			new_pos = (int32_t)pf->fd_pos + offset; 
			break;
		case SEEK_END: 
			new_pos = file_size + offset;
			break;
		default :
			printk("lseek: unknown whence!\n");
	
			break;
	}
	
	if (new_pos < 0 || new_pos > file_size) {	 
		return -1;
	}
	
	pf->fd_pos = new_pos;
	return pf->fd_pos;
}

int32_t sys_write(int32_t fd, void* buf, uint32_t count)
{
	if (fd < 0) {
		printk("write: fd error\n");
		return -1;
	}
	if(count == 0) {
		printk("swrite: count zero\n");
		return -1;
	}
	
    struct file* wr_file = &file_table[fd];
    if(wr_file->fd_flags & O_WRONLY || wr_file->fd_flags & O_RDWR){
		uint32_t bytes_written  = file_write(wr_file, buf, count);
		return bytes_written;
	} else {
		printk("write: not allowed to write file without flag O_RDWR or O_WRONLY\n");
		return -1;
	}
}

int32_t sys_read(int32_t fd, void* buf, uint32_t count)
{
	if (fd < 0) {
		printk("read: fd error\n");
		return -1;
	}
	if (count == 0) {
		printk("read: count zero\n");
		return -1;
	}
	
	struct file* rd_file = &file_table[fd];
	
	//printk("Read file name %s\n", rd_file->fd_dir->name);
    if(rd_file->fd_flags & O_RDONLY || rd_file->fd_flags & O_RDWR){
		uint32_t bytes_road  = file_read(rd_file, buf, count);
		return bytes_road;
	} else {
		printk("read: not allowed to read file without flag O_RDONLY or O_WRONLY\n");
		return -1;
	}
}

/* 将最上层路径名称解析出来*/
char* path_parse(char* pathname, char* name_store) 
{
   if (pathname[0] == '/') {   // 根目录不需要单独解析
    /* 路径中出现1个或多个连续的字符'/',将这些'/'跳过,如"///a/b" */
       while(*(++pathname) == '/');
   }

   /* 开始一般的路径解析 */
   while (*pathname != '/' && *pathname != 0) {
      *name_store++ = *pathname++;
   }
   if (pathname[0] == 0) {   // 若路径字符串为空则返回NULL
      return NULL;
   }
   return pathname; 
}

int32_t sys_open(const char *pathname,uint8_t flags)
{
	//printk("open:path %s flags %x\n", pathname, flags);
	
	struct file_search_record record;
	int32_t fd = -1;	   //default not found
	char name[DE_NAME_LEN];
	int reopen = 0;	//file has exist,but operate has O_CREAT
	memset(name,0,DE_NAME_LEN);
	
	//printk("Open file name %s\n", pathname);
	
	int found = search_file(pathname, &record);
	//printk("sys_open:path %s flags %x\n", pathname, flags);
	
	if(path_to_name(pathname, name)){
		//printk("path:%s to name:%s error!\n", pathname, name);
	}else{
		//printk("path:%s to name:%s success!\n", pathname, name);
	}
	//printk("@ 1\n");
	if(found == 1){	//fount
		//printk("parent:%s attr:%x\n", record.parent_dir->name, record.child_dir->attributes);
		//printk("found!\n");
		if(record.child_dir->attributes & ATTR_DIRECTORY){	//found a dir
			printk("open: can't open a direcotry with open(), use opendir() to instead!\n");
			close_dir_entry(record.parent_dir);
			close_dir_entry(record.child_dir);
			//printk("Open file name %s faild 1 end\n", pathname);
			return -1;
		}
		if ((flags & O_CREAT)) {  // file we create has exist!
			//printk("open: %s has already exist!\n", pathname);
			if((flags & O_RDONLY) || (flags & O_WRONLY) || (flags & O_RDWR)){	//just create
				reopen = 1;
				//printk("%s %s can be reopen.\n", pathname, name);
			}else{
				
				close_dir_entry(record.parent_dir);
				close_dir_entry(record.child_dir);
				printk("open: file name %s error!\n", pathname);
				return -1;
			}
		}
	}else {
		//printk("not found!\n");
		if (!found && !(flags & O_CREAT)) {	//not found ,not create
			printk("open: path %s, file isn't exist!\n", pathname);
			close_dir_entry(record.parent_dir);
			close_dir_entry(record.child_dir);
			//printk("Open file name %s faild 3 end\n", pathname);
			return -1;
		}
	}
	//printk("@ 2\n");
	//set attributes
	uint8_t attr = ATTR_NORMAL;
	
	if(flags & O_RDONLY){
		attr |= ATTR_READONLY;
	}else if(flags & O_WRONLY){
		attr |= ATTR_WRITEONLY;
	}else if(flags & O_RDWR){
		attr |= ATTR_RDWR;
	}
	
	if(flags & O_CREAT) {	
		if(!reopen){	//only not found
			//printk("now creating file...\n");
			fd = file_create(record.parent_dir, name, attr);
			//printk("now the fd is %d\n", fd);
			if(fd != -1){	//no error,
				close_dir_entry(record.child_dir);
			}else{	//error, we don't need to close child dir, we had close it in create
				close_dir_entry(record.parent_dir);
			}
			//printk("Open file name %s end\n", pathname);
			
			return fd;
		}
		//found, do nothing
	}
	//
	if((flags & O_RDONLY) || (flags & O_WRONLY) || (flags & O_RDWR)){
		// open exsit file
		//O_RDONLY,O_WRONLY,O_RDWR
		//printk("now opening file...\n");
		fd = file_open(record.parent_dir, name);
		if(fd != -1){	//no error,
			close_dir_entry(record.child_dir);
		}else{	//error, we don't need to close child dir, we had close it in create
			close_dir_entry(record.parent_dir);
		}
		/*
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);*/
	}
	/*
	switch (flags & O_CREAT) {
		case O_CREAT:
			printk("creating file\n");
			fd = file_create(record.parent_dir, name, attr);
			close_dir_entry(record.parent_dir);
			break;
		default:
			// open exsit file
			//O_RDONLY,O_WRONLY,O_RDWR
			fd = file_open(record.parent_dir, name);
			close_dir_entry(record.parent_dir);
			break;
	}*/
	//printk("Open file name %s end of ok\n", pathname);
	//printk("now the fd is %d\n", fd);
	return fd;
	
	/*
	//not found , not create, return -1
	if (!found && !(flags & O_CREAT)) {
		printk("path %s, file is't exist\n", pathname);
		close_dir_entry(record.parent_dir);
		return -1;
	} else if (found && (flags & O_CREAT)) {  // file we create has exist!
		printk("%s has already exist!\n", pathname);
		close_dir_entry(record.parent_dir);
		return -1;
	}*/
	
}
/*
only file ,not dir
*/
int32_t sys_unlink(const char* pathname)
{
	//1.file is exist？
	struct file_search_record record;
	int found = search_file(pathname, &record);
	if(found == 1){	//fount
		//printk("parent:%s attr:%x\n", record.parent_dir->name, record.child_dir->attributes);
		
		if(record.child_dir->attributes & ATTR_DIRECTORY){	//found a dir
			printk("unlink: can't delete a direcotry with unlink(), use rmdir() to instead!\n");
			close_dir_entry(record.parent_dir);
			close_dir_entry(record.child_dir);
			return -1;
		}
		//found a file
	}else {
		printk("unlink: file %s not found!\n", pathname);
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		return -1;
	}
	
	//2.file is in file table
	uint32_t file_idx = 0;
	while (file_idx < MAX_FILE_OPEN) {
		//if name is same and cluster same, that the file we want
		if (file_table[file_idx].fd_dir != NULL &&\
			strcmp(record.child_dir->name, file_table[file_idx].fd_dir->name) == 0 &&\
			(record.child_dir->high_cluster<<16|record.child_dir->low_cluster) == (file_table[file_idx].fd_dir->high_cluster<<16|file_table[file_idx].fd_dir->low_cluster))
		{
			break;
		}
		file_idx++;
	}
	if (file_idx < MAX_FILE_OPEN) {
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		printk("unlink: file %s is in use, not allow to delete!\n", pathname);
		return -1;
	}
	
	void *io_buf = (void *)kmalloc(SECTOR_SIZE);
	if (io_buf == NULL) {
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		printk("unlink: malloc for io_buf failed\n");
		return -1;
	}
	//3.empty file's sector and cluster
	release_dir_entry(record.child_dir, io_buf);
	//printk("sys_unlink: release file data success!\n");
	
	//4.del file in parent dir
	if(empty_dir_entry(record.parent_dir, record.child_dir, io_buf)){
		printk("unlink: delete file %s/%s.\n",record.parent_dir->name, record.child_dir->name);
		kfree(io_buf);
		close_dir_entry(record.parent_dir);
		close_dir_entry(record.child_dir);
		//printk("unlink file %s success!\n", pathname);
		return 0;
	}
	kfree(io_buf);
	close_dir_entry(record.parent_dir);
	close_dir_entry(record.child_dir);
	printk("unlink: delete file %s/%s faild!\n",record.parent_dir->name, record.child_dir->name);
	return -1;
}

/*
close file which fd point to, return 0 is ok, error -1
*/
int32_t sys_close(int32_t fd)
{
	int32_t ret = -1;   // defaut -1,error
	if (fd >= 0) {
		ret = file_close(&file_table[fd]);
		//printk("close fd:%d success!\n", fd);
	}else{
		printk("close: fd %d error!\n", fd);
	}
	
	return ret;
}

/*
get the name from path
*/
int path_to_name(const char *pathname, char *name_buf)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[DE_NAME_LEN];
	int i,j;
	
	if(*p != '/'){	//First must be /
		return 1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}
	//printk("deep:%d \n",deep);
	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		
		memset(name,0,DE_NAME_LEN);
		
		p++;	//skip '/'
		j = 0;
		//get a dir name
		while(*p != '/' && j < DE_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;	// transform to A~Z
			j++;
			p++;
		}
		name[j] = 0;
		//printk("name:%s %d\n",name, i);
		if(name[0] == 0){	//no name
			return 1;
		}
		
		if(i == deep-1){	//name is what we need
			j = 0;
			while(name[j]){	//if not arrive next '/'
				name_buf[j] = name[j];	// transform to A~Z
				j++;
			}
			name_buf[j] = 0;
			return 0;	//success
		}
	}
	return 1;
}

static void init_fsinfo()
{
	memset(&fsinfo, 0, sizeof(struct fsinfo ));
	//clean_sector(1);
	hd_read_sectors(1, &fsinfo, 1);
	if(fsinfo.fsinfo_signature != 0x61417272){
		//Init fsinfo
		fsinfo.extended_flags = 0x41615252;
		fsinfo.fsinfo_signature = 0x61417272;
		fsinfo.free_cluster_number = 0xffffffff;
		fsinfo.next_available_cluster = 0x00000002;	//Start at cluster 2 
		
		hd_write_sectors(1, &fsinfo, 1);
	}
	printk("fsinfo: next available cluster:%d\n",fsinfo.next_available_cluster);
}

static void init_dbr()
{
	memset(&dbr, 0, sizeof(struct dbr ));
	//clean_sector(0);
	hd_read_sectors(0, &dbr, 1);
	
	if(dbr.magic_flags != 0xaa55){
		
		dbr.bpb.byte_per_sector = 0x200;
		dbr.bpb.sector_per_cluster = 0x01;
		dbr.bpb.reserved_sector = 0x20;
		dbr.bpb.fats = 0x01;
		dbr.bpb.media_descriptor = 0xf8;
		dbr.bpb.fat_sectors = 2;	//Every fat has how many sectors
		dbr.bpb.hidden_sector = 0;
		dbr.bpb.totla_sector = hda.sectors;
		
		dbr.ext_bpb.physical_drive_number = 0x80;
		strcpy(dbr.ext_bpb.system_ID, "FAT32");
		strcpy(dbr.oem_name, "Mediocre");
		dbr.magic_flags = 0xaa55;
		hd_write_sectors(0, &dbr, 1);

	}
	
	sector_per_cluster = dbr.bpb.sector_per_cluster;
	
	
	printk("dbr: byte per sector:%x sector per cluster:%x\n\
    reserved sector:%x fats:%d fat sectors:%d\n",\
		dbr.bpb.byte_per_sector,\
		dbr.bpb.sector_per_cluster,\
		dbr.bpb.reserved_sector,\
		dbr.bpb.fats,\
		dbr.bpb.fat_sectors);
}

/*
*sync fsinfo into disk
*/

void sync_fsinfo()
{
	hd_write_sectors(1, &fsinfo, 1);
}
