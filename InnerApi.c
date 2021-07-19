#include "InnerApi.h"
#include "defs.h"

void InnerInit()
{
}

void InnerUninit()
{
}

void* InnerMalloc(unsigned size)
{
    void* ptr = malloc(size);
    return ptr;
}

void InnerFree(void* ptr)
{
    free(ptr);
}

