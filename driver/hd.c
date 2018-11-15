#include "driver/hd.h"
#include "kernel/debug.h"
#include "kernel/descriptor.h"
#include "driver/mouse.h"
#include "kernel/memory.h"
#include "kernel/asm.h"
#include "driver/clock.h"
#include "kernel/lock.h"
#include "kernel/mailbox.h"

#define MAX_ERRORS 7
struct hd_service hd_service;
struct hard_disk hda, hdb;
struct hard_disk *current_disk;

struct hd_message  read_msg, write_msg;

static void wait_hd_status(uint8_t reg, uint8_t status);
static void hd_specify(struct hard_disk *disk);
static void wait_hd_interrupt();
static void print_identify_info(struct hard_disk *disk, uint16_t* hdinfo);

static int drive_busy(void);
static int disk_result(void);
static int controller_ready(void);
static void rw_sector(int cmd, int lba, char *buf);
static void reset_controller(void);
static void reset_hd();
static void bad_rw(void);

void init_hdd()
{
	//printk("hd begin\n");
	hd_service.disk_nr = *((uint8_t *)HARDDISK_NR_ADDR);
	hd_service.disk_interrupt = 0;
	
	hd_service.buf = kmalloc(SECTOR_SIZE*2);
	
	
	printk("hd_nr:%d\n", hd_service.disk_nr);
	
	hd_service.lock = create_lock();
	lock_init(hd_service.lock);
	
	//初始化读写信息
	memset(&read_msg,0,HD_MSG_SIZE);
	memset(&write_msg,0,HD_MSG_SIZE);
	
	read_msg.buffer = kmalloc(MAX_RW_SECTORS);
	write_msg.buffer = kmalloc(MAX_RW_SECTORS);
	
	//初始化磁盘
	memset(&hda,0,sizeof(struct hard_disk));
	memset(&hdb,0,sizeof(struct hard_disk));
	hda.master_slave = 0;
	hdb.master_slave = 1;
	hda.buf = kmalloc(SECTOR_SIZE*2);
	hdb.buf = kmalloc(SECTOR_SIZE*2);
	
	
	hd_service.init_done = false;
	hd_service.task = NULL;
	hd_service.ioqueue = NULL;
	
	put_irq_handler(HDA_IRQ, hd_handler);
	enable_irq(CASCADE_IRQ);
	enable_irq(HDA_IRQ);
}

int msg_read_sectors(int lba, void *buf, int sectors)
{
	if(hd_service.init_done){
		printk("start to send read msg to hd service.\n");
		//memset(&read_msg, 0, HD_MSG_SIZE);
		read_msg.lba = lba;
		read_msg.sectors = sectors;
		read_msg.status = 1;
		printk("msg:lba %d sectors %d.\n", read_msg.lba, read_msg.sectors);
		//发送信息
		ioqueue_put(hd_service.ioqueue, 1);
		
		while(1){
			if(read_msg.status == 0){
				break;
			}
		}
		
		printk("msg had deal.\n");
		//等待处理完毕
		
		printk("copy data to caller\n");
		memcpy(buf, read_msg.buffer, sectors*SECTOR_SIZE);
		return 1;
	}else{
		return 0;
	}
}

int msg_write_sectors(int lba, void *buf, int sectors)
{
	if(hd_service.init_done){
		printk("start to send write msg to hd service.\n");
		//memset(&read_msg, 0, HD_MSG_SIZE);
		write_msg.lba = lba;
		write_msg.sectors = sectors;
		memcpy(write_msg.buffer,buf, sectors*SECTOR_SIZE);
		
		write_msg.status = 1;
		printk("msg:lba %d sectors %d.\n", write_msg.lba, write_msg.sectors);
		//发送信息
		ioqueue_put(hd_service.ioqueue, 2);
		
		printk("msg had deal.\n");
		//等待处理完毕
		while(write_msg.status){
			
		}
		printk("back to caller\n");
		return 1;
	}else{
		return 0;
	}
}



void wait_msg_return(int type)
{
	
	
	
}

void sync_disk_sector(int lba)
{
	if(hd_service.init_done){
		ioqueue_put(hd_service.ioqueue, lba|WRITE_MASK);
	}
}

void hd_handler(int irq)
{
	if (disk_result()) {
		bad_rw();	//R/W error
		printk("!bad intr!\n");
	}
	hd_service.disk_interrupt = 1;	//Intr ocurred!
}

static void wait_hd_status(uint8_t reg, uint8_t status)
{
	int i = WAIT_TIMEOUT;	
	//wait register's status,if OK, return
	while(i){
		if(io_in8(REG_STATUS)&reg == status){
			return;
		}
		i--;
	}
	//printk("!wait hd error\n");
}

static void wait_hd_interrupt()
{
	int i = WAIT_TIMEOUT;
	while(i > 0){
		if(hd_service.disk_interrupt){
			hd_service.disk_interrupt = 0;	//reset
			return;
		}
		i++;
	}
	printk("!hd inter error\n");
}

static int controller_ready(void)
{
	int retries=1000;

	while (--retries && (io_in8(REG_STATUS)&(STATUS_READY|STATUS_BUSY))!=STATUS_READY);
	return (retries);	// retries != 0 is ready
}

void hd_identify(struct hard_disk *disk)
{
	io_out8(REG_DEVICE, MAKE_DEVICE_REG(0, disk->master_slave, 0));
	io_out8(REG_CMD, ATA_IDENTIFY);
	
	wait_hd_interrupt();	//Wait disk intr
	port_read(REG_DATA, disk->buf, SECTOR_SIZE);	//Read from buffer
	print_identify_info(disk, (uint16_t*)disk->buf);
}

static void hd_specify(struct hard_disk *disk)
{
	io_out8(REG_DEVICE, MAKE_DEVICE_REG(0, disk->master_slave, 0));
	io_out8(REG_CMD, ATA_SPECIFY);
}

static void print_identify_info(struct hard_disk *disk, uint16_t* hdinfo)
{
	//Show information
	int capabilities = hdinfo[49];
	printk("LBA supported:%s ",(capabilities & 0x0200) ? "Yes" : "No");
	int cmd_set_supported = hdinfo[83];
	printk("LBA48 supported:%s ",((cmd_set_supported & 0x0400) ? "Yes" : "No"));
	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	
	disk->sectors = sectors;
	printk("HD size: %x MB\n",sectors * SECTOR_SIZE / (1024*1024));
	
}
/*
*Reset driver controller
*/
static void reset_controller(void)
{
	int	i;

	io_out8(REG_CMD,4);
	for(i = 0; i < 1000; i++) asm_nop();
	io_out8(REG_CMD,0);
	for(i = 0; i < 10000 && drive_busy(); i++) /* nothing */;
	if (drive_busy())
		printk("HD-controller still busy\n");
	if((i = io_in8(STATUS_ERR)) != 1)
		printk("HD-controller reset failed: %x\n",i);
}

static void bad_rw(void)
{
	if (++current_disk->errors >= MAX_ERRORS) {
		return;
	}
	if (current_disk->errors >= MAX_ERRORS/2) {
		reset_hd();
	}
}

static void reset_hd()
{
	reset_controller();
	hd_specify(current_disk); //Setting up driver parameters
}

static int drive_busy(void)
{
	uint32_t i;
	for (i = 0; i < 100000; i++)
		if (STATUS_READY == (io_in8(REG_STATUS) & (STATUS_BUSY | STATUS_READY)))
			break;
	i = io_in8(REG_STATUS);
	i &= STATUS_BUSY | STATUS_READY;
	if (i == STATUS_READY)
		return 0;
	printk("HD controller times out\n");
	return 1;
}

static int disk_result(void)
{
	int i=io_in8(REG_STATUS);

	if ((i & (STATUS_BUSY | STATUS_READY|STATUS_WREER|STATUS_ERR)) == STATUS_READY)
		return 0; /* ok */
	if (i&STATUS_ERR) i=io_in8(REG_ERROR);
	return 1;
}

void hd_read_sectors(int lba, void *buf, int sectors)
{
	//clock.can_schdule = false;
	//If disk is busy for read, we wait
	//while(current_disk->reading);
	int l = lba, s = sectors;
	uint8_t *b = buf;
	lock_acquire(hd_service.lock);
	
	while(s  > 0){
		rw_sector(ATA_READ, l, b);
		l++;
		b += 512;      
		s--;
	}
	//clock.can_schdule = true;
	lock_release(hd_service.lock);
}

void hd_write_sectors(int lba, void *buf, int sectors)
{
	int l = lba, s = sectors;
	uint8_t *b = buf;
	//If disk is busy for write, we wait
	//while(current_disk->writing);
	lock_acquire(hd_service.lock);
	
	while(s > 0){
		rw_sector(ATA_WRITE, l, b);
		l++;
		b += 512;      
		s--;
	}
	
	lock_release(hd_service.lock);
}

static void rw_sector(int cmd, int lba, char *buf)
{
	//Driver should not be busy
	if (!controller_ready() && !drive_busy()) panic("hd error.");

	if(cmd == ATA_READ){	//Read cmd
		current_disk->reading = 1;	//Reading...
		current_disk->errors = 0;
		io_out8(REG_DEV_CTRL, 0);	//Active disk int
		
		//Set command info
		io_out8(REG_FEATURES, -1);
		io_out8(REG_NSECTOR, 1);
		io_out8(REG_LBA_LOW, lba);
		io_out8(REG_LBA_MID, lba>>8);
		io_out8(REG_LBA_HIGH, lba>>16);
		io_out8(REG_DEVICE, MAKE_DEVICE_REG(1, current_disk->master_slave, (lba >> 24) & 0xF));
		
		io_out8(REG_CMD, ATA_READ);	//Send read command
		
		
		wait_hd_interrupt();	//Wait disk intr
		
		port_read(REG_DATA, buf, SECTOR_SIZE);	//Now, disk finished it's task, we read data from disk buffer
		current_disk->reading = 0;	//End reading.
		
	}else if(cmd == ATA_WRITE){	//Read cmd
		current_disk->writing = 1;	//Writing...
		current_disk->errors = 0;
		io_out8(REG_DEV_CTRL, 0);	//Active disk int
		//Set command info
		io_out8(REG_FEATURES, -1);
		io_out8(REG_NSECTOR, 1);
		io_out8(REG_LBA_LOW, lba);
		io_out8(REG_LBA_MID, lba>>8);
		io_out8(REG_LBA_HIGH, lba>>16);
		io_out8(REG_DEVICE, MAKE_DEVICE_REG(1, current_disk->master_slave, (lba >> 24) & 0xF));
		
		io_out8(REG_CMD, ATA_WRITE);	//Send write command
		
		wait_hd_status(STATUS_DRQ, STATUS_DRQ);	//Ready to transform data to disk controller
		
		port_write(REG_DATA, buf, SECTOR_SIZE);//Now, we write data from disk buffer, and disk will write then into sector
		
		wait_hd_interrupt();	//Wait disk intr
		current_disk->writing = 0;	//End wring.
	}else{
		panic("Unknown command!\n");
	}
}

void set_current_disk(struct hard_disk *disk)
{
	current_disk = disk;
}


void use_disk(int dev)
{
	if(0 < dev || dev >= 2){
		return;
	}
	printk("set disk %d.\n",dev);
	switch(dev){
		case 0:
			set_current_disk(&hda);
			
			break;
		case 1:
			set_current_disk(&hdb);
			break;
		default:
			break;
	}
}
