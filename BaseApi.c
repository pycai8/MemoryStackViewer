#include "BaseApi.h"

static PFN_Malloc g_pfnMalloc = 0;
static PFN_Free g_pfnFree = 0;
static PFN_Calloc g_pfnCalloc = 0;
static PFN_Realloc g_pfnRealloc = 0;
static PFN_Valloc g_pfnValloc = 0;
static PFN_PValloc g_pfnPValloc = 0;
static PFN_MemAlign g_pfnMemAlign = 0;
static PFN_LibcMemAlign g_pfnLibcMemAlign = 0;
static PFN_PosixMemAlign g_pfnPosixMemAlign = 0;
static PFN_AlignedAlloc g_pfnAlignedAlloc = 0;

void BaseInit()
{
    g_pfnMalloc = (PFN_Malloc)dlsym(RTLD_NEXT, "malloc");
    g_pfnFree = (PFN_Free)dlsym(RTLD_NEXT, "free");
    g_pfnCalloc = (PFN_Calloc)dlsym(RTLD_NEXT, "calloc");
    g_pfnRealloc = (PFN_Realloc)dlsym(RTLD_NEXT, "realloc");
    g_pfnValloc = (PFN_Valloc)dlsym(RTLD_NEXT, "valloc");
    g_pfnPValloc = (PFN_PValloc)dlsym(RTLD_NEXT, "pvalloc");
    g_pfnMemAlign = (PFN_MemAlign)dlsym(RTLD_NEXT, "memalign");
    g_pfnLibcMemAlign = (PFN_LibcMemAlign)dlsym(RTLD_NEXT, "libc_memalign");
    g_pfnPosixMemAlign = (PFN_PosixMemAlign)dlsym(RTLD_NEXT, "posix_memalign");
    g_pfnAlignedAlloc = (PFN_AlignedAlloc)dlsym(RTLD_NEXT, "aligned_alloc");
}

void BaseUninit()
{
}

void* BaseMalloc(size_t size)
{
    if (!g_pfnMalloc) return 0;
    return g_pfnMalloc(size);
}

void BaseFree(void* ptr)
{
    if (!g_pfnFree) return;
    g_pfnFree(ptr);
}

void* BaseCalloc(size_t cnt, size_t size)
{
    if (!g_pfnCalloc) return 0;
    return g_pfnCalloc(cnt, size);
}

void* BaseRealloc(void* ptr, size_t size)
{
    if (!g_pfnRealloc) return 0;
    return g_pfnRealloc(ptr, size);
}

void* BaseValloc(size_t size)
{
    if (!g_pfnValloc) return 0;
    return g_pfnValloc(size);
}

void* BasePValloc(size_t size)
{
    if (!g_pfnPValloc) return 0;
    return g_pfnPValloc(size);
}

void* BaseMemAlign(size_t alignment, size_t size)
{
    if (!g_pfnMemAlign) return 0;
    return g_pfnMemAlign(alignment, size);
}

void* BaseLibcMemAlign(size_t alignment, size_t size)
{
    if (!g_pfnLibcMemAlign) return 0;
    return g_pfnLibcMemAlign(alignment, size);
}

int BasePosixMemAlign(void** ptr, size_t alignment, size_t size)
{
    if (!g_pfnPosixMemAlign) return EINVAL;
    return g_pfnPosixMemAlign(ptr, alignment, size);
}

void* BaseAlignedAlloc(size_t alignment, size_t size)
{
    if (!g_pfnAlignedAlloc) return 0;
    return g_pfnAlignedAlloc(alignment, size);
}

