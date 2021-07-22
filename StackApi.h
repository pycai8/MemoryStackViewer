#ifndef __STACK_API_H__
#define __STACK_API_H__

#include "defs.h"

void StackInit();

void StackUninit();

void StackMalloc(void* ptr, size_t size);

void StackFree(void* ptr);

#endif

