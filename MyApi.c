#include "MyApi.h"

#include "BaseApi.h"
#include "InnerApi.h"

static thread_local bool g_innerCall = false;

void MyInit()
{
    if (g_innerCall)
    {
        BaseInit();
        return;
    }

    g_innerCall = true;
    InnerInit();
    g_innerCall = false;
}

void MyUninit()
{
    if (g_innerCall)
    {
        BaseUninit();
        return;
    }

    g_innerCall = true;
    InnerUninit();
    g_innerCall = false;
}

void* MyMalloc(unsigned size)
{
    if (g_innerCall)
    {
        return BaseMalloc(size);
    }

    g_innerCall = true;
    void* ret = InnerMalloc(size);
    g_innerCall = false;
    return ret;
}

void MyFree(void* ptr)
{
    if (g_innerCall)
    {
        BaseFree(ptr);
        return;
    }

    g_innerCall = true;
    InnerFree(ptr);
    g_innerCall = false;
}

