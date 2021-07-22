#ifndef __INNER_API_H__
#define __INNER_API_H__

#include "defs.h"

void InnerInit();

void InnerUninit();

void* InnerMalloc(size_t size);

void InnerFree(void* ptr);

void* InnerCalloc(size_t cnt, size_t size);

void* InnerRealloc(void* ptr, size_t size);

void* InnerValloc(size_t size);

void* InnerPValloc(size_t size);

void* InnerMemAlign(size_t alignment, size_t size);

void* InnerLibcMemAlign(size_t alignment, size_t size);

int InnerPosixMemAlign(void** ptr, size_t alignment, size_t size);

void* InnerAlignedAlloc(size_t alignment, size_t size);

#endif

