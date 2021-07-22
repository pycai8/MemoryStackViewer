#include "defs.h"
#include "MyApi.h"

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
    
    if (size == 0) return 0;

    return MyMalloc(size);
}

void free(void* ptr)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (ptr == 0) return;

    MyFree(ptr);
}

void* calloc(size_t cnt, size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (cnt == 0 || size == 0) return 0;

    return MyCalloc(cnt, size);
}

void* realloc(void* ptr, size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (ptr == 0 && size == 0) return 0;

    if (ptr == 0 && size != 0) return MyMalloc(size);

    if (ptr != 0 && size == 0)
    {
        MyFree(ptr);
        return 0;
    }

    // ptr != 0 && size != 0
    return MyRealloc(ptr, size);
}

void* valloc(size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (size == 0) return 0;

    return MyValloc(size);
}

void* pvalloc(size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (size == 0) return 0;

    return MyPValloc(size);
}

void* memalign(size_t alignment, size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (size == 0) return 0;

    return MyMemAlign(alignment, size);
}


void* libc_memalign(size_t alignment, size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (size == 0) return 0;

    return MyLibcMemAlign(alignment, size);
}

int posix_memalign(void** ptr, size_t alignment, size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (ptr == 0) return EINVAL;

    if (size == 0)
    {
        *ptr = 0;
        return EINVAL;
    }

    return MyPosixMemAlign(ptr, alignment, size);
}

void* aligned_alloc(size_t alignment, size_t size)
{
    if (!g_init)
    {
        g_init = 1;
        MyInit();
    }

    if (size == 0) return 0;

    return MyAlignedAlloc(alignment, size);
}


