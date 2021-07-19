#include "MyApi.h"
#include <stddef.h>
#include <stdlib.h>

static int g_init = 0;

__attribute__ ((constructor))
static void init()
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }
}

__attribute__ ((destructor))
static void uninit()
{
    if (g_init)
    {
        MyUninit();
        g_init = 0;
    }
}

void* malloc(size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }
    
    return MyMalloc(size);
}

void free(void* ptr)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    MyFree(ptr);
}

