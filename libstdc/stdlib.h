#ifndef STDLIB_H
#define STDLIB_H

#if _M32
typedef unsigned size_t;
#else
typedef unsigned long long size_t;
#endif

extern void* malloc(size_t bytes);
extern void free(void* block);

#endif