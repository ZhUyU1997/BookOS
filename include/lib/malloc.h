#ifndef _LIB_MEMORY_H
#define _LIB_MEMORY_H
#include "kernel/types.h"

void* malloc(uint32_t size);
void free(void* ptr);

#endif

