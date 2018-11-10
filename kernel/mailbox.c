#include "kernel/mailbox.h"
#include "driver/hd.h"
#include "lib/stdarg.h"

#define MAILBOX_DEBUG 0

struct task *task_mailbox;
struct mailbox *service_mailbox;
void mailbox_service_process()
{
	task_mailbox = task_current();
	service_mailbox = task_mailbox->mailbox;
	printk("%s is running...\n", task_mailbox->name);
	
	int ret;
	
	struct mailbox *mailbox;
	struct hard_disk *old_disk;
	
	//service_mailbox->msg.v0 = kmalloc(128);
	//�ַ���128�ֽ�
	service_mailbox->msg.v1 = kmalloc(128);
	//buf������512kb
	service_mailbox->msg.v2 = kmalloc(SECTOR_SIZE*1024);
	
	while(1){
		
		if(service_mailbox->status == MAILBOX_IDLE){
			if(task_mailbox->status != TASK_SLEEP){
				task_sleep(task_mailbox);
			}
		}else if(service_mailbox->status == MAILBOX_BUSY){	//����Ϣ���ݽ���
			/*mailbox_print("[mailbox] receive mailbox.\n");
			mailbox_print(" operate:%d\n sender:%s\n", service_mailbox->operate, service_mailbox->sender);
			mailbox_print(" msg i1:%d i2:%d i3:%d\n",service_mailbox->msg.i1, service_mailbox->msg.i2, service_mailbox->msg.i3);
			*/
			switch(service_mailbox->operate){
				case MAILBOX_FOPEN:
					ret = sys_open(service_mailbox->msg.v1, service_mailbox->msg.i1);
					break;	
				case MAILBOX_FREAD: 
					ret = sys_read(service_mailbox->msg.i1, service_mailbox->msg.v2, service_mailbox->msg.i2);
					break;	
				case MAILBOX_FWRITE:
					ret = sys_write(service_mailbox->msg.i1, service_mailbox->msg.v2, service_mailbox->msg.i2);
					break;	
				case MAILBOX_FSEEK:
					ret = sys_lseek(service_mailbox->msg.i1, service_mailbox->msg.i2, service_mailbox->msg.i3);
					break;	
				case MAILBOX_FCLOSE:
					ret = sys_close(service_mailbox->msg.i1);
					break;	
				case MAILBOX_FUNLINK:	
					ret = sys_unlink(service_mailbox->msg.v1);
					break;	
				case MAILBOX_MKDIR:	
					ret = sys_mkdir(service_mailbox->msg.v1);
					break;	
				case MAILBOX_RMDIR:		
					ret = sys_rmdir(service_mailbox->msg.v1);
					break;	
				case MAILBOX_CHDIR:		
					ret = sys_chdir(service_mailbox->sender,   service_mailbox->msg.v1);
					break;	
				case MAILBOX_GETCWD:		
					ret = sys_getcwd(service_mailbox->sender, service_mailbox->msg.v2, service_mailbox->msg.i1);
					break;	
				case MAILBOX_LS:		
					sys_ls(service_mailbox->msg.v1, service_mailbox->msg.i1);
					ret = 0;
					break;	
				case MAILBOX_EXECV:	
					//printk("path:%s\n", service_mailbox->msg.v1);
					ret = task_execcv(service_mailbox->msg.v1, service_mailbox->msg.i1 , service_mailbox->msg.v2);
					
					//ret = do_execv(service_mailbox->msg.v1, service_mailbox->msg.v2);
					break;	
				case MAILBOX_WAIT:		
					ret = task_wait(service_mailbox->msg.i1);
					break;	
				case MAILBOX_READ_SECTORS:
					
					printk("dev %d sector %d buf %x counts %d\n", service_mailbox->msg.i1, service_mailbox->msg.i2, (void *)service_mailbox->msg.v2,service_mailbox->msg.i3);
					old_disk = current_disk;
					use_disk(service_mailbox->msg.i1);
					hd_read_sectors(service_mailbox->msg.i2, (void *)service_mailbox->msg.v2,service_mailbox->msg.i3);
					ret = 0;
					set_current_disk(old_disk);
					
					break;	
				case MAILBOX_WRITE_SECTORS:		
					old_disk = current_disk;
					use_disk(service_mailbox->msg.i1);
					hd_write_sectors(service_mailbox->msg.i2,(void *)service_mailbox->msg.v2,service_mailbox->msg.i3);
					ret = 0;
					set_current_disk(old_disk);
					
					break;			
				default : 
					printk("\n[mailbox] unknown message!\n");
					break;
			}
			mailbox = service_mailbox->sender->mailbox;
			mailbox->msg.i0 = ret;
			mailbox->status = MAILBOX_BACK;
			//mailbox_print("[mailbox] back\n");
			service_mailbox->status = MAILBOX_IDLE;
		}
	}
}

int mailbox_put(struct mailbox *mailbox)
{
	//ȷ�����͸�˭��
	if(service_mailbox->status == MAILBOX_IDLE){
		if(mailbox->operator == MAILBOX_FS){
			struct task *task = task_current();
			/*mailbox_print("[mailbox] put mailbox\n");
			mailbox_print(" msg i1:%d i2:%d i3:%d\n",mailbox->msg.i1, mailbox->msg.i2, mailbox->msg.i3);*/
			/*
			���ݲ�ͬ�Ĳ������趨��ͬ����Ϣ
			*/
			switch(mailbox->operate){
				case MAILBOX_FOPEN: 
					//�趨�ַ���
					strcpy(service_mailbox->msg.v1, mailbox->msg.v1);
					//�趨flags
					service_mailbox->msg.i1 = mailbox->msg.i1;
					break;
				case MAILBOX_FREAD: 
					//�趨fd
					service_mailbox->msg.i1 = mailbox->msg.i1;
					
					//�趨buf
					memset(service_mailbox->msg.v2,0,mailbox->msg.i2);
					task->mailbox->msg.v2 = service_mailbox->msg.v2;
					
					//�趨counts
					service_mailbox->msg.i2 = mailbox->msg.i2;
					break;
				case MAILBOX_FWRITE:
					
					//�趨fd
					service_mailbox->msg.i1 = mailbox->msg.i1;
					
					//�趨buf
					/*
					�Ȱ����ݸ��Ƶ�hd��buf����
					*/
					memset(service_mailbox->msg.v2,0,mailbox->msg.i2);
					memcpy(service_mailbox->msg.v2, mailbox->msg.v2, mailbox->msg.i2);
					
					//�趨counts
					service_mailbox->msg.i2 = mailbox->msg.i2;
					break;	
				case MAILBOX_FSEEK:
					
					//�趨fd
					service_mailbox->msg.i1 = mailbox->msg.i1;
					//offset
					service_mailbox->msg.i2 = mailbox->msg.i2;
					//whence
					service_mailbox->msg.i3 = mailbox->msg.i3;
					break;	
				case MAILBOX_FCLOSE:
				
					//�趨fd
					service_mailbox->msg.i1 = mailbox->msg.i1;
					break;	
				case MAILBOX_FUNLINK:	
				
					//�趨name
					
					strcpy(service_mailbox->msg.v1, mailbox->msg.v1);
					break;		
				case MAILBOX_MKDIR:		
				
					//�趨name
					strcpy(service_mailbox->msg.v1, mailbox->msg.v1);
					break;	
				case MAILBOX_RMDIR:		
				
					//�趨name
					
					strcpy(service_mailbox->msg.v1, mailbox->msg.v1);
					break;	
				case MAILBOX_CHDIR:		
					
					
					strcpy(service_mailbox->msg.v1, mailbox->msg.v1);
					break;	
				case MAILBOX_GETCWD:		
					
					//buf
					memset(service_mailbox->msg.v2,0,mailbox->msg.i1);
					task->mailbox->msg.v2 = service_mailbox->msg.v2;
					
					//size
					service_mailbox->msg.i1 = mailbox->msg.i1;
					break;	
				case MAILBOX_LS:		
				
					//path
					strcpy(service_mailbox->msg.v1, mailbox->msg.v1);
					
					//size
					service_mailbox->msg.i1 = mailbox->msg.i1;
					break;	
				case MAILBOX_EXECV:		
					
					//path
					strcpy(service_mailbox->msg.v1, mailbox->msg.v1);
					//argv
					//now v3 -> v2 , but when we in mailbox, he don't known where it  is.
					//so we should make arguments there.
					//printk("read argv 2\n");
					
					int argc = make_new_arguments_v(service_mailbox->msg.v2, mailbox->msg.v2);
					service_mailbox->msg.i1 = argc;
					//service_mailbox->msg.v2[argc] = NULL;
					/*printk("argc:%d\n", argc);
					
					printk("read argv 2 end\n");
					*/
					
					//service_mailbox->msg.v3 = mailbox->msg.v2;
					break;	
				case MAILBOX_WAIT:		
				
					//pid
					service_mailbox->msg.i1 = mailbox->msg.i1;
					break;	
				case MAILBOX_READ_SECTORS:		
				
					//dev
					service_mailbox->msg.i1 = mailbox->msg.i1;
					//sector
					service_mailbox->msg.i2 = mailbox->msg.i2;
					//buf
					memset(service_mailbox->msg.v2,0,mailbox->msg.i3*SECTOR_SIZE);
					task->mailbox->msg.v2 = service_mailbox->msg.v2;
					
					//counts
					service_mailbox->msg.i3 = mailbox->msg.i3;
				case MAILBOX_WRITE_SECTORS:		
				
					//dev
					service_mailbox->msg.i1 = mailbox->msg.i1;
					//sector
					service_mailbox->msg.i2 = mailbox->msg.i2;
					//buf
					memset(service_mailbox->msg.v2,0,mailbox->msg.i3*SECTOR_SIZE);
					memcpy(service_mailbox->msg.v2, mailbox->msg.v2, mailbox->msg.i3*SECTOR_SIZE);

					//counts
					service_mailbox->msg.i3 = mailbox->msg.i3;

					break;	
		
				default :
					break;
			}

			//��������״̬
			service_mailbox->status = mailbox->status = MAILBOX_BUSY;
			
			//���ò���
			service_mailbox->operate = mailbox->operate;
			service_mailbox->sender = task;
			task_wakeup(task_mailbox);
			//mailbox_print("[mailbox] put success!\n");
			//�ɹ�����
			return 1;
		}
	}
	//����ʧ��
	return 0;
}

int mailbox_get(struct mailbox *mailbox)
{
	struct task *task = task_current();
	
	//����ǿ��Է���
	if(task->mailbox->status == MAILBOX_BACK){	
		//mailbox_print("[mailbox] get mailbox.\n operate:%d\n", mailbox->operate);
		
		/*
		*Ĭ�ϵķ�����Ϣ����
		*/
		mailbox->msg.i0 = task->mailbox->msg.i0;
		//mailbox_print("[message] i0:%d\n", mailbox->msg.i0);
		
		/*
		*�������ز����������������������������ȡ���ݽ���������Ҫ���ƻ���������
		*/
		switch(mailbox->operate){
			case MAILBOX_FREAD:
				//��������
				memcpy(mailbox->msg.v2, task->mailbox->msg.v2, mailbox->msg.i2);
				break;
			case MAILBOX_GETCWD:
				//��������
				memcpy(mailbox->msg.v2, task->mailbox->msg.v2, mailbox->msg.i1);
				break;
			case MAILBOX_READ_SECTORS:
				//��������
				memcpy(mailbox->msg.v2, task->mailbox->msg.v2, mailbox->msg.i3*SECTOR_SIZE);	
			default :
				break;
		}
		
		task->mailbox->status = MAILBOX_IDLE;
		//mailbox_print("[mailbox] get success!\n");
		return 1;
	}
	return 0;
}

struct mailbox *create_mailbox()
{
	return (struct mailbox *)kmalloc(MAILBOX_SIZE);
}

void mailbox_init(struct mailbox *mailbox)
{
	memset(mailbox, 0, MAILBOX_SIZE);
	mailbox->status = MAILBOX_IDLE; 
	mailbox->sender = NULL;
	mailbox->msg.v0 = mailbox->msg.v1 = mailbox->msg.v2 = mailbox->msg.v3 = NULL;
}

int mailbox_print(const char *fmt, ...)
{
	if(MAILBOX_DEBUG){
		int i;
		char buf[256];
		va_list arg = (va_list)((char*)(&fmt) + 4); /*4�ǲ���fmt��ռ��ջ�еĴ�С*/
		i = vsprintf(buf, fmt, arg);
		print_buf(buf, i);
	}
}

