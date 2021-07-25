#include "StackApi.h"

typedef struct tagMemoryStack
{
    unsigned long long total;
    unsigned long long count;

    struct tagMemoryStack* prev;
    struct tagMemoryStack* next;

    size_t depth;
    unsigned long long vid;
    void* bt[MAX_STACK_LEN];
} MemoryStack_t;

typedef struct tagPtrHeader
{
    unsigned long size;
    MemoryStack_t* stack;
} PtrHeader_t;

MemoryStack_t* StackGetCurrent()
{
    MemoryStack_t* ret = (MemoryStack_t*)malloc(sizeof(MemoryStack_t));
    if (ret == 0) return 0;

    ret->count = 0;
    ret->total = 0;
    ret->prev = 0;
    ret->next = 0;
    ret->depth = MAX_STACK_LEN;
    ret->vid = 0;
    
    // get bt
    ret->depth = backtrace(ret->bt, ret->depth);
    
    return ret;
}

void StackDelCurrent(MemoryStack_t* stack)
{
    free(stack);
}

void StackPrintCurrent(MemoryStack_t* stack)
{
    int i = 0;
    DBG("count:%llu, total:%llu", stack->count, stack->total);
    for (i = 0; i < stack->depth; i++)
    {
        void* ptr = stack->bt[i];
        Dl_info info = {0};
        int ret = dladdr(ptr, &info);
        if (ret == 0)
        {
            DBG("src[%p] unknow[%s]", ptr, dlerror());
            continue;
        }

        unsigned long diff = 0;
        if (info.dli_saddr != 0 && info.dli_fbase != 0)
            diff = info.dli_saddr - info.dli_fbase;

        DBG("#%d %s(%s+0x%x) [%p]"
            , i
            , (info.dli_fname != 0 && *info.dli_fname != 0 ? info.dli_fname : "unknow")
            , (info.dli_sname != 0 && *info.dli_sname != 0 ? info.dli_sname : "unknow")
            , diff
            , ptr);
    }
}

void StackInit()
{
}

void StackUninit()
{
}

void StackMalloc(void* ptr, size_t size)
{
    PtrHeader_t* h = (PtrHeader_t*)ptr;
    h->size = size - HDR_LEN;
    h->stack = StackGetCurrent();
    if (!h->stack) return;

    h->stack->total += h->size;
    h->stack->count++;
}

void StackFree(void* ptr)
{
    PtrHeader_t* h = (PtrHeader_t*)ptr;
    if (!h->stack) return;

    StackPrintCurrent(h->stack);

    h->stack->count--;
    h->stack->total -= h->size;
    if (h->stack->total == 0) StackDelCurrent(h->stack);
}
