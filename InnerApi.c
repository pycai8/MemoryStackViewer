#include "InnerApi.h"

#include "StackApi.h"

void InnerInit()
{
    StackInit();
}

void InnerUninit()
{
    StackUninit();
}

void* InnerMalloc(size_t size)
{
    size += HDR_LEN;
    
    void* ptr = malloc(size);
    if (ptr == 0) return 0;

    StackMalloc(ptr, size);
    return CUT_HDR(ptr);
}

void InnerFree(void* ptr)
{
    void* tmp = ADD_HDR(ptr);
    StackFree(tmp);
    free(tmp);
}

void* InnerCalloc(size_t cnt, size_t size)
{
    size *= cnt;
    size += HDR_LEN;
    
    void* ret = calloc(size, 1);
    if (ret == 0) return 0;

    StackMalloc(ret, size);
    return CUT_HDR(ret);
}

void* InnerRealloc(void* ptr, size_t size)
{
    size += HDR_LEN;
    void* tmp = ADD_HDR(ptr);

    void* ret = realloc(tmp, size);
    if (ret == 0) return 0;

    StackFree(tmp); // ??? tmp freed already
    StackMalloc(ret, size);
    return CUT_HDR(ret);
}

void* InnerValloc(size_t size)
{
    size += HDR_LEN;
    
    void* ptr = valloc(size);
    if (ptr == 0) return 0;

    StackMalloc(ptr, size);
    return CUT_HDR(ptr);
}

void* InnerPValloc(size_t size)
{
    size += HDR_LEN;
    
    void* ptr = pvalloc(size);
    if (ptr == 0) return 0;

    StackMalloc(ptr, size);
    return CUT_HDR(ptr);
}

void* InnerMemAlign(size_t alignment, size_t size)
{
    size += HDR_LEN;

    void* ret = memalign(alignment, size);
    if (ret == 0) return 0;

    StackMalloc(ret, size);
    return CUT_HDR(ret);
}

void* InnerLibcMemAlign(size_t alignment, size_t size);
{
    size += HDR_LEN;

    void* ret = libc_memalign(alignment, size);
    if (ret == 0) return 0;

    StackMalloc(ret, size);
    return CUT_HDR(ret);
}

int InnerPosixMemAlign(void** ptr, size_t alignment, size_t size);

void* InnerAlignedAlloc(size_t alignment, size_t size);
{
    size += HDR_LEN;

    void* ret = aligned_alloc(alignment, size);
    if (ret == 0) return 0;

    StackMalloc(ret, size);
    return CUT_HDR(ret);
}

