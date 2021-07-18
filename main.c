#include "MyApi.h"

static bool g_init = false;

__attribute__ ((constructor))
static void init()
{
    if (!g_init)
    {
        MyInit();
        g_init = true;
    }
}

__attribute__ ((destructor))
static void uninit()
{
    if (g_init)
    {
        MyUninit();
        g_init = false;
    }
}

void* malloc(unsigned size)
{
    if (!g_init)
    {
        MyInit();
        g_init = true;
    }
    
    return MyMalloc(size);
}

void free(void* ptr)
{
    if (!g_init)
    {
        MyInit();
        g_init = true;
    }

    MyFree(ptr);
}

