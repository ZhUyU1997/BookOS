#include "shell/shell.h"
#include "lib/string.h"
#include "fs/fatxe.h"
#include "lib/file.h"
#include "kernel/mailbox.h"

#define CMD_LINE_LEN 128
#define MAX_ARG_NR 16

//var
char final_path[MAX_PATH_LEN] = {0}; // 用于清洗路径时的缓冲
char cmd_line[CMD_LINE_LEN] = {0};
char cwd_cache[MAX_PATH_LEN] = {0};
char *cmd_argv[MAX_ARG_NR];
int argc = -1;

//func
void print_prompt();
int cmd_parse(char * cmd_str, char **argv, char token);
int read_key(char *buf);
void readline( char *buf, uint32_t count);

void wash_path(char *old_abs_path, char * new_abs_path);
void make_clear_abs_path(char *path, char *final_path);
char* path_parse(char* pathname, char* name_store);

//cmd
void cmd_cls(uint32_t argc, char** argv);
void cmd_pwd(uint32_t argc, char** argv);
char *cmd_cd(uint32_t argc, char** argv);
void cmd_ls(uint32_t argc, char** argv);
void cmd_help(uint32_t argc, char** argv);
void cmd_ps(uint32_t argc, char** argv);
int cmd_mkdir(uint32_t argc, char** argv);
int cmd_rmdir(uint32_t argc, char** argv);
int cmd_rm(uint32_t argc, char** argv);
int cmd_cat(uint32_t argc, char** argv);

void shell_process()
{
	cwd_cache[0] = '/';
	char buf[MAX_PATH_LEN] = {0};
	int arg_idx = 0;
	char sb[512];
	int i;
	char hdbuf[512];
	//clear();
	while(1){
		print_prompt();
		memset(cmd_line, 0, CMD_LINE_LEN);
		readline(cmd_line, CMD_LINE_LEN);
		
		if(cmd_line[0] == 0){
			continue;
		}
		
		argc = -1;
		argc = cmd_parse(cmd_line, cmd_argv, ' ');
		
		//printf("cmd arg %s\n",cmd_argv);

		if(argc == -1){
			printf("shell: num of arguments exceed %d\n",MAX_ARG_NR);
			continue;
		}

		arg_idx = 0;
		while(arg_idx < argc){
			make_clear_abs_path(cmd_argv[arg_idx], buf);
			arg_idx++;
		}

		if(!strcmp("cls", cmd_argv[0])){
			cmd_cls(argc, cmd_argv);
		}else if(!strcmp("pwd", cmd_argv[0])){
			cmd_pwd(argc, cmd_argv);
		}else if(!strcmp("cd", cmd_argv[0])){
			if(cmd_cd(argc, cmd_argv) != NULL){
				memset(cwd_cache,0, MAX_PATH_LEN);
				strcpy(cwd_cache, final_path);
			}
		}else if(!strcmp("ls", cmd_argv[0])){
			cmd_ls(argc, cmd_argv);
		}else if(!strcmp("ps", cmd_argv[0])){
			cmd_ps(argc, cmd_argv);
		}else if(!strcmp("help", cmd_argv[0])){
			cmd_help(argc, cmd_argv);
		}else if(!strcmp("mkdir", cmd_argv[0])){
			cmd_mkdir(argc, cmd_argv);	
		}else if(!strcmp("rmdir", cmd_argv[0])){
			cmd_rmdir(argc, cmd_argv);
		}else if(!strcmp("rm", cmd_argv[0])){
			cmd_rm(argc, cmd_argv);
		}else{
			make_clear_abs_path(cmd_argv[0], final_path);
			//printf("operate %s\n",final_path);
			cmd_argv[argc] = NULL;
			int pid = execv((const char *)final_path, (const char **)cmd_argv);
			
			//int pid = execl((const char *)final_path, cmd_argv[0], cmd_argv[1]);
			
			
			
			if(pid != -1){
				//printf("child pid %d.\n",pid);
				int status =  wait(pid);
				//printf("status %d.\n",status);
			}else{
				printf("shell: unknown cmd!");
			}
		}
		printf("\n");
	}
}

void cmd_help(uint32_t argc, char** argv)
{
	if(argc != 1){
		printf("help: no argument support!\n");
		return;
	}
	printf("  cls         clean screen.\n");
	printf("  pwd         print work directory.\n");
	printf("  cd          change current work dirctory.\n");
	printf("  ls          list files in current dirctory.\n");
	printf("  ps          print tasks.\n");
	printf("  mkdir       create a dir.\n");
	printf("  rmdir       remove a dir.\n");
	printf("  rm          delete a file.\n");
}

void cmd_ps(uint32_t argc, char** argv)
{
	if(argc != 1){
		printf("ps: no argument support!\n");
		return;
	}
	ps();
}
	
void cmd_ls(uint32_t argc, char** argv)
{
	char *pathname = NULL;
	int detail = 0;
	uint32_t arg_path_nr = 0;
	uint32_t arg_idx = 1;	//跳过argv[0]
	while(arg_idx < argc){
		if(argv[arg_idx][0] == '-'){	//参数形式
			if(!strcmp(argv[arg_idx], "-l")){
				detail = 1;
			}else if(!strcmp(argv[arg_idx], "-h")){
				printf("  -l list all infomation about the file.\n\
  -h get command  help\n\
  list all files in the current dirctory if no option.\n");
				return;
			}
		}else {	//是个路径
			if(arg_path_nr == 0){
				pathname = argv[arg_idx];
				
				arg_path_nr = 1;
			}else{
				printf("ls: only support one path!\n");
				return;
			}
		}
		arg_idx++;
	}
	if(pathname == NULL){
		if(!getcwd(final_path, MAX_PATH_LEN)){
			pathname = final_path;
		}else{
			printf("ls: getcwd for default path faild!\n");
			return;
		}
	}else{
		make_clear_abs_path(pathname, final_path);
		pathname = final_path;
	}
	//printf("ls: path:%s\n", pathname);
	ls(pathname, detail);
	if(!detail){
		printf("\n");
	}
	
}

char *cmd_cd(uint32_t argc, char** argv)
{
	//printf("pwd: argc %d\n", argc);
	if(argc > 2){
		printf("cd: only support 1 argument!\n");
		return NULL;
	}
	
	if(argc == 1){	//只有cd
		final_path[0] = '/';
		final_path[1] = 0;
	}else{
		//不只有cd ，例如 cd a
		make_clear_abs_path(argv[1], final_path);
	}
	//现在已经有了绝对路径
	//尝试改变目录，如果没有就失败
	//printf("cd: operate %s\n",final_path);
	
	if(chdir(final_path) == -1){
		printf("cd: no such directory %s\n",final_path);
		return NULL;
	}
	return final_path;
}

void cmd_pwd(uint32_t argc, char** argv)
{
	//printf("pwd: argc %d\n", argc);
	if(argc != 1){
		printf("pwd: no argument support!\n");
		return;
	}else{
		if(!getcwd(final_path, MAX_PATH_LEN)){
			printf("%s\n", final_path);
		}else{
			printf("pwd: get current work directory failed!\n");
		}
	}
}

void cmd_cls(uint32_t argc, char** argv)
{
	//printf("cls: argc %d\n", argc);
	if(argc != 1){
		printf("cls: no argument support!\n");
		return;
	}
	clear();
}

int cmd_mkdir(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		printf("mkdir: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		
		
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(mkdir(final_path) == 0){
				printf("mkdir: create a dir %s success.\n", final_path);
				ret = 0;
			}else{
				printf("mkdir: create directory %s faild!\n", argv[1]);
			}
			
		}
		
	}
	return ret;
}

int cmd_rmdir(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		printf("mkdir: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(rmdir(final_path) == 0){
				printf("rmdir: remove %s success.\n", final_path);
				ret = 0;
			}else{
				printf("rmdir: remove %s faild!\n", final_path);
			}
		}
	}
	return ret;
}

int cmd_rm(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		printf("rm: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(funlink(final_path) == 0){
				printf("rm: delete %s success.\n", final_path);
				ret = 0;
			}else{
				printf("rm: delete %s faild!\n", final_path);
			}
		}
	}
	return ret;
}

void make_clear_abs_path(char *path, char *final_path)
{
	char abs_path[MAX_PATH_LEN] = {0};
	/* 先判断是否输入的是绝对路径 */
	if(path[0] != '/'){
		//不是绝对路径，把它拼接成绝对路径
		memset(abs_path,0, MAX_PATH_LEN);
		
		if (!getcwd(abs_path, MAX_PATH_LEN)) {
			//printf("cwd %s\n", abs_path);
			if (!((abs_path[0] == '/') && (abs_path[1] == 0))) {
				// 若 abs_path 表示的当前目录不是根目录/
				strcat(abs_path, "/");
			}
		}
	}
	strcat(abs_path, path);
	wash_path(abs_path, final_path);
}

void wash_path(char *old_abs_path, char * new_abs_path)
{
	char name[MAX_FILE_NAME_LEN]= {0};
	char *sub_path = old_abs_path;
	sub_path = path_parse(sub_path, name);
	if(name[0] == 0){	//只有根目录"/"
		new_abs_path[0] = '/';
		new_abs_path[1] = 0;
		return;
	}
	//避免传给new_abs_path的缓冲不干净
	new_abs_path[0] = 0;
	strcat(new_abs_path, "/");
	while(name[0]){
		//如果是上一级目录
		if(!strcmp(name,"..")){
			
			char *slash_ptr = (char *)strrchr(new_abs_path, '/');
			/*如果未到 new_abs_path 中的顶层目录，就将最右边的'/'替换为 0，
			这样便去除了 new_abs_path 中最后一层路径，相当于到了上一级目录 
			*/
			if(slash_ptr != new_abs_path){
				// 如 new_abs_path 为“/a/b”， ".."之后则变为“/a
				*slash_ptr = 0;
			}else{ // 如 new_abs_path 为"/a"， ".."之后则变为"/"
				/* 若 new_abs_path 中只有 1 个'/'，即表示已经到了顶层目录,
				就将下一个字符置为结束符 0 
				*/
				*(slash_ptr + 1) = 0;
			}
			
		}else if(strcmp(name,".")){
			// 如果路径不是‘.’，就将 name 拼接到 new_abs_path
			if (strcmp(new_abs_path, "/")) {
				// 如果 new_abs_path 不是"/"
				// 就拼接一个"/",此处的判断是为了避免路径开头变成这样"//"
				strcat(new_abs_path, "/");
			}
			strcat(new_abs_path, name);
		}
		memset(name, 0, MAX_FILE_NAME_LEN);
		if(sub_path){
			sub_path = path_parse(sub_path, name);
		}
	}
}

int cmd_parse(char * cmd_str, char **argv, char token)
{
	if(cmd_str == NULL){
		return;
	}
	int arg_idx = 0;
	while(arg_idx < MAX_ARG_NR){
		argv[arg_idx] = NULL;
		arg_idx++;
	}
	char *next = cmd_str;
	int argc = 0;
	while(*next){
		//跳过token字符
		while(*next == token){
			next++;
		}
		//如果最后一个参数后有空格 例如"cd / "
		if(*next ==0){
			break;
		}
		//存入一个字符串地址，保存一个参数项
		argv[argc] = next;
		
		//每一个参数确定后，next跳到这个参数的最后面
		while(*next && *next != token){
			next++;
		}
		//如果此时还没有解析完，就把这个空格变成'\0'，当做字符串结尾
		if(*next){
			*next++ = 0;
		}
		//参数越界，解析失败
		if(argc > MAX_ARG_NR){
			return -1;
		}
		//指向下一个参数
		argc++;
		//让下一个字符串指向0
		argv[argc] = 0;
	}
	return argc;
}

void readline( char *buf, uint32_t count)
{
	char *pos = buf;
	while(read_key(pos) && (pos - buf) < count){
		switch(*pos){
			case '\n':
			case '\r':
				//当到达底部了就不在继续了，目前还没有设定
				*pos = 0;
				printf("\n");
				return;
			case '\b':
				if(buf[0] != '\b'){
					--pos;
					printf("\b");
				}
				break;
			default:
				printf("%c", *pos);
				pos++;
		}
	}
	printf("readline: error!");
}

int read_key(char *buf)
{
	char key;
	do{
		key = getch();
	}while(key == -1);
	*buf = key;
	return 1;
}

void print_prompt()
{
	printf("%s>", cwd_cache);
}
