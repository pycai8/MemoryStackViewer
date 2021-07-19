#include "defs.h"
#include "BaseApi.h"
#define _GNU_SOURCE
#include <dlfcn.h>

static PFN_Malloc g_pfnMalloc = 0;
static PFN_Free   g_pfnFree   = 0;

void BaseInit()
{
    g_pfnMalloc = (PFN_Malloc)dlsym(RTLD_NEXT, "malloc");
    g_pfnFree = (PFN_Free)dlsym(RTLD_NEXT, "free");
}

void BaseUninit()
{
}

void* BaseMalloc(unsigned size)
{
    if (!g_pfnMalloc) return 0;
    return g_pfnMalloc(size);
}

void BaseFree(void* ptr)
{
    if (!g_pfnFree) return;
    g_pfnFree(ptr);
}

