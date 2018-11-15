#####
#@Name: Makefile
#@Auther: Hu Zicheng
#@Time: 2018/7/7
#####

#The kernel's virtual entry
KERNEL_ENTRY = 0x80100000
#Load is on the 1 sector of disk, it will use 8 sectors
LOAD_SECTOR_OFFSET = 1		
LOAD_SECTORS = 8

#Kernel is on the 9 sector,it will use 256 sectors(256*512/1024 = 128KB)
#The elf file will use memory address 0x10000, and end at  0x42000 (0x10000+0x3200)
KERNEL_SECTOR_OFFEST = 9
KERNEL_SECTORS = 348

#The complete tools path
NASM 		= nasm
CC			= gcc
LD 			= ld
QEMU 		= qemu

#All the temporary file will be put under build/
BUILD_DIR 		= build/

#The complete flags
ASM_BOOT_FLAGS	= -I boot/include/
ASM_KERNEL_FLAGS	= -I include/ -f elf
C_KERNEL_FLAGS	= -I include/ -c -fno-builtin
LDFLAGS		= -N -e _start -Ttext $(KERNEL_ENTRY)

#The finally file path
BOOTHD_BIN = $(BUILD_DIR)boot/boothd.bin
LOADHD_BIN = $(BUILD_DIR)boot/loadhd.bin

BOOTFD_BIN = $(BUILD_DIR)boot/bootfd.bin
LOADFD_BIN = $(BUILD_DIR)boot/loadfd.bin

KERNEL_FILE = $(BUILD_DIR)kernel/KERNEL
FLAPPY_IMG = image/floppy.img
HDA_IMG = image/hda.img
HDB_IMG = image/hdb.img

#The objs for the kernel
OBJS =  $(BUILD_DIR)kernel/_start.o\
		$(BUILD_DIR)init/main.o\
		$(BUILD_DIR)kernel/io.o\
		$(BUILD_DIR)kernel/page.o\
		$(BUILD_DIR)kernel/memory.o\
		$(BUILD_DIR)kernel/ards.o\
		$(BUILD_DIR)kernel/debug.o\
		$(BUILD_DIR)kernel/task.o\
		$(BUILD_DIR)kernel/tss.o\
		$(BUILD_DIR)kernel/8259a.o\
		$(BUILD_DIR)kernel/interrupt.o\
		$(BUILD_DIR)kernel/descriptor.o\
		$(BUILD_DIR)kernel/asm.o\
		$(BUILD_DIR)kernel/syscall.o\
		$(BUILD_DIR)kernel/semaphore.o\
		$(BUILD_DIR)kernel/lock.o\
		$(BUILD_DIR)kernel/ioqueue.o\
		$(BUILD_DIR)kernel/irqservice.o\
		$(BUILD_DIR)kernel/mailbox.o\
		$(BUILD_DIR)kernel/request.o\
		$(BUILD_DIR)driver/vga.o\
		$(BUILD_DIR)driver/clock.o\
		$(BUILD_DIR)driver/hd.o\
		$(BUILD_DIR)driver/keyboard.o\
		$(BUILD_DIR)driver/cmos.o\
		$(BUILD_DIR)driver/mouse.o\
		$(BUILD_DIR)lib/vsprintf.o\
		$(BUILD_DIR)lib/string.o\
		$(BUILD_DIR)lib/bitmap.o\
		$(BUILD_DIR)lib/exit.o\
		$(BUILD_DIR)lib/printf.o\
		$(BUILD_DIR)lib/getpid.o\
		$(BUILD_DIR)lib/file.o\
		$(BUILD_DIR)lib/malloc.o\
		$(BUILD_DIR)lib/getticks.o\
		$(BUILD_DIR)lib/getch.o\
		$(BUILD_DIR)lib/clear.o\
		$(BUILD_DIR)lib/ps.o\
		$(BUILD_DIR)lib/func.o\
		$(BUILD_DIR)lib/task.o\
		$(BUILD_DIR)lib/putch.o\
		$(BUILD_DIR)lib/wait.o\
		$(BUILD_DIR)lib/mailbox.o\
		$(BUILD_DIR)lib/disk.o\
		$(BUILD_DIR)lib/request.o\
		$(BUILD_DIR)fs/fatxe.o\
		$(BUILD_DIR)fs/fat.o\
		$(BUILD_DIR)fs/dir.o\
		$(BUILD_DIR)fs/file.o
		
#Finall destination is all
.PHONY: style0

#All is clean temporary files, compile files, write file into disk(floppy)
#all: compile write_disk write_file
style0: hd_compile hd_image

style1: fd_compile fd_image

#Compile files
hd_compile: $(BOOTHD_BIN) $(LOADHD_BIN) $(KERNEL_FILE)

fd_compile: $(BOOTFD_BIN) $(LOADFD_BIN) $(KERNEL_FILE)


#Write file into disk
hd_image:	
	dd if=$(BOOTHD_BIN) of=$(HDA_IMG) bs=512 count=1 conv=notrunc
	dd if=$(LOADHD_BIN) of=$(HDA_IMG) bs=512 seek=$(LOAD_SECTOR_OFFSET) count=$(LOAD_SECTORS) conv=notrunc
	dd if=$(KERNEL_FILE) of=$(HDA_IMG) bs=512 seek=$(KERNEL_SECTOR_OFFEST) count=$(KERNEL_SECTORS) conv=notrunc

fd_image:
	dd if=$(BOOTFD_BIN) of=$(FLAPPY_IMG) bs=512 count=1 conv=notrunc
	dd if=$(LOADFD_BIN) of=$(FLAPPY_IMG) bs=512 seek=$(LOAD_SECTOR_OFFSET) count=$(LOAD_SECTORS) conv=notrunc
	dd if=$(KERNEL_FILE) of=$(FLAPPY_IMG) bs=512 seek=$(KERNEL_SECTOR_OFFEST) count=$(KERNEL_SECTORS) conv=notrunc

#Clean temporary files
clean:
	-rm $(BUILD_DIR)kernel/KERNEL 
	-rm $(BUILD_DIR)kernel/*.o
	-rm $(BUILD_DIR)lib/*.o
	-rm $(BUILD_DIR)driver/*.o
	-rm $(BUILD_DIR)boot/*.bin
	-rm $(BUILD_DIR)fs/*.o
	-rm $(BUILD_DIR)init/*.o

#Only clean kernel file
clean_kernel:
	-rm $(BUILD_DIR)kernel/KERNEL 

#make hd
hd: style0
	qemu-system-i386 -m 64 -hda $(HDA_IMG) -hdb $(HDB_IMG) -boot c
#make fd
fd: style1
	qemu-system-i386 -m 64 -fda $(FLAPPY_IMG) -hda $(HDA_IMG) -hdb $(HDB_IMG) -boot a

#Create boot&load file
boot_load_hd:
$(BOOTHD_BIN):boot/boothd.asm
	$(NASM) $(ASM_BOOT_FLAGS) -o $@ $<
$(LOADHD_BIN) : boot/loadhd.asm
	$(NASM) $(ASM_BOOT_FLAGS) -o $@ $<

boot_load_fd:
$(BOOTFD_BIN):boot/bootfd.asm
	$(NASM) $(ASM_BOOT_FLAGS) -o $@ $<
$(LOADFD_BIN) : boot/loadfd.asm
	$(NASM) $(ASM_BOOT_FLAGS) -o $@ $<
	
#Create kernel file
$(KERNEL_FILE): $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL_FILE) $(OBJS)

#####
#kernel file
#####
$(BUILD_DIR)kernel/_start.o : kernel/_start.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)init/main.o : init/main.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)kernel/io.o : kernel/io.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)kernel/memory.o : kernel/memory.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)kernel/tss.o : kernel/tss.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<		

$(BUILD_DIR)kernel/syscall.o : kernel/syscall.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)kernel/task.o : kernel/task.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/8259a.o : kernel/8259a.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)kernel/interrupt.o : kernel/interrupt.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
	
$(BUILD_DIR)kernel/descriptor.o : kernel/descriptor.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
	
$(BUILD_DIR)kernel/ards.o : kernel/ards.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)kernel/cpu.o : kernel/cpu.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)kernel/asm.o : kernel/asm.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)kernel/debug.o : kernel/debug.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)kernel/message.o : kernel/message.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/page.o : kernel/page.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/kernel.o : kernel/kernel.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/semaphore.o : kernel/semaphore.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/lock.o : kernel/lock.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/ioqueue.o : kernel/ioqueue.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/irqservice.o : kernel/irqservice.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/console.o : kernel/console.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/mailbox.o : kernel/mailbox.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)kernel/request.o : kernel/request.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	

$(BUILD_DIR)kernel/shell.o : kernel/shell.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
	
############
#driver file
############
$(BUILD_DIR)driver/vga.o : driver/vga.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
	
$(BUILD_DIR)driver/clock.o : driver/clock.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)driver/keyboard.o : driver/keyboard.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)driver/mouse.o : driver/mouse.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)driver/cmos.o : driver/cmos.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)driver/timer.o : driver/timer.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)driver/hd.o : driver/hd.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

############	
#lib file 	
############
$(BUILD_DIR)lib/string.o : lib/string.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/vsprintf.o : lib/vsprintf.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)lib/bitmap.o : lib/bitmap.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)lib/exit.o  : lib/exit.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)lib/printf.o : lib/printf.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
	
$(BUILD_DIR)lib/getpid.o : lib/getpid.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)lib/fatxe.o : lib/fatxe.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/malloc.o : lib/malloc.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/getticks.o : lib/getticks.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/getch.o : lib/getch.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/clear.o : lib/clear.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/ps.o : lib/ps.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/func.o : lib/func.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/task.o : lib/task.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/putch.o : lib/putch.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/file.o : lib/file.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)lib/wait.o : lib/wait.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/mailbox.o : lib/mailbox.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)lib/disk.o : lib/disk.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)lib/graphic.o : lib/graphic.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<	
$(BUILD_DIR)lib/request.o : lib/request.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<
	
############
#syscall file
############ 	
$(BUILD_DIR)syscall/syscall.o : syscall/syscall.asm
	$(NASM) $(ASM_KERNEL_FLAGS) -o $@ $<

$(BUILD_DIR)syscall/printf.o : syscall/printf.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
	
############
#fs file
############ 	
$(BUILD_DIR)fs/fatxe.o : fs/fatxe.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)fs/fat.o : fs/fat.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)fs/dir.o : fs/dir.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
$(BUILD_DIR)fs/file.o : fs/file.c
	$(CC) $(C_KERNEL_FLAGS) -o $@ $<
