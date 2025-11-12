#include <stdio.h>

#include "alloc.h"

#ifdef CHECK_ALLOC 

typedef struct _alloc_info {
  void* ptr;
  size_t size;
  int line;
  char* file;
  struct _alloc_info* prev;
} alloc_info_t;

alloc_info_t* AllocList = NULL;

static alloc_info_t* NewAlloc(void* ptr, size_t s, int l, char* f) {
  alloc_info_t* out = malloc(sizeof(alloc_info_t));

  out->ptr = ptr;
  out->size = s;
  out->line = l;
  out->file = f;
  return out;
}

void* tDebugMalloc(size_t s, int l, char* f) {
  void* ptr;
  alloc_info_t* al;

  ptr = malloc(s);

  al = NewAlloc(ptr, s, l, f);
  al->prev = AllocList;
  AllocList = al;

  return ptr;
}

void tDebugFree(void* ptr) {
  alloc_info_t* current;
  alloc_info_t* next;

  current = AllocList;
  next = current;

  while (NULL != current) {
    if (current->ptr == ptr) break;

    next = current;
    current = current->prev;
  }

  if (current == next) {
    AllocList = AllocList->prev;
  } else {
    next->prev = current->prev;
  }

  free(current);

  free(ptr);
}

void tEndAssert(void) {
  alloc_info_t* current;

  while (NULL != AllocList) {
    printf("Unhandled allocation at %p of size %lu, allocated in file %s on line %d\n", AllocList->ptr, AllocList->size, AllocList->file, AllocList->line);
    current = AllocList->prev;
    free(AllocList);
    AllocList = current;
  }
}

#endif 
