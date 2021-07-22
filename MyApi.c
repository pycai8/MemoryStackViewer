#include "MyApi.h"

#include "BaseApi.h"
#include "InnerApi.h"

static __thread int g_innerCall = 0;

void MyInit()
{
    g_innerCall = 1;
    BaseInit();
    InnerInit();
    g_innerCall = 0;
}

void MyUninit()
{
    g_innerCall = 1;
    BaseUninit();
    InnerUninit();
    g_innerCall = 0;
}

void* MyMalloc(size_t size)
{
    if (g_innerCall)
    {
        return BaseMalloc(size);
    }

    g_innerCall = 1;
    void* ret = InnerMalloc(size);
    g_innerCall = 0;
    return ret;
}

void MyFree(void* ptr)
{
    if (g_innerCall)
    {
        BaseFree(ptr);
        return;
    }

    g_innerCall = 1;
    InnerFree(ptr);
    g_innerCall = 0;
}


void* MyCalloc(size_t cnt, size_t size)
{
    if (g_innerCall)
    {
        return BaseCalloc(cnt, size);
    }

    g_innerCall = 1;
    void* ret = InnerCalloc(cnt, size);
    g_innerCall = 0;
    return ret;
}

void* MyRealloc(void* ptr, size_t size)
{
    if (g_innerCall)
    {
        return BaseRealloc(ptr, size);
    }

    g_innerCall = 1;
    void* ret = InnerRealloc(ptr, size);
    g_innerCall = 0;
    return ret;
}

void* MyValloc(size_t size)
{
    if (g_innerCall)
    {
        return BaseValloc(size);
    }

    g_innerCall = 1;
    void* ret = InnerValloc(size);
    g_innerCall = 0;
    return ret;
}

void* MyPValloc(size_t size)
{
    if (g_innerCall)
    {
        return BasePValloc(size);
    }

    g_innerCall = 1;
    void* ret = InnerPValloc(size);
    g_innerCall = 0;
    return ret;
}

void* MyMemAlign(size_t alignment, size_t size)
{
    if (g_innerCall)
    {
        return BaseMemAlign(alignment, size);
    }

    g_innerCall = 1;
    void* ret = InnerMemAlign(alignment, size);
    g_innerCall = 0;
    return ret;
}

void* MyLibcMemAlign(size_t alignment, size_t size)
{
    if (g_innerCall)
    {
        return BaseLibcMemAlign(alignment, size);
    }

    g_innerCall = 1;
    void* ret = InnerLibcMemAlign(alignment, size);
    g_innerCall = 0;
    return ret;
}

int MyPosixMemAlign(void** ptr, size_t alignment, size_t size)
{
    if (g_innerCall)
    {
        return BasePosixMemAlign(ptr, alignment, size);
    }

    g_innerCall = 1;
    int ret = InnerPosixMemAlign(ptr, alignment, size);
    g_innerCall = 0;
    return ret;
}

void* MyAlignedAlloc(size_t alignment, size_t size)
{
    if (g_innerCall)
    {
        return BaseAlignedAlloc(alignment, size);
    }

    g_innerCall = 1;
    void* ret = InnerAlignedAlloc(alignment, size);
    g_innerCall = 0;
    return ret;
}

