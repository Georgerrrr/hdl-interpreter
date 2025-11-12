#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

//#define CHECK_ALLOC

#ifndef CHECK_ALLOC
#define tMalloc(x) (malloc(x))
#define tFree(x)   (free(x))
#else 
#define tMalloc(x) (tDebugMalloc(x, __LINE__, __FILE__))
#define tFree(x)   (tDebugFree(x))

void* tDebugMalloc(size_t, int, char*);
void tDebugFree(void*);

void tEndAssert(void);
#endif

#endif // ALLOC_H
