#ifndef STDLIB_H
#define STDLIB_H

#if _M32
typedef unsigned size_t;
#else
typedef unsigned long long size_t;
#endif

extern void* malloc(size_t bytes);
extern void free(void* block);

extern void exit(int status);

extern void qsort(const void* ptr, size_t count, size_t size,
        int(*cmp)(const void*, const void*));

#endif