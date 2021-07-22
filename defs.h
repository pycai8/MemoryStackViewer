#ifndef __DEFS_H__
#define __DEFS_H__

#define _GNU_SOURCE
#include <dlfcn.h>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>

#define HDR_LEN (2*sizeof(void*))
#define MAX_STACK_LEN 100
#define ADD_HDR(ptr) (void*)((unsigned char*)ptr-HDR_LEN)
#define CUT_HDR(ptr) (void*)((unsigned char*)ptr+HDR_LEN)

#define DBG(fmt, ...) {printf(fmt, __VA_ARGS__);printf("\n");}

void* malloc(size_t size);
typedef void* (*PFN_Malloc)(size_t size);

void free(void* ptr);
typedef void (*PFN_Free)(void* ptr);

void* calloc(size_t cnt, size_t size);
typedef void* (*PFN_Calloc)(size_t cnt, size_t size);

void* realloc(void* ptr, size_t size);
typedef void* (*PFN_Realloc)(void* ptr, size_t size);

void* valloc(size_t size);
typedef void* (*PFN_Valloc)(size_t size);

void* pvalloc(size_t size);
typedef void* (*PFN_PValloc)(size_t size);

void* memalign(size_t alignment, size_t size);
typedef void* (*PFN_MemAlign)(size_t alignment, size_t size);

void* libc_memalign(size_t alignment, size_t size);
typedef void* (*PFN_LibcMemAlign)(size_t alignment, size_t size);

int posix_memalign(void** ptr, size_t alignment, size_t size);
typedef int (*PFN_PosixMemAlign)(void** ptr, size_t alignment, size_t size);

void* aligned_alloc(size_t alignment, size_t size);
typedef void* (*PFN_AlignedAlloc)(size_t alignment, size_t size);

#endif

