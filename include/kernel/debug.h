#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "kernel/types.h"
#include "gui/window.h"

struct debug
{
	uint32_t cursor_x, cursor_y;
	
	struct window *window;
	uint32_t background_color, font_color;
};
extern struct debug debug;

void task_debug_entry();

void init_debug();
int debug_line(uint8_t *vram, uint32_t wide,uint32_t color, uint8_t *s);
int debug_print_buf(char* buf);
int debug_printf(const char *fmt, ...);
void debug_newline();
void set_debug_color(uint32_t color);
void reset_debug_color();

//断言
#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

void spin(char * func_name);
void panic(const char *fmt, ...);

#endif