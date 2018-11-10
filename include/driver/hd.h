#ifndef _HD_H
#define _HD_H
#include "kernel/types.h"
#include "fs/fatxe.h"
#include "kernel/lock.h"
#include "kernel/task.h"
#include "kernel/ioqueue.h"

#define HARDDISK_NR_ADDR 0x80000475
#define HDA_IRQ 14

#define REG_DATA	0x1F0	//Data register
#define REG_FEATURES	0x1F1	
#define REG_ERROR	REG_FEATURES	
#define REG_NSECTOR	0x1F2
#define REG_LBA_LOW	0x1F3
#define REG_LBA_MID	0x1F4
#define REG_LBA_HIGH	0x1F5
#define REG_DEVICE	0x1F6
#define REG_STATUS	0x1F7

#define REG_CMD		REG_STATUS
#define REG_DEV_CTRL	0x3F6
#define REG_ALT_STATUS	REG_DEV_CTRL
#define REG_DRV_ADDR	0x3F7

/*8 bit main status registers*/
#define	STATUS_BUSY	0x80	//Disk busy
#define	STATUS_READY	0x40	//Disk ready for 
#define	STATUS_WREER	0x20	//Disk error
#define	STATUS_SEEK	0x10	//Seek end
#define	STATUS_DRQ	0x08	//Request data
#define	STATUS_ECC	0x04	//ECC check error
#define	STATUS_INDEX	0x02	//Receive index
#define	STATUS_ERR	0x01	//Command error
/*AT disk controller command*/
#define	ATA_RESTORE	0x10	//driver retsore
#define	ATA_READ	0x20	//Read sector
#define	ATA_WRITE	0x30	//Write sector
#define	ATA_VERIFY	0x40	//Sector check
#define	ATA_FORMAT	0x50	//Format track
#define	ATA_INIT	0x60	//Controller init
#define	ATA_SEEK	0x70	//Seek operate
#define	ATA_DIAGNOSE	0x90	//Driver diagnosis command
#define	ATA_SPECIFY	0x91	//Setting up driver parameters
#define ATA_IDENTIFY		0xEC	//Get disk information

#define SECTOR_SIZE		512

#define	MAKE_DEVICE_REG(lba_mode,drv,lba_highest) (((lba_mode) << 6) |\
					      ((drv) << 4) |\
					      (lba_highest & 0xF) | (0xA0))

#define HDA		0
#define HDB		1
#define WAIT_TIMEOUT	0x5000

/*
ioqueue data mask
if 0x80000000 then
	write disk 
	
*/
#define WRITE_MASK	0x80000000
#define READ_MASK	0x40000000

struct hd_cmd {
	uint8_t	features;
	uint8_t	count;
	uint8_t	lba_low;
	uint8_t	lba_mid;
	uint8_t	lba_high;
	uint8_t	device;
	uint8_t	command;
};

/*
硬盘驱动管理
*/
struct hd_service
{
	uint8_t disk_nr;	// how many disk on the computer
	uint8_t disk_interrupt;
	uint8_t reg_status;
	
	bool init_done;	//初始化结束与否
	uint8_t *buf;
	struct lock *lock;
	struct task *task;
	struct ioqueue *ioqueue;
	struct mailbox *mailbox;
};

struct hard_disk
{
	uint32_t sectors;	//硬盘扇区数
	uint32_t master_slave;	//硬盘是主盘还是从盘
	uint8_t *buf;
	uint32_t rw_sectors;
	uint8_t reading, writing;
	uint8_t errors;
	
	uint8_t *virtual_memory;	//磁盘对应的虚拟内存地址
};

struct hd_message
{
	uint8_t *buffer;
	int lba;
	int sectors;
	int status;
};

#define HD_MSG_SIZE	sizeof(struct hd_message)

/*
每次读写最大的扇区数
*/
#define MAX_RW_SECTORS	256

extern struct hd_service hd_service;
extern struct hard_disk hda, hdb;
extern struct hard_disk *current_disk;

void hd_main();
void hd_service_process();
void IRQ_primary_channel();

void hd_handler(int irq);
void task_ioctl_entry();
void init_hdd();

void hd_read_sectors(int lba, void *buf, int sectors);
void hd_write_sectors(int lba, void *buf, int sectors);
void set_current_disk(struct hard_disk *hd);

void sync_disk_sector(int lba);

int msg_read_sectors(int lba, void *buf, int sectors);
int msg_write_sectors(int lba, void *buf, int sectors);

int do_f1();

#endif

