#ifndef STRING_H
#define STRING_H

#if _M32
typedef unsigned size_t;
#else
typedef unsigned long long size_t;
#endif

extern void* memcpy(void* dest, const void* src, size_t num);

#endif