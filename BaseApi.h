#ifndef __BASE_API_H__
#define __BASE_API_H__

#include "defs.h"

void BaseInit();

void BaseUninit();

void* BaseMalloc(size_t size);

void BaseFree(void* ptr);

void* BaseCalloc(size_t cnt, size_t size);

void* BaseRealloc(void* ptr, size_t size);

void* BaseValloc(size_t size);

void* BasePValloc(size_t size);

void* BaseMemAlign(size_t alignment, size_t size);

void* BaseLibcMemAlign(size_t alignment, size_t size);

int BasePosixMemAlign(void** ptr, size_t alignment, size_t size);

void* BaseAlignedAlloc(size_t alignment, size_t size);

#endif

