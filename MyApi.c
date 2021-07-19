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

void* MyMalloc(unsigned size)
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

