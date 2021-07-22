#ifndef __MY_API_H__
#define __MY_API_H__

#include "defs.h"

void MyInit();

void MyUninit();

void* MyMalloc(size_t size);

void MyFree(void* ptr);

void* MyCalloc(size_t cnt, size_t size);

void* MyRealloc(void* ptr, size_t size);

void* MyValloc(size_t size);

void* MyPValloc(size_t size);

void* MyMemAlign(size_t alignment, size_t size);

void* MyLibcMemAlign(size_t alignment, size_t size);

int MyPosixMemAlign(void** ptr, size_t alignment, size_t size);

void* MyAlignedAlloc(size_t alignment, size_t size);

#endif

