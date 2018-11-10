#include "driver/keyboard.h"
#include "kernel/descriptor.h"
#include "kernel/io.h"
#include "driver/vga.h"
#include "driver/keymap.h"
#include "gui/gui.h"
#include "gui/graphic.h"
#include "gui/layer.h"
#include "driver/console.h"
#include "kernel/syscall.h"
#include "kernel/task.h"
#include "gui/window.h"
#include "kernel/message.h"
#include "kernel/debug.h"
#include "kernel/interruption.h"
#include "kernel/irqservice.h"

int key_buf[KEYBOARD_FIFO_LEN];
int syscall_keyboard_char;

uint8_t get_byte_from_kbuf() ;
static	int	code_with_E0 = 0;
static	int	shift_l;	/* l shift state */
static	int	shift_r;	/* r shift state */
static	int	alt_l;		/* l alt state	 */
static	int	alt_r;		/* r left state	 */
static	int	ctrl_l;		/* l ctrl state	 */
static	int	ctrl_r;		/* l ctrl state	 */
static	int	caps_lock;	/* Caps Lock	 */
static	int	num_lock;	/* Num Lock	 */
static	int	scroll_lock;	/* Scroll Lock	 */
static	int	column;

static void kb_wait();
static void kb_ack();
static void set_leds();

int keyboard_data;


void key_char_process(uint32_t key);
void put_key(uint32_t key);
void wait_KBC_sendready(void);

extern struct layer *active_layer;
extern struct task *task_console, *task_keyboard;
extern struct irq_service irq_service;


void keyboard_handler(int32_t irq)
{
	uint8_t scan_code = io_in8(KB_DATA);
	//printk("key %x\n",scan_code);
	//fifo32_put(&key_fifo, scan_code);
	ioqueue_put(&irq_service.ioqueue, scan_code + KEYBOARD_DATA_LOW);
	irq_service.task->priority = PRIORITY_LEVEL_I;
	task_wakeup(irq_service.task);
	
}

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void init_keyboard()
{
	
	//fifo32_init(&key_fifo, KEYBOARD_FIFO_LEN, key_buf);
	
	shift_l	= shift_r = 0;
	alt_l	= alt_r   = 0;
	ctrl_l	= ctrl_r  = 0;
	
	caps_lock   = 0;
	num_lock    = 1;
	scroll_lock = 0;
	
	//syscall_keyboard_char = -1;
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	
	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
	
	put_str(">init keyboard\n");
	
}


void wait_KBC_sendready(void)
{
	/*等待键盘控制电路准备完毕*/
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}
/*
void task_keyboard_entry()
{
	int i = 0;
	struct task *task = task_current();
	//printf("Driver of keyboard is running.\n");

	for(;;){
		if(fifo32_status(&key_fifo) == 0){
			if(task->status != TASK_SLEEP){
				//task_sleep(task);
			}
		} else {
			if(fifo32_status(&key_fifo) != 0){
				keyboard_read();
			}
		}
	}
}*/

/*
把数据交给keyboard处理
*/
void keyboard_main()
{
	uint8_t scan_code;
	int make;
	
	uint32_t key = 0;
	uint32_t* keyrow;

		code_with_E0 = 0;

		scan_code = get_byte_from_kbuf();
		
		if(scan_code == 0xe1){
			int i;
			uint8_t pausebrk_scode[] = {0xE1, 0x1D, 0x45,
					       0xE1, 0x9D, 0xC5};
			int is_pausebreak = 1;
			for(i=1;i<6;i++){
				if (get_byte_from_kbuf() != pausebrk_scode[i]) {
					is_pausebreak = 0;
					break;
				}
			}
			if (is_pausebreak) {
				key = PAUSEBREAK;
			}
		} else if(scan_code == 0xe0){
			scan_code = get_byte_from_kbuf();

			//PrintScreen 被按下
			if (scan_code == 0x2A) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				}
			}
			//PrintScreen 被释放
			if (scan_code == 0xB7) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			//不是PrintScreen, 此时scan_code为0xE0紧跟的那个值. 
			if (key == 0) {
				code_with_E0 = 1;
			}
		}if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
			make = (scan_code & FLAG_BREAK ? 0 : 1);

			//先定位到 keymap 中的行 
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
			
			column = 0;
			int caps = shift_l || shift_r;
			if (caps_lock) {
				if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')){
					caps = !caps;
				}
			}
			if (caps) {
				column = 1;
			}

			if (code_with_E0) {
				column = 2;
			}
			
			key = keyrow[column];
			
			switch(key) {
			case SHIFT_L:
				shift_l = make;
				break;
			case SHIFT_R:
				shift_r = make;
				break;
			case CTRL_L:
				ctrl_l = make;
				break;
			case CTRL_R:
				ctrl_r = make;
				break;
			case ALT_L:
				alt_l = make;
				break;
			case ALT_R:
				alt_l = make;
				break;
			case CAPS_LOCK:
				if (make) {
					caps_lock   = !caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if (make) {
					num_lock    = !num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if (make) {
					scroll_lock = !scroll_lock;
					set_leds();
				}
				break;	
			default:
				break;
			}
			
			if (make) { //忽略 Break Code
				int pad = 0;

				//首先处理小键盘
				if ((key >= PAD_SLASH) && (key <= PAD_9)) {
					pad = 1;
					switch(key) {
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (num_lock &&
						    (key >= PAD_0) &&
						    (key <= PAD_9)) {
							key = key - PAD_0 + '0';
						}
						else if (num_lock &&
							 (key == PAD_DOT)) {
							key = '.';
						}
						else{
							switch(key) {
							case PAD_HOME:
								key = HOME;
								break;
							case PAD_END:
								key = END;
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								break;
							case PAD_PAGEDOWN:
								key = PAGEDOWN;
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}
				
				key |= shift_l	? FLAG_SHIFT_L	: 0;
				key |= shift_r	? FLAG_SHIFT_R	: 0;
				key |= ctrl_l	? FLAG_CTRL_L	: 0;
				key |= ctrl_r	? FLAG_CTRL_R	: 0;
				key |= alt_l	? FLAG_ALT_L	: 0;
				key |= alt_r	? FLAG_ALT_R	: 0;
				key |= pad      ? FLAG_PAD      : 0;
				
				//put_str(&key);
				key_char_process(key);
			}
		}
	/*if(irq_service.keyboard_key != -1){
		printk("[keyboard] key=%c\n", irq_service.keyboard_key);
	}	*/
}

void key_char_process(uint32_t key)
{
	//char output[2] = {'\0', '\0'};

	if (!(key & FLAG_EXT)) {
		//gprintk("C");
		put_key( key);
	} else {
		int raw_code = key & MASK_RAW;
		switch(raw_code) {
			case ENTER:
				put_key('\n');
				break;
			case BACKSPACE:
				put_key( '\b');
				break;
			case TAB:
				put_key( '\t');
				break;
			case F1:
				//put_key( F1);
				
				break;
			case F2:
				//put_key( F2);
				
				break;
			case F3:
				//put_key( F3);
				
				break;
			case F4:
				//put_key( F4);
				
				break;
			case F5:
				put_key( F5);
				break;	
				
			case F12:  
				put_key( F12);
				break;
				
			case UP:
				put_key( UP);
				break;
			case DOWN:
				put_key(DOWN);
				break;
			case LEFT:
				//gprintk("LEFT");
				put_key(LEFT);
				break;
			case RIGHT:
				//gprintk("RIGHT");
				put_key(RIGHT);
				break;
				//syscall_keyboard_char = 1000;
			default:
				break;
		}
	}
}

void put_key(uint32_t key)
{
	//ioqueue_put(current_console->ioqueue, key);
	//current_console->key = key;
	irq_service.keyboard_key = key;
}

//从键盘缓冲区中读取下一个字节 

uint8_t get_byte_from_kbuf()       
{
    return (uint8_t )(ioqueue_get(&irq_service.ioqueue, IQ_MODE_MOVE) - KEYBOARD_DATA_LOW);
}

// 等待 8042 的输入缓冲区空
static void kb_wait()	
{
	uint8_t kb_stat;

	do {
		kb_stat = io_in8(KB_CMD);
	} while (kb_stat & 0x02);
}

static void kb_ack()
{
	uint8_t kb_read;

	do {
		kb_read = io_in8(KB_DATA);
	} while (kb_read =! KB_ACK);
}

static void set_leds()
{
	uint8_t leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;

	kb_wait();
	io_out8(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	io_out8(KB_DATA, leds);
	kb_ack();
}
/*
keyboard处理完数据后输出
*/
void keyboard_output()
{
	//处理数据
	//if(irq_service.keyboard_key != -1){
		//irq_service.keyboard_data = irq_service.keyboard_output_data;
		//irq_service.keyboard_key = -1;
	//}
}

int keyobard_getch()
{
	int data = EOF;
	//处理数据
	if(irq_service.keyboard_key != -1){
		data = irq_service.keyboard_key;
		irq_service.keyboard_key = -1;
	}
	return data;
}

