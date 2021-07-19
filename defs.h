#include <stddef.h>

typedef void*    (*PFN_Malloc)    (size_t size);
typedef void     (*PFN_Free)      (void* ptr);

void* malloc(size_t size);
void  free  (void* ptr);

